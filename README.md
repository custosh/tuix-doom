
# Doom Terminal Port

A terminal-based port of the classic Doom engine using the TUIX terminal UI library.

This project runs the core gameplay entirely inside a terminal using TUIX. Certain optional subsystems—such as sound, joystick support and windowing—use SDL2 when available.

---

## Features

- Terminal-first gameplay powered by TUIX
- Optional SDL2-backed subsystems:
    - Windowing / rendering helpers
    - Audio and music (SDL_mixer)
    - Joystick/gamepad support
    - Optional SDL_net support for networking backends
- Cross-platform: Linux, Windows and macOS (where TUIX and SDL2 are available)

---

## Requirements

- C compiler (MSVC, GCC or Clang)
- CMake (recommended build system)
- Optional but recommended: `SDL2` (and `SDL2_mixer`, `SDL2_net`) for sound, joystick and networking
- A POSIX-compatible terminal (or a capable Windows terminal emulator)

If SDL2 is not available, the core terminal gameplay still works; optional features will be disabled.

---

## Building

This project uses CMake. On Windows we recommend using vcpkg for dependencies; on Unix-like systems install the SDL2 development packages via your package manager.

Example (Windows, using vcpkg):

```powershell
# From project root
cmake -S . -B build -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg.cmake
cmake --build build --config Release --paralle l
```

Example (Linux/macOS):

```bash
mkdir build && cd build
cmake ..
cmake --build . -- -j$(nproc)
```

If you want to build without SDL2, pass the appropriate CMake options or install without SDL2. See `CMakeLists.txt` for available options such as `ENABLE_SDL2_MIXER` and `ENABLE_SDL2_NET`.

---

## Running

After a successful build, run the executable from the build output. Example (Unix):

```bash
./build/src/doom/Release/chocolate-doom
```

On Windows use the corresponding `.exe` in `build\src\Release` or the configured output directory.

### Controls

- Movement: Arrow keys
- Fire / Action: Enter
- Pause / Menu: Escape

Keyboard input is the primary control method; mouse support is limited and terminal-dependent.

---

## SDL2 usage

SDL2 is used for:

- Audio and music playback (via `SDL_mixer`) — `src/i_sdlmusic.c`, `src/i_sdlsound.c`
- Joystick and controller support — `src/i_joystick.c`
- Some platform helpers (timers, file system, endian helpers) — various `i_*` modules
- Optional network backend (`SDL_net`) — `src/net_sdl.c`

If you need a build that excludes SDL2, disable the optional CMake flags or remove SDL2 development packages before configuring.

---

## Contributing

Contributions are welcome. Please open issues for bugs or feature requests and submit pull requests for improvements. Follow existing code style and run the test/build steps locally before submitting changes.

---

## License

This project is distributed under the GPL-2.0 license. The original Doom code belongs to id Software and is licensed under GPL-2.0 where applicable.

---

## Acknowledgements

- id Software — original Doom source
- TUIX — terminal UI framework core
- SDL2 — optional subsystems (audio, joystick, networking)
- Chocolate Doom — original Chocolate Doom codebase used as the starting point for this port