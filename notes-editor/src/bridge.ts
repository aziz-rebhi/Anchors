/**
 * Anchors notes editor <-> Qt host bridge.
 *
 * The Qt shell embeds this page in a `WebEngineView` and talks to it over a
 * `QWebChannel`. Two directions exist:
 *
 *   page -> host : `saveNote` / `titleChanged` / `ready`
 *                  Sent over the real WebChannel transport
 *                  (`window.qt.webChannelTransport`, injected by Qt WebEngine).
 *
 *   host -> page : `loadNote` / `setTheme`
 *                  Delivered by `NotesEditorHost` via `QWebEnginePage::runJavaScript`
 *                  into `__dispatch()` below. Using `runJavaScript` for the push
 *                  direction keeps this file down to three WebChannel message
 *                  types (init / invokeMethod / response) instead of also having
 *                  to implement `connectToSignal` bookkeeping. The trade-off is
 *                  that the host cannot await a reply from the page - nothing
 *                  here needs one.
 *
 * Outside Qt (plain `npm run dev` in a browser) every call degrades to a no-op,
 * so `http://localhost:5173/?demo=1` works with no host attached.
 */

// --- WebChannel wire protocol -----------------------------------------------
// Numeric ids come from Qt's qwebchannel.js (QtWebChannel). Stable since 5.12.
const MSG_INIT = 3;
const MSG_IDLE = 4;
const MSG_INVOKE_METHOD = 6;
const MSG_RESPONSE = 10;

/** Name this host object is registered under by `NotesEditorHost`. */
export const HOST_OBJECT_NAME = "anchorsNotes";

export type ThemeMode = "dark" | "light";

/** A note as handed to us by the host. */
export interface HostNote {
  title: string;
  /**
   * BlockNote document JSON. May be empty, may be legacy Anchors Document JSON
   * - callers decide how to handle that, see `parseBlockNoteDoc` in App.tsx.
   */
  json: string;
}

/** The API surface required on `window.anchorsNotes`. */
export interface NotesBridge {
  /** Host -> page. Replaces the editor contents and the title. */
  loadNote(payload: HostNote): void;
  /** Host -> page. */
  setTheme(mode: ThemeMode): void;
  /** Host -> page. Push any debounced edit right now, then clear the timer. */
  flushSave(): void;
  /** Page -> host. Debounced by the caller before it fires. */
  saveNote(payload: { title: string; json: string }): void;
  /** Page -> host. Fire on every keystroke in the title field. */
  titleChanged(title: string): void;
  /** Page -> host. Tells the host the bridge is mounted and may be flushed. */
  ready(): void;
  /** True once a Qt transport has been found. */
  readonly connected: boolean;
}

interface QtTransport {
  send(data: string): void;
  onmessage?: (event: { data: unknown }) => void;
}

type HostHandler = (args: unknown[]) => void;

// Host -> page handlers, keyed by the method name C++ dispatches.
const hostHandlers = new Map<string, HostHandler>();

let transport: QtTransport | null = null;
let channelStarted = false;
/** Flipped to true once Qt answers our init handshake. */
let handshakeDone = false;
/** Monotonic id for page -> host calls; Qt correlates its reply with this. */
let msgId = 1;

function findTransport(): QtTransport | null {
  const qt = (window as { qt?: { webChannelTransport?: QtTransport } }).qt;
  const t = qt?.webChannelTransport;
  if (t && typeof t.send === "function") return t;
  return null;
}

function handleMessage(event: { data: unknown }) {
  let msg: { type?: number; id?: number };
  try {
    msg = (typeof event.data === "string" ? JSON.parse(event.data) : event.data) as {
      type?: number;
      id?: number;
    };
  } catch {
    return;
  }
  if (!msg || typeof msg !== "object") return;
  // We only care about the init reply; it proves the channel is fully wired up
  // and tells us the host has `anchorsNotes` registered.
  if (msg.type === MSG_RESPONSE && msg.id === 0) handshakeDone = true;
}

/**
 * Attach to the Qt transport if present. Safe to call more than once and safe
 * to call outside Qt, where it simply leaves the page in standalone mode.
 */
export function startChannel(): boolean {
  if (channelStarted) return transport !== null;
  channelStarted = true;

  transport = findTransport();
  if (!transport) return false;

  transport.onmessage = handleMessage;
  transport.send(JSON.stringify({ type: MSG_INIT, id: 0 }));
  transport.send(JSON.stringify({ type: MSG_IDLE }));
  return true;
}

function invokeOnHost(method: string, args: unknown[]): boolean {
  if (!transport) return false;
  transport.send(
    JSON.stringify({
      type: MSG_INVOKE_METHOD,
      // Qt 6.11 requires `id` on INVOKE_METHOD. Without it the message is
      // discarded without any reply or warning, so saveNote/ready would never
      // reach the host. Verified by A/B on a live channel: with `id` the slot
      // fires, without it nothing happens.
      id: msgId++,
      object: HOST_OBJECT_NAME,
      method,
      args,
    }),
  );
  return true;
}

export const isConnected = () => transport !== null;
export const isHandshakeComplete = () => handshakeDone;

// --- Public bridge ----------------------------------------------------------

export const bridge: NotesBridge = {
  /** Host -> page. Installed into the object the host dispatches to. */
  loadNote(payload) {
    hostHandlers.get("loadNote")?.([payload]);
  },

  setTheme(mode) {
    hostHandlers.get("setTheme")?.([mode]);
  },

  flushSave() {
    hostHandlers.get("flushSave")?.([]);
  },

  saveNote(payload) {
    invokeOnHost("saveNote", [payload.title, payload.json]);
  },

  titleChanged(title) {
    invokeOnHost("titleChanged", [title]);
  },

  ready() {
    invokeOnHost("ready", []);
  },

  get connected() {
    return transport !== null;
  },
};

/** Register a handler for one of the host -> page calls. */
export function onHostCall(method: string, handler: HostHandler): void {
  hostHandlers.set(method, handler);
}

/**
 * Install `window.anchorsNotes` so `NotesEditorHost` can reach us, and start
 * the channel. Call once, at module load.
 */
export function installBridge(): void {
  const api = {
    loadNote: (payload: HostNote) => bridge.loadNote(payload),
    setTheme: (mode: ThemeMode) => bridge.setTheme(mode),
    flushSave: () => bridge.flushSave(),
    saveNote: (payload: { title: string; json: string }) => bridge.saveNote(payload),
    titleChanged: (title: string) => bridge.titleChanged(title),
    ready: () => bridge.ready(),
    // Live view, not a snapshot: isConnected() is false while this literal is
    // being built because startChannel() has not run yet.
    get connected() {
      return isConnected();
    },

    /**
     * Transport for the host's `runJavaScript` push. `args` arrives as a JSON
     * string because it is inlined into a generated script; it is parsed here
     * so handlers always see real objects.
     */
    __dispatch(method: string, argsJson: string) {
      let args: unknown[] = [];
      try {
        const parsed: unknown = JSON.parse(argsJson);
        args = Array.isArray(parsed) ? parsed : [parsed];
      } catch {
        args = [];
      }
      hostHandlers.get(method)?.(args);
    },
  };

  (window as { anchorsNotes?: unknown }).anchorsNotes = api;
  startChannel();
}