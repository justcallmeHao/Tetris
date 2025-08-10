# 🎮 Tetris — C++/raylib (Game Boy–Inspired)

> _“Simple rules. Endless mastery.”_  
> This project is a clean, modern take on **Tetris**, written in **C++** with **raylib**, inspired by the iconic **Game Boy Tetris (1989)** vibe

---

## 🧱 About
A lightweight Tetris that keeps the classic feel while adding a polished UI: ghost piece, next & hold, soft/hard drop, lock delay, levels & scoring, pause/restart, resizable window, optional fullscreen, and optional music/SFX.

- Inspired by the **look-and-feel** of **Game Boy Tetris** 
- Techstack: **raylib**, **C++**

---

## 🕹️ How to Play
- **← / →** Move  
- **↓** Soft drop  **Space** Hard drop  
- **↑ / Z** Rotate (CW / CCW)  
- **Shift** Hold / Swap  
- **P** Pause  **R** Restart  **F11** Fullscreen  
- **M** Mute the music
- **Top-out** (any locked blocks above the top row) ⇒ **Game Over**

> Tip: Hard dropping rewards more points the further you drop.

---

## ✨ Features
- 7‑bag RNG • Ghost piece • Lock delay  
- Next queue & Hold piece (once per spawn)  
- Classic scoring (40/100/300/1200 × level), level up every 10 lines  
- Resizable window, **F11** fullscreen, smooth letterbox scaling (optional)  
- Background music (streamed **.ogg**) + instant SFX (e.g., **.wav**)
---

## 📺 Demo

<video src="README/demo.H264" controls width="720" playsinline></video>


---

## ⚙️ Quick Setup

### 1) Get raylib (no package manager)
Download a prebuilt from the raylib Releases for your OS (MinGW/MSVC/macOS/Linux).
This repo also included the necessary **raylib** folder for Windows x64.

```
project/
  raylib/
    include/   (raylib.h, ...)
    lib/       (libraylib.a  or  raylib.dll + raylib.dll.a on Windows)
```

### 2) Project layout
```
project/
  CMakeLists.txt
  src/
    main.cpp
  raylib/...
  assets/
    theme.ogg    (lengendary Tetris music)
    drop.wav     (SFX: hard drop)
    clear.wav    (SFX: line clear)
```

### 3) Minimal CMake

See CMakeLists.txt

---

## 🔊 Audio Notes
- **Music:** Use `LoadMusicStream("assets/theme.mp3")` and call `UpdateMusicStream(music)` each frame.  
- **SFX:** Use `LoadSound("assets/drop.wav")` / `LoadSound("assets/clear.wav")` and `PlaySound(...)`.  
- Prefer **.mp3** for background music (streamed), **.wav** for SFX (low-latency).

---

## 🙌 Credits & Inspiration
- Built using **[raylib](https://www.raylib.com/)**.  
- This project is **not affiliated** with or endorsed by Nintendo/The Tetris Company.

