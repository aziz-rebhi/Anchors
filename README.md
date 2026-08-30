# Anchors

**Your private workspace. Fully offline. Fully yours.**

Anchors is a desktop app for notes, passwords, tasks, and calendar — encrypted on your machine with a master password. No account. No cloud. No telemetry.

[![Release](https://img.shields.io/github/v/release/aziz-rebhi/Anchors?style=flat-square)](https://github.com/aziz-rebhi/Anchors/releases/latest)
[![License](https://img.shields.io/badge/license-TBD-lightgrey?style=flat-square)](#license)

**[Download the latest release](https://github.com/aziz-rebhi/Anchors/releases/latest)** · [Report an issue](https://github.com/aziz-rebhi/Anchors/issues)

---

## Why Anchors?

Most productivity and password tools quietly depend on servers you don’t control. Anchors takes the opposite path:

| Principle | What it means |
|-----------|----------------|
| **Local-first** | Data lives on your disk, not in someone else’s cloud |
| **Encrypted by default** | Protected with your master password (Argon2id + libsodium) |
| **Offline by design** | Core features work with no network |
| **No account** | Nothing to register, subscribe to, or leak |
| **Yours to keep** | Export and back up your data folder anytime |

If you care about privacy without giving up a modern desktop experience, Anchors is built for you.

---

## Features

| Module | What you get |
|--------|----------------|
| **Vault** | Password entries, quick copy, organized storage |
| **Notes** | Block-based editor — headings, lists, code, tables, callouts, columns, and more |
| **To-Do** | Tasks grouped into projects with emoji and color |
| **Calendar** | Events and day planning |
| **Dashboard** | A simple overview of your workspace |
| **Settings** | Theme, accent, start page, auto-lock, backup export/import |

**Security controls:** session lock, optional auto-lock on idle, lock when minimized, and configurable clipboard behavior.

> **Status:** Actively developed (beta). Core workflows are usable; always keep backups of your data folder.

---

## Download & install

Official builds: **[GitHub Releases](https://github.com/aziz-rebhi/Anchors/releases/latest)**

| Platform | File | Best for |
|----------|------|----------|
| **Windows** | `Anchors-Setup-*.exe` | Normal install, Start Menu, optional desktop shortcut |
| **Windows** | `Anchors-windows-x64.zip` | Portable use (no installer) |
| **Linux** | `Anchors.flatpak` | Install from the app menu on most distros |
| **Linux** | `Anchors-linux-x86_64` | Dev/test binary (needs system Qt 6 + libsodium) |

### Windows (recommended)

1. Download **`Anchors-Setup-x.y.z.exe`**.
2. Run the installer and choose an install folder.
3. Optionally enable **Create a desktop shortcut**.
4. Launch **Anchors** and create your **master password** on first run.

**Upgrading:** download the newer Setup and run it. You do not need to uninstall first. Your data stays in the app data folder.

### Linux (Flatpak — recommended)

```bash
# Requires Flatpak on your system
flatpak install --user ./Anchors.flatpak
flatpak run org.anchors.Anchors
