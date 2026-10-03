/**
 * Slash-menu entries for the column layouts.
 *
 * BlockNote builds its slash menu from a hardcoded list of block types
 * (getDefaultSlashMenuItems.ts), so a custom block type does not appear in the
 * menu on its own. Adding an item therefore means taking over the list rather
 * than extending it - see the `slashMenu={false}` in App.tsx and the
 * SuggestionMenuController rendered alongside the view.
 *
 * The alternative was a keybinding or an input rule, both of which stay
 * undiscoverable. The user's call was slash menu, and the reason it was chosen
 * over a visible toolbar is that permanent furniture works against the Notion
 * look the rest of the canvas is built for.
 */
import { filterSuggestionItems } from "@blocknote/core/extensions";
import type { BlockNoteEditor } from "@blocknote/core";
import {
  getDefaultReactSlashMenuItems,
  type DefaultReactSuggestionItem,
} from "@blocknote/react";

import { insertColumns } from "./columns";

/** An N-column glyph: N vertical bars, matching what gets inserted. */
function ColumnsIcon({ count }: { count: 2 | 3 }) {
  const bar = 2.5;
  const gap = 3;
  const width = count * bar + (count - 1) * gap;
  return (
    <svg width={18} height={18} viewBox={`0 0 ${width} 16`} fill="none" aria-hidden="true">
      {Array.from({ length: count }, (_, i) => (
        <rect
          key={i}
          x={i * (bar + gap)}
          y={0}
          width={bar}
          height={16}
          rx={1}
          fill="currentColor"
        />
      ))}
    </svg>
  );
}

/**
 * The keys are not in BlockNote's dictionary - `DefaultReactSuggestionItem`
 * leaves `key` out of its type because the core items carry it through a spread.
 * These are declared with it explicitly so the list can be typed locally
 * without an excess-property fight, and concatenated with the built-in items
 * as plain variables rather than one big literal, for the same reason.
 */
type SlashItem = DefaultReactSuggestionItem & { key: string };

const COLUMN_ITEMS: Record<2 | 3, Omit<SlashItem, "onItemClick">> = {
  2: {
    key: "columns_2",
    title: "2 columns",
    subtext: "Side by side",
    aliases: ["columns", "column", "2 columns", "side by side", "split"],
    group: "Layout",
    icon: <ColumnsIcon count={2} />,
  },
  3: {
    key: "columns_3",
    title: "3 columns",
    subtext: "Side by side",
    aliases: ["columns", "column", "3 columns", "side by side", "split"],
    group: "Layout",
    icon: <ColumnsIcon count={3} />,
  },
};

/**
 * `getItems` for SuggestionMenuController: the built-in items plus the column
 * layouts, filtered by the typed query.
 *
 * The filter is BlockNote's own, so the column entries rank against the
 * built-ins by exactly the same rules instead of being matched separately.
 */
export function makeSlashMenuItems(editor: BlockNoteEditor<any, any, any>) {
  const defaults = getDefaultReactSlashMenuItems(editor);

  const extras = (Object.keys(COLUMN_ITEMS) as unknown as (2 | 3)[]).map((count) => ({
    ...COLUMN_ITEMS[count],
    onItemClick: () => insertColumns(editor, count),
  }));

  const items: DefaultReactSuggestionItem[] = [...defaults, ...extras];

  return async (query: string) => filterSuggestionItems(items, query);
}