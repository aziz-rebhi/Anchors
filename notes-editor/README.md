# Anchors Notes Editor

The notes **editor canvas** only. The rest of Anchors (sidebar, Vault, Calendar,
Todo, Dashboard, Settings, Lock) stays in Qt/QML.

This package is a Vite + React + TypeScript app using
[BlockNote](https://www.blocknotejs.org), rendered inside a `QWebEngineView` by
the Qt shell. Encryption, the note repository, and all disk I/O stay in C++ —
the page only ever sees decrypted JSON while a note is open.

## Run it

```bash
npm install
npm run dev          # http://localhost:5173/  (strictPort: never hops ports)
```

Check the editor on its own, without Qt:

```
http://localhost:5173/?demo=1
```

`?demo=1` loads sample content and reports "Demo mode - changes are not saved
anywhere". Without a Qt host attached the bridge is a no-op, so the page is
fully usable in a plain browser.

## Build

```bash
npm run build        # typecheck + bundle to dist/
npm run preview
```

`vite.config.ts` sets `base: "./"`, so `dist/` can be loaded from a `qrc:` URL
without a server. Wiring that into the Qt build is still TODO — see below.

## Bridge contract

`src/bridge.ts` owns both directions.

| Direction | Method | Payload |
| --- | --- | --- |
| host → page | `loadNote({ title, json })` | replaces document + title |
| host → page | `setTheme("dark" \| "light")` | switches BlockNote theme |
| page → host | `saveNote({ title, json })` | debounced 800 ms |
| page → host | `titleChanged(title)` | every title keystroke |
| page → host | `ready()` | bridge mounted; host may flush |

**page → host** goes over the real `QWebChannel`
(`window.qt.webChannelTransport`, injected by Qt WebEngine) using a minimal
client that implements only the `init` / `invokeMethod` / `response` message
types — about 60 lines, no vendored `qwebchannel.js`.

**host → page** is pushed with `QWebEnginePage::runJavaScript` from
`NotesEditorHost`, because WebChannel can only deliver *signals* to a page that
has opted in via `connectToSignal`. Nothing in this direction needs a reply, so
the extra handshake would be dead weight.

`ready()` is the handshake that makes the round trip reliable: a note selected
while the WebEngine view is still painting is stored on the host's `pendingTitle`
/ `pendingJson` properties and replayed the moment the page reports in.

## Document format

Notes store **BlockNote JSON** — `{ "type": "doc", "content": [ ...blocks ] }` —
directly in the note's content column.

Pre-existing notes hold the old Anchors Document JSON (`{ id, title, blocks }`).
Those shapes are close enough to be tempting and different enough to corrupt
data, so `parseBlockNoteDoc()` returns `null` for anything that is not
`type: "doc"` and the page shows:

> Legacy format - your first edit replaces this note

It renders an empty document and **suppresses `saveNote` entirely** until the
first real edit. Merely opening a legacy note therefore cannot destroy it. When
you do start typing, the note is converted to BlockNote JSON from that point on.

There is no automatic converter yet, so legacy content is not preserved when
converted — it is replaced by whatever you type. Write a converter before
migrating real vaults.

## Debounce

Edits are coalesced for 800 ms, then pushed as one full document. A flush also
fires on `beforeunload` and when the document is hidden, so an edit in the final
800 ms before the view is torn down is not lost.

To avoid mistaking a programmatic load for user input, `App.tsx` keeps
`baselineRef` — the serialization taken immediately after each `loadNote()`.
`onChange` ignores any event whose serialization still matches the baseline.
This is timing-independent, unlike a `suppressNotify` flag, which would race
BlockNote's async change events.

## Production TODO

1. `npm run build`, then add `dist/` to a `.qrc` and point `editorUrl` at
   something like `qrc:/notes-editor/index.html`.
2. `WebEngineView` needs the local content origin declared for that URL, and the
   `qwebchannel` origin allowlist has to cover the qrc scheme.
3. Decide on a converter for legacy Anchors Document JSON before migrating
   existing vaults.