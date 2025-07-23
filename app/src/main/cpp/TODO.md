# ✅ TODO List for Game Boy Emulator

_Target Date: **Before August 7**_

---

## 🔥 Must-Have – Core Emulator Functionality

These are essential to make the emulator playable and stable.

- [ ] **Finish core emulator logic**
  - [ ] Implement and finalize the **PPU**
  - [ ] Fix **slow recomposition / startup delay** (e.g., warm-up frame buffer)
  - [ ] Use `DisposableEffect` to **stop emulator when back is pressed**
  - [ ] Fix **Canvas border cropping**
  - [ ] Add **gap at the top** and bring buttons closer

---

## ⚙️ Performance & Stability Improvements

Improve speed, responsiveness, and avoid unnecessary computation.

- [ ] Cache missing images as **null** to avoid repeated failed lookups
- [ ] Add **loading screen** for ROMs
- [ ] Add **fuzzy search module** for ROMs (optimize over O(N²))
- [ ] Add **recursive file reading** for ROM discovery
- [ ] On startup, **check if ROM directory exists**; if not, perform **factory reset**
- [ ] Ensure **factory reset clears all caches**

---

## 🎮 Emulator Features

Non-essential but valuable improvements for users.

- [ ] Save **save-files** in the same directory as ROMs; create directory if needed
- [ ] Add **edge-to-edge layout**
- [ ] Add **additional color palettes**
- [ ] Add **music support**

---

## 📡 Link Cable Emulation (Advanced)

Advanced features for multiplayer emulation.

- [ ] Add **Link Cable emulation**
  - [ ] Support **Bluetooth**
  - [ ] Support **Wi-Fi**

---

## 🚀 Final Touches & Publishing

Finishing steps for a public release.

- [ ] Add **file sharing** support
- [ ] **Publish app to Play Store**

---
