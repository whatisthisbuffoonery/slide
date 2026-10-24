# Slide: readable Windows version

The previous attempt is preserved in ../windows-archive-2026-10-23. This folder is a new modular application, not the archive renamed back.

Run **Slide.exe in this folder** for development. Its editable assets and shaders are beside it. For sharing, use dist/ or Slide-Windows.zip.

## Change a course without compiling

Each course has its own folder:

    assets/courses/01-work-chat/
        course.json
        messages.json
        audio/
            music.wav
            jump.wav
            checkpoint.wav
            defeat.wav
            victory.wav

Edit the JSON or replace a WAV, then press **F5** in the game. Reload restarts the current course and keeps completed-course history. If an edit is invalid, an error identifies the file and the existing course stays available, paused.

See [the asset guide](docs/ASSETS.md) for exact fields, message selection, audio formats, and adding courses.

## Read the code in this order

1. [src/app.cpp](src/app.cpp): the caller flow. Input, simulation, sound events, then drawing.
2. [src/game/game.hpp](src/game/game.hpp) and [game.cpp](src/game/game.cpp): state, checkpoints, failures and consequence history.
3. [src/game/physics.cpp](src/game/physics.cpp): explicit vec_add / vec_scale / vec_dot movement and collision maths.
4. [src/content/course.cpp](src/content/course.cpp): JSON becomes a Course.
5. [shaders/draw_frame.hlsl](shaders/draw_frame.hlsl): per-pixel entry, ray direction, hit selection and shading.
6. [src/render/d3d_init.cpp](src/render/d3d_init.cpp): graphics API setup, when you need it.
7. [src/audio](src/audio): WAV decoding is separate from Windows playback.

The application loop deliberately calls named steps:

    handle_controls()
    advance_simulation()
    dispatch_sound_events()
    draw_frame()

UI drawing and individual screens live in src/ui/. Automated diagnostic transitions live in src/smoke_test.cpp, outside gameplay.

## What comes from miniRT

See [docs/ORIGINS.md](docs/ORIGINS.md) for an exact accounting. Your original C vector files are compiled into the game and used by physics and camera maths. The GPU pipeline translates recognizable sphere-intersection, ray-direction, and Phong operations. MLX, the scene parser, and the full original object/material system are not silently claimed as ports.

## Build

From WSL:

    cd ~/slide/windows
    bash build-wsl.sh

This builds and runs portable tests, cross-compiles Windows, and writes both Slide.exe and dist/. Requires gcc/g++, MinGW-w64, and bash; JSON is vendored, so there is no package download during a build.

On native Windows, use build.ps1 with CMake and a C/C++ toolchain (Visual Studio 2022 C++ workload or MinGW-w64). The prebuilt executable needs neither.

    cmake -S . -B build-cmake
    cmake --build build-cmake --config Release
    ctest --test-dir build-cmake -C Release --output-on-failure

## Check content or rendering

    Slide.exe --validate
    Slide.exe --smoke

Validation reads every course, message pool and WAV, writing validation.log. Smoke mode renders the title, courses, message/fall defeats, and aftermath into BMP captures, while muted and hidden. Failures are recorded in error.log.

The tests cover physics, BVH leaf coverage, course traversal, message history, original C vector integration, adding message IDs without rebuilding, wildcard selection, invalid references, duplicate IDs, and audio loading.

## Deliberate limits

- Compute ray tracing through Direct3D 11, not DXR. No ray-tracing cores required.
- Procedural platform meshes and analytic player/beacon spheres; no imported mesh assets.
- Message pools are dynamic. A course currently supports four active message cards and three checkpoints, matching the GPU buffers. Invalid counts produce errors.
- Recaps paginate with Page Up / Page Down.
- WAV music loops; effects play once. MP3/OGG and 24-bit WAV are not decoded.
- Fixed 1280x720 window, with a 960x540 render texture.
- This fresh version keeps run timing and consequences but does not yet persist personal-best records.

Generated placeholder WAVs are original project assets. The third-party JSON parser is nlohmann/json 3.11.3, MIT licensed; see third_party/nlohmann/LICENSE.MIT.
