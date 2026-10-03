/**
 * Notion-style columns for the notes canvas.
 *
 * BlockNote 0.55 knows about columns but does not ship them. The layout CSS
 * (`.bn-block-column-list` / `.bn-block-column`), the hover and resize classes,
 * and the Tab/Shift-Tab handling across column boundaries all exist in core;
 * the node types that would have produced them lived in a multi-column package
 * that is not published to npm. So the two node types are declared here.
 *
 * Three pieces of core are load-bearing, and all three key on the literal names
 * "columnList" and "column":
 *
 *  - UniqueID is configured with
 *    `types: ["blockContainer", "columnList", "column"]`, so naming the nodes
 *    exactly this is what gives them an `id` attribute.
 *  - blockToNode routes a node that is in the bnBlock group but NOT the
 *    blockContent group down a container path (blockToNode.ts:384-393). That
 *    branch is dead code in the default schema, which is how you can tell it
 *    was written for exactly this case.
 *  - getBlockInfoFromPos resolves a node in the `childContainer` group as its
 *    own child container (getBlockInfoFromPos.ts:230-242), and nodeToBlock
 *    reads `children` from there. This is the part that makes the blocks inside
 *    a column survive a save/load round trip.
 *
 * Renaming either node would compile cleanly and then quietly drop every block
 * inside every column on the next save. The names are load-bearing.
 *
 * --- why these two nodes are shaped the way they are ------------------------
 *
 * Both nodes were originally declared with `group: "blockGroupChild bnBlock
 * childContainer"` and `content: "bnBlock+"`. That combination is fatal, and it
 * was not a theoretical schema smell - it broke every legacy note:
 *
 *  1. `column` is in the `bnBlock` group AND requires `bnBlock+`, so a column is
 *     a valid child of a column. ProseMirror's ContentMatch.fillBefore walks
 *     the possible node types for a position and, once it finds a match, calls
 *     createAndFill() on each type it picked (prosemirror-model
 *     fillBefore, `Fragment.from(types.map(tp => tp.createAndFill()))`). When a
 *     type can contain itself, createAndFill -> fillBefore -> createAndFill
 *     never bottoms out and the stack overflows. This fires wherever a node has
 *     to be auto-created, which is to say on every empty document - and
 *     `replaceDocument(editor, [])` produces exactly that when a legacy note
 *     loads.
 *
 *  2. The RangeError escaped loadNote() before it set the title, so the note came
 *     up blank, read-only (editable is bound to hasNote) and unable to save.
 *     Nothing in the UI said why.
 *
 *  3. Fixing only the content expression is not enough. Both nodes would still
 *     be *fillable*, and custom specs are ordered ahead of the blockContainer
 *     that core appends last, so `blockGroup`'s `blockGroupChild+` would resolve
 *     to columnList. Every empty document then auto-filled as
 *     columnList > column > paragraph, and the first save of any legacy note
 *     wrote that shape to disk. Silently wrong beats loudly broken here.
 *
 * So the content expression excludes the column types, and columnList carries a
 * sentinel attribute that makes it non-generatable. ProseMirror skips any type
 * with required attributes in both fillBefore and findWrapping, which is
 * exactly the semantics wanted: an auto-created column list is never correct,
 * but a user-created one always is. `column` needs no such attribute, because
 * its only parent is a columnList, and a columnList is never auto-created.
 *
 * The sentinel is not a setting - it exists only to make the attribute required.
 * It has to live in propSchema as well, because nodeToBlock only copies
 * attributes that propSchema knows about; without that it would be dropped on
 * save and the next load would throw "No value supplied for attribute".
 *
 * The one cost of this is that a columnList in stored JSON must carry the
 * sentinel. Every columnList this editor writes does, because makeColumnsBlock
 * is the only thing that creates one and nodeToBlock copies the attribute back
 * out on save. Hand-written columnList JSON without it will fail to load -
 * there is no way to have both a required attribute and a load-time default,
 * because hasRequiredAttrs() reads the same spec that supplies the default.
 */
import { Node } from "@tiptap/core";
import {
  BlockNoteSchema,
  createBlockConfig,
  createBlockSpec,
  UniqueID,
  type BlockImplementation,
  type BlockNoteDOMAttributes,
  type BlockNoteEditor,
  type PartialBlock,
} from "@blocknote/core";

type ColumnNodeOptions = {
  editor: BlockNoteEditor<any, any, any>;
  domAttributes?: BlockNoteDOMAttributes;
};

/**
 * Build the single element that serves as both a container node's `dom` and its
 * `contentDOM`.
 */
function containerDom(
  className: string,
  type: string,
  HTMLAttributes: Record<string, unknown>,
): HTMLDivElement {
  const dom = document.createElement("div");
  dom.className = className;
  dom.setAttribute("data-node-type", type);
  for (const [name, value] of Object.entries(HTMLAttributes)) {
    if (name === "class" || value === null || value === undefined) continue;
    dom.setAttribute(name, String(value));
  }
  return dom;
}

/**
 * Attribute whose *absence of a default* is the load-bearing part.
 *
 * Only columnList has one, and that is exactly enough. It is the node ProseMirror
 * was auto-creating for an empty document, so making it non-generatable is what
 * stops an empty document from becoming a one-column layout. A column can never
 * be auto-created in the first place - its only parent is a columnList.
 *
 * blockToNode() spreads `...block.props` into the node's attributes, and
 * nodeToBlock() copies node attributes back into props - but only those listed
 * in propSchema. So this name has to appear in both places, and makeColumnsBlock
 * is the only thing that supplies it. See the header comment.
 */
const MARKER_ATTR = "anchorsColumn";

/** The only value MARKER_ATTR ever takes. Its content is irrelevant. */
const MARKER_VALUE = "manual";

/**
 * A single column: holds any number of ordinary blocks.
 *
 * `bnBlock` is what blockToNode and getBlockInfoWithManualOffset test for;
 * `childContainer` is what makes this node its own parent for `children`.
 *
 * Deliberately NOT in `blockGroupChild`. A column's only parent is a columnList,
 * so it never has to be a direct child of blockGroup - and staying out of that
 * group is what keeps it away from blockGroup's `blockGroupChild+` fill. See
 * the header comment for why that matters.
 *
 * `content` names `blockContainer` rather than the `bnBlock` group. `bnBlock` is
 * a superset that includes this very node, which is what made the fill
 * recursion unbounded; naming the container type directly states the real
 * intent - a column holds ordinary blocks, never another layout.
 */
const columnNode = Node.create<ColumnNodeOptions>({
  name: "column",
  group: "bnBlock childContainer",
  content: "blockContainer+",
  defining: true,
  isolating: true,

  // No required attribute here, unlike columnList. It cannot have one:
  // columnList requires `column+`, and ProseMirror's checkForDeadEnds rejects
  // the whole schema when a required position can only be filled by a node with
  // required attributes ("Only non-generatable nodes in a required position").
  // Staying generatable is safe because no fill path can reach a column: only
  // a columnList holds one, and a columnList is never auto-created.
  parseHTML() {
    return [{ tag: 'div[data-node-type="column"]' }];
  },

  renderHTML({ HTMLAttributes }) {
    const dom = containerDom("anchors-col", "column", HTMLAttributes);
    return { dom, contentDOM: dom };
  },
});

/** The row that holds the columns side by side. */
const columnListNode = Node.create<ColumnNodeOptions>({
  name: "columnList",
  group: "blockGroupChild bnBlock childContainer",
  content: "column+",
  defining: true,
  isolating: true,

  // `isRequired` (not an omitted `default`) is how Tiptap is asked for a
  // ProseMirror attribute with no default: getSchemaByResolvedExtensions only
  // copies `default` onto the spec when isRequired is false, so
  // hasRequiredAttrs() comes out true and fillBefore/findWrapping skip this
  // type. Merging `{}` instead would not work - Tiptap fills in `default: null`,
  // the node becomes fillable again, and an empty document resolves to
  // columnList because custom specs are ordered ahead of blockContainer.
  addAttributes() {
    return { [MARKER_ATTR]: { isRequired: true, rendered: false } };
  },

  parseHTML() {
    return [{ tag: 'div[data-node-type="columnList"]' }];
  },

  renderHTML({ HTMLAttributes }) {
    const dom = containerDom("anchors-col-list", "columnList", HTMLAttributes);
    return { dom, contentDOM: dom };
  },
});

/**
 * Attach a hand-written ProseMirror node to a block implementation.
 *
 * `node` is not part of BlockImplementation's public type, but
 * addNodeAndExtensionsToSpec reads it first and uses it verbatim instead of
 * deriving a node from the config (createSpec.ts:182-183). It is the only way
 * into the bnBlock / childContainer groups: the derived path hardcodes
 * `group: "blockContent"` (createSpec.ts:208), which is the wrong shape for a
 * container - blockToNode would wrap it in a blockContainer and look for its
 * children in a sibling blockGroup that nothing creates.
 */
function withNode<T extends Record<string, unknown>>(
  implementation: T,
  node: Node,
): T & { node: Node } {
  return { ...implementation, node };
}

const columnListBlock = createBlockSpec(
  createBlockConfig(() => ({
    type: "columnList" as const,
    // Declared so nodeToBlock copies MARKER_ATTR back into props on save. The
    // default here is irrelevant to ProseMirror - the node attribute comes from
    // addAttributes() above - but nodeToBlock reads `default` to decide whether
    // an undefined value is meaningful, so it has to be a real value.
    propSchema: { [MARKER_ATTR]: { type: "string" as const, default: "" } },
    // Unused for container blocks - the real content expression lives on the
    // node above. createBlockSpec only accepts "inline" | "none" | "plain" here,
    // so "none" is the honest value rather than a cast that pretends otherwise.
    content: "none" as const,
  })),
  withNode(
    {
      meta: { isolating: true },
      parse() {
        // Columns have no external HTML form. Anything pasted that claims to be
        // one falls through to plain text rather than becoming a broken column.
        return undefined;
      },
      render() {
        const dom = document.createElement("div");
        dom.className = "anchors-col-list";
        return { dom, contentDOM: dom };
      },
    } satisfies BlockImplementation<"columnList", {}, "none">,
    columnListNode,
  ),
);

const columnBlock = createBlockSpec(
  createBlockConfig(() => ({
    type: "column" as const,
    // No marker prop: a column is never auto-created, so it needs no way to
    // tell a generated one from a real one.
    propSchema: {},
    content: "none" as const,
  })),
  withNode(
    {
      meta: { isolating: true },
      parse() {
        return undefined;
      },
      render() {
        const dom = document.createElement("div");
        dom.className = "anchors-col";
        return { dom, contentDOM: dom };
      },
    } satisfies BlockImplementation<"column", {}, "none">,
    columnNode,
  ),
);

/** The default schema plus columns. Passed to `useCreateBlockNote({ schema })`. */
export const schemaWithColumns = BlockNoteSchema.create().extend({
  blockSpecs: { columnList: columnListBlock(), column: columnBlock() },
});

/**
 * A block from the extended schema, before it reaches the editor's generics.
 * The default `PartialBlock` is a discriminated union over the built-in block
 * types, which cannot describe a `column`.
 */
type AnyBlock = PartialBlock<any, any, any>;

/** An empty column holding one empty paragraph - enough to type into. */
function makeColumn(): AnyBlock {
  return {
    id: UniqueID.options.generateID(),
    type: "column",
    props: {},
    children: [
      {
        id: UniqueID.options.generateID(),
        type: "paragraph",
        props: {},
        content: [],
        children: [],
      },
    ],
  } as unknown as AnyBlock;
}

/** A column list of `count` equal-width columns, each with an empty paragraph. */
export function makeColumnsBlock(count: 2 | 3): AnyBlock {
  return {
    id: UniqueID.options.generateID(),
    type: "columnList",
    // Required attribute: blockToNode() calls create(), not createChecked(),
    // but computeAttrs() throws on a missing required attribute either way. This
    // is the only place a columnList is born, so it is the only place that has
    // to know.
    props: { [MARKER_ATTR]: MARKER_VALUE },
    children: Array.from({ length: count }, makeColumn),
  } as unknown as AnyBlock;
}

/** Flat inline text of a block, so emptiness can be tested. */
function blockText(block: { type: string; content?: unknown }): string {
  if (block.type !== "paragraph") return "";
  if (typeof block.content === "string") return block.content;
  if (!Array.isArray(block.content)) return "";
  return block.content
    .map((part) =>
      part && typeof part === "object" && "text" in part
        ? String((part as { text: unknown }).text)
        : "",
    )
    .join("");
}

/**
 * Insert a column layout at the cursor.
 *
 * The block the slash menu was opened on is consumed when it is empty, so
 * "/columns" does not leave a stray blank paragraph above the columns the way
 * it would if we only ever inserted. A paragraph with real text in it is kept
 * and the columns go below it.
 */
export function insertColumns(
  editor: BlockNoteEditor<any, any, any>,
  count: 2 | 3,
): void {
  const cursor = editor.getTextCursorPosition();
  const current = cursor.block;
  const columns = makeColumnsBlock(count);

  editor.insertBlocks([columns], current, "after");

  if (blockText(current).trim() === "") {
    editor.removeBlocks([current.id]);
  }

  // Drop the caret into the first column rather than leaving it wherever the
  // removed block used to be.
  const firstChild = columns.children?.[0]?.children?.[0];
  if (firstChild?.id) editor.setTextCursorPosition(firstChild.id, "start");
}