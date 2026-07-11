# Anchor

A fully offline, local-first desktop application that brings together password
management, note-taking, task tracking, calendar events, and resume building —
all encrypted, all on your machine, with zero network dependency by design.

> ⚠️ **Status: early development.** Core crypto is implemented and tested, but
> most features below are not built yet. Not ready for real use.

## Why

Most productivity and password tools quietly depend on the cloud. Anchor takes
the opposite approach: everything stays local, encrypted with your master
password, with no account, no sync server, and no telemetry. If you value
owning your data as much as protecting it, this is built for that.

## Stack

- **Language:** C++17
- **UI:** Qt6 Widgets
- **Encryption:** [libsodium](https://libsodium.gitbook.io/doc/) — Argon2id
  for key derivation, XSalsa20-Poly1305 (`crypto_secretbox`) for authenticated
  encryption
- **Storage:** No database. Flat encrypted files, one per module (`vault.enc`,
  `notes.enc`, `tasks.enc`, `calendar.enc`, `resume.enc`), each holding a
  JSON blob encrypted as a whole
- **Network:** None. Anchor never makes a network request.

## Architecture

```
UI  →  Repository  →  CryptoManager + EncryptedFileStore
```

- `core/` has zero UI dependencies — pure logic, testable in isolation.
- `ui/` never touches encryption or storage directly, only repositories.
- Each `.enc` file layout: `[salt (16 bytes)][nonce (24 bytes)][ciphertext + auth tag]`
- Writes are atomic (`file.enc.tmp` → rename) to avoid corruption on crash.

## Current status

- [x] Crypto module (`CryptoManager`, `SecureBuffer`) — implemented and
      tested (7/7 tests passing)
- [ ] Encrypted file storage (`EncryptedFileStore`)
- [ ] Login screen / session handling / auto-lock
- [ ] Vault (passwords) — categorized by Social, Work, Learning, Entertainment
- [ ] Notes
- [ ] Tasks (to-do)
- [ ] Calendar
- [ ] Resume builder + PDF export
- [ ] Collapsible sidebar navigation + dashboard

## Design

Color palette — "Green Abyss":

| Role | Color |
|---|---|
| Deep green (primary/accent) | `#0B3D0B` |
| Medium green (secondary accent) | `#2E8B57` |
| Soft green (highlights) | `#4F9F4F` |
| Dark gray (background) | `#1F1F1F` |
| Soft neutral (text/cards) | `#D1E7D1` |

## Building

Requires Qt6, a C++17 compiler, and libsodium installed (`libsodium-dev` /
`libsodium` depending on your distro's package manager).

Open `Anchors.pro` in Qt Creator and build, or via command line:

```bash
qmake Anchors.pro
make
```

## Running tests

The crypto test suite is a separate qmake project. Open `Tests/Tests.pro` in
Qt Creator (File → Open File or Project), select it as the active project,
build, and run.

Expected output:

```
PASS: "deriveKey() produces a 32-byte key"
PASS: "encrypt() produces non-empty output"
PASS: "decrypt() round-trip matches the original plaintext exactly"
PASS: "wrong master password fails decryption cleanly (no partial data)"
PASS: "tampered ciphertext is rejected (authentication catches it)"
PASS: "encrypting identical plaintext twice yields different ciphertext (nonce randomness)"
PASS: "garbage/too-short input is rejected without crashing"

Results: 7 passed, 0 failed
```

## Security notes

- Every write to disk is encrypted; nothing sensitive is ever stored in
  plaintext.
- Session keys are held in `SecureBuffer` (libsodium guarded memory) and
  wiped on auto-lock or app close.
- Losing your master password means losing access to your data — there is
  no recovery mechanism, by design.
- This project has not been independently audited. Treat it accordingly
  until it has.

## License

TBD — leaning toward an open-core model: core app under MIT or GPLv3, with
optional paid features (e.g. sync) decided later.

## Backup

Your `.enc` files under the app's data directory *are* your backup — copying
them elsewhere is sufficient. No separate export tool is needed.
