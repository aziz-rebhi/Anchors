import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import { useCreateBlockNote, SuggestionMenuController } from "@blocknote/react";
import { BlockNoteView } from "@blocknote/mantine";
import type { BlockNoteEditor } from "@blocknote/core";
import type { PartialBlock } from "@blocknote/core";

import "@blocknote/core/fonts/inter.css";
import "@blocknote/mantine/style.css";
import "./bridge.css";

import { bridge, installBridge, onHostCall, type HostNote, type ThemeMode } from "./bridge";
import { schemaWithColumns } from "./columns";
import { makeSlashMenuItems } from "./slashItems";

/** Debounce window before a change is pushed to the Qt host. */
const SAVE_DEBOUNCE_MS = 800;

/**
 * Backstop for the case where no input event is ever seen (a programmatic
 * editor mutation, say). Non-legacy notes lift suppression after this long.
 * Legacy notes deliberately ignore it - see suppressChanges().
 */
const SUPPRESS_ARM_MS = 10_000;

/**
 * After a programmatic load, change events are BlockNote normalising the
 * document, not the user editing it, and must not trigger a save.
 *
 * This used to be a fixed 250ms window. That is a guess, and it
 * guesses wrong: normalisation is not bounded in time, so under load a late
 * event lands after the window closes. It then looks like a user edit, arms the
 * save timer, and for a legacy note overwrites the body on disk without anyone
 * typing. Observed in 1 of 3 runs with the fixed window.
 *
 * A timer cannot tell those two cases apart, so we don't ask it to: suppression
 * is lifted by the first real input event inside the editor canvas instead -
 * a positive signal of user intent rather than an absence of activity.
 */

type AnyEditor = BlockNoteEditor<any, any, any>;

const isDemo =
  typeof window !== "undefined" &&
  new URLSearchParams(window.location.search).get("demo") === "1";

// --- document (de)serialisation --------------------------------------------

/** Wrap the editor's top-level blocks in BlockNote's document envelope. */
export function serializeDoc(editor: AnyEditor): string {
  return JSON.stringify({ type: "doc", content: editor.document });
}

/**
 * Parse a stored note into BlockNote blocks.
 *
 * Returns `null` when the payload is not a BlockNote document at all - that is
 * the legacy Anchors Document JSON (`{ id, title, blocks: [...] }`). The caller
 * must not try to render those as BlockNote blocks: the shapes are close enough
 * to be tempting and different enough to corrupt content.
 */
export function parseBlockNoteDoc(raw: string, editor: AnyEditor): PartialBlock[] | null {
  const trimmed = raw.trim();
  if (trimmed.length === 0) return [];

  let parsed: unknown;
  try {
    parsed = JSON.parse(trimmed);
  } catch {
    return null;
  }

  if (typeof parsed !== "object" || parsed === null || Array.isArray(parsed)) return null;
  const doc = parsed as Record<string, unknown>;

  // Anchors' own document format. Caught explicitly so the warning below names
  // the actual cause instead of a generic shape mismatch.
  if (typeof doc.blocks === "object" && doc.blocks !== null) {
    console.warn("[anchors] legacy Anchors Document JSON - cannot be shown by BlockNote");
    return null;
  }

  if (doc.type !== "doc" || !Array.isArray(doc.content)) return null;

  const known = new Set(Object.keys(editor.schema.blockSchema));
  const blocks = doc.content as unknown[];
  const usable = blocks.filter((b): b is PartialBlock => {
    if (typeof b !== "object" || b === null) return false;
    const type = (b as { type?: unknown }).type;
    if (typeof type !== "string") return false;
    if (known.has(type)) return true;
    console.warn(`[anchors] dropping block of unknown type "${type}"`);
    return false;
  });

  return usable.length > 0 ? usable : [];
}

/** Replace the entire document. Returns the blocks now in the editor. */
function replaceDocument(editor: AnyEditor, blocks: PartialBlock[]) {
  const current = editor.document.map((b) => b.id);
  editor.replaceBlocks(current, blocks);
}

/** Sample content for `?demo=1`, so the editor can be checked in a browser. */
const DEMO_BLOCKS: PartialBlock[] = [
  {
    type: "heading",
    props: { level: 1 },
    content: "BlockNote inside Qt WebEngine",
  },
  {
    type: "paragraph",
    content: [
      { type: "text", text: "This canvas is a ", styles: {} },
      { type: "text", text: "TypeScript", styles: { bold: true } },
      { type: "text", text: " app rendered by ", styles: {} },
      { type: "text", text: "QWebEngineView", styles: { code: true } },
      { type: "text", text: ". Editing is debounced by ", styles: {} },
      { type: "text", text: `${SAVE_DEBOUNCE_MS}ms`, styles: { bold: true } },
      { type: "text", text: " and persisted by C++.", styles: {} },
    ],
  },
  { type: "heading", props: { level: 2 }, content: "Try it" },
  {
    type: "bulletListItem",
    content: "Type / to open the slash menu",
  },
  { type: "bulletListItem", content: "Markdown-style shortcuts work" },
  {
    type: "checkListItem",
    props: { checked: true },
    content: "Checklist item",
  },
  {
    type: "checkListItem",
    props: { checked: false },
    content: "Autosave is off until you type",
  },
  { type: "codeBlock", props: { language: "cpp" }, content: "QtWebEngineQuick::initialize();" },
  { type: "quote", content: "Crypto and disk writes stay in C++." },
];

// --- component --------------------------------------------------------------

export default function App() {
  const editor = useCreateBlockNote({ schema: schemaWithColumns });

  /**
   * The slash menu has to be supplied by hand because BlockNote builds its item
   * list from a hardcoded enumeration of block types, so `column` and
   * `columnList` would otherwise be invisible in it. The built-in controller is
   * switched off with slashMenu={false} and this one rendered as a child of the
   * view instead - BlockNoteView renders children inside the editor's own
   * contexts, which is what the controller reads.
   *
   * `getItems` replaces the defaults rather than extending them, hence the
   * spread of getDefaultReactSlashMenuItems inside makeSlashMenuItems.
   */
  const slashMenuItems = useMemo(() => makeSlashMenuItems(editor), [editor]);

  const [title, setTitle] = useState("");
  const [theme, setTheme] = useState<ThemeMode>(() =>
    isDemo && window.matchMedia?.("(prefers-color-scheme: light)").matches ? "light" : "dark",
  );
  /** False until the host (or demo mode) has loaded something. */
  const [hasNote, setHasNote] = useState(isDemo);
  /** True while the stored content is a legacy Anchors document. */
  const [legacyContent, setLegacyContent] = useState(false);

  // Refs mirror state that the timers and listeners need without re-binding.
  const editorRef = useRef<AnyEditor>(editor);
  const titleRef = useRef(title);
  const hasNoteRef = useRef(hasNote);
  const legacyRef = useRef(legacyContent);
  const saveTimerRef = useRef<number | null>(null);
  /** Last title handed to the host, so programmatic loads do not echo back. */
  const emittedTitleRef = useRef("");
  /**
   * True while change events must be attributed to a load rather than to the
   * user. Lifted by the first input event inside the canvas, not by a timer -
   * see the note on SUPPRESS_ARM_MS above.
   */
  const suppressedRef = useRef(false);
  /** Backstop timer for suppressedRef. Null once suppression is lifted. */
  const suppressTimerRef = useRef<number | null>(null);
  /**
   * Serialised document as of the last observed state. A change event that
   * leaves this identical is not an edit at all - BlockNote emits some on
   * transactions that do not change the content.
   */
  const baselineRef = useRef("");
  /**
   * Set once the user interacts with the canvas after a load. This is what
   * separates "BlockNote finished normalising" from "the user did something":
   * a content diff alone cannot, because to both a normaliser and a human
   * the document simply changed.
   */
  const touchedRef = useRef(false);
  /** The editor canvas, so input can be attributed to the body vs the title. */
  const canvasRef = useRef<HTMLDivElement | null>(null);

  editorRef.current = editor;
  titleRef.current = title;
  hasNoteRef.current = hasNote;
  legacyRef.current = legacyContent;

  const applyTitle = useCallback((next: string) => {
    titleRef.current = next;
    setTitle(next);
  }, []);

  const pushSave = useCallback(() => {
    bridge.saveNote({
      title: titleRef.current,
      json: serializeDoc(editorRef.current),
    });
  }, []);

  const scheduleSave = useCallback(() => {
    if (saveTimerRef.current !== null) window.clearTimeout(saveTimerRef.current);
    saveTimerRef.current = window.setTimeout(() => {
      saveTimerRef.current = null;
      pushSave();
    }, SAVE_DEBOUNCE_MS);
  }, [pushSave]);

  const cancelPendingSave = useCallback(() => {
    if (saveTimerRef.current !== null) {
      window.clearTimeout(saveTimerRef.current);
      saveTimerRef.current = null;
    }
    if (suppressTimerRef.current !== null) {
      window.clearTimeout(suppressTimerRef.current);
      suppressTimerRef.current = null;
    }
  }, []);

  /**
   * Arm suppression for the next programmatic load. Must be called before any
   * mutation: `replaceDocument` can fire onChange synchronously, and those
   * events have to be recognised as load-caused.
   *
   * A legacy note ignores the backstop timer. Its body is only ever handed to
   * the new format by a real edit inside the canvas - letting a timer lift
   * suppression would hand a late normalisation event the same permission, and
   * that is precisely the overwrite this exists to prevent.
   */
  const suppressChanges = useCallback(() => {
    suppressedRef.current = true;
    touchedRef.current = false;
    baselineRef.current = "";
    if (suppressTimerRef.current !== null) window.clearTimeout(suppressTimerRef.current);
    suppressTimerRef.current = window.setTimeout(() => {
      suppressTimerRef.current = null;
      if (legacyRef.current) return;
      suppressedRef.current = false;
    }, SUPPRESS_ARM_MS);
  }, []);

  const loadNote = useCallback(
    (payload: HostNote) => {
      const ed = editorRef.current;
      cancelPendingSave();

      const incoming = payload?.json ?? "";
      const title = payload?.title ?? "";

      // Must precede every mutation below: replaceDocument() can fire onChange
      // synchronously, and those events have to be recognised as load-caused.
      suppressChanges();

      // An empty title with empty body is the host clearing the canvas (note
      // deselected or deleted) - not a note to edit.
      if (title.length === 0 && incoming.trim().length === 0) {
        replaceDocument(ed, []);
        setLegacyContent(false);
        legacyRef.current = false;
        applyTitle("");
        emittedTitleRef.current = "";
        setHasNote(false);
        hasNoteRef.current = false;
        return;
      }

      const blocks = parseBlockNoteDoc(incoming, ed);

      if (blocks === null) {
        // Legacy (or corrupt) content. Show an empty document but hold the save
        // hostage until the user actually edits, so merely opening the note
        // cannot overwrite what is already on disk.
        replaceDocument(ed, []);
        setLegacyContent(true);
        legacyRef.current = true;
      } else {
        replaceDocument(ed, blocks);
        setLegacyContent(false);
        legacyRef.current = false;
      }

      applyTitle(title);
      emittedTitleRef.current = title;

      setHasNote(true);
      hasNoteRef.current = true;
    },
    [applyTitle, suppressChanges, cancelPendingSave],
  );

  const handleTheme = useCallback((mode: unknown) => {
    setTheme(mode === "light" ? "light" : "dark");
  }, []);

  // Host -> page: push a pending edit immediately. Used when the user switches
  // notes, so an edit still inside the debounce window is not silently dropped.
  const handleFlushSave = useCallback(() => {
    if (saveTimerRef.current === null) return;
    cancelPendingSave();
    pushSave();
  }, [cancelPendingSave, pushSave]);

  // Bridge registration + demo bootstrap. Runs once; the editor instance is
  // stable for the lifetime of the component.
  useEffect(() => {
    installBridge();
    onHostCall("loadNote", (args) => loadNote(args[0] as HostNote));
    onHostCall("setTheme", (args) => handleTheme(args[0]));
    onHostCall("flushSave", () => handleFlushSave());

    if (isDemo) {
      loadNote({ title: "Demo note", json: JSON.stringify({ type: "doc", content: DEMO_BLOCKS }) });
    }

    return () => {
      // Push anything still pending before React drops the editor.
      if (saveTimerRef.current !== null) {
        cancelPendingSave();
        pushSave();
      } else {
        cancelPendingSave();
      }
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // Tell the host we are mounted; from here on queued loadNote/setTheme flush.
  useEffect(() => {
    bridge.ready();
  }, []);

  // Keep html in sync so our own chrome matches BlockNote's colour scheme.
  useEffect(() => {
    document.documentElement.dataset.anchorsTheme = theme;
    document.documentElement.style.colorScheme = theme;
  }, [theme]);

  // Last-chance flush so an edit in the final 800ms is not lost when the
  // WebEngineView is torn down or the app closes. The named handler is kept so
  // it can actually be removed again - an inline arrow here would leak a
  // listener on every re-render of this effect.
  useEffect(() => {
    const onHidden = () => {
      if (document.visibilityState !== "hidden") return;
      // A pending save means the user edited, so flushing is correct even for a
      // note we would otherwise refuse to save.
      if (saveTimerRef.current === null) return;
      cancelPendingSave();
      pushSave();
    };
    window.addEventListener("beforeunload", onHidden);
    document.addEventListener("visibilitychange", onHidden);
    return () => {
      window.removeEventListener("beforeunload", onHidden);
      document.removeEventListener("visibilitychange", onHidden);
    };
  }, [cancelPendingSave, pushSave]);

  // Mark the canvas as user-touched after a load.
  //
  // Deliberately broad: pointerdown and keydown are included because plenty of
  // genuine edits never fire beforeinput - ticking a checkListItem or dragging
  // a block by its handle are click/pointer transactions, not text input.
  // Gating on beforeinput alone made those edits silently unsaveable.
  //
  // Being broad is safe because touching the canvas is only half the signal:
  // handleChange still requires the document to actually differ from the
  // baseline. A stray click therefore marks the canvas touched but saves
  // nothing, which is exactly what we want for a legacy note.
  useEffect(() => {
    const options = { capture: true } as const;
    const onInput = (event: Event) => {
      const target = event.target;
      if (!(target instanceof Node)) return;
      // Scoped to the canvas: typing in the title must not count as editing
      // the body. For a legacy note that is the difference between "retitle
      // it" and "overwrite its contents", because the canvas currently holds
      // an empty document. Title edits persist on their own via titleChanged.
      if (!canvasRef.current?.contains(target)) return;
      touchedRef.current = true;
    };
    const events = [
      "pointerdown",
      "keydown",
      "beforeinput",
      "paste",
      "drop",
      "cut",
      "compositionstart",
    ];
    events.forEach((name) => document.addEventListener(name, onInput, options));
    return () => {
      events.forEach((name) => document.removeEventListener(name, onInput, options));
    };
  }, []);

  const handleChange = useCallback(() => {
    const snapshot = serializeDoc(editorRef.current);

    // Transaction that left the content identical - not an edit.
    if (snapshot === baselineRef.current) return;

    if (suppressedRef.current && !touchedRef.current) {
      // BlockNote normalising after a load. Adopt the new state as the
      // baseline so the next event is judged against settled content.
      baselineRef.current = snapshot;
      return;
    }

    // Genuine edit: stop suppressing for the rest of this note's lifetime.
    suppressedRef.current = false;
    if (suppressTimerRef.current !== null) {
      window.clearTimeout(suppressTimerRef.current);
      suppressTimerRef.current = null;
    }

    // First genuine edit of a legacy note: hand the note over to the new
    // format from here on.
    if (legacyRef.current) {
      legacyRef.current = false;
      setLegacyContent(false);
    }

    scheduleSave();
  }, [scheduleSave]);

  const handleTitleChange = useCallback((next: string) => {
    applyTitle(next);
    if (next === emittedTitleRef.current) return;
    emittedTitleRef.current = next;
    bridge.titleChanged(next);
  }, [applyTitle]);

  const statusText = useMemo(() => {
    if (!hasNote) return "No note selected";
    if (legacyContent) return "Legacy format - your first edit replaces this note";
    return isDemo ? "Demo mode - changes are not saved anywhere" : "Saved automatically";
  }, [hasNote, legacyContent]);

  return (
    <div className="anchors-shell">
      <input
        className="anchors-title"
        value={title}
        placeholder="Title"
        spellCheck={false}
        onChange={(e) => handleTitleChange(e.target.value)}
      />

      <div className="anchors-status" data-testid="status">
        {statusText}
      </div>

      <div className="anchors-canvas" ref={canvasRef}>
        <BlockNoteView
          editor={editor}
          theme={theme}
          editable={hasNote}
          onChange={handleChange}
          slashMenu={false}
        >
          <SuggestionMenuController
            triggerCharacter="/"
            shouldOpen={(state) =>
              !state.selection.$from.parent.type.isInGroup("tableContent")
            }
            getItems={slashMenuItems}
          />
        </BlockNoteView>
      </div>
    </div>
  );
}