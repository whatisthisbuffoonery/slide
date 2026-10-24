#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p build/linux build/windows dist
common=(-std=c++17 -Wall -Wextra -Werror -O2 -Isrc -Ithird_party)
portable=(src/content/course.cpp src/game/game.cpp src/game/physics.cpp src/audio/wav.cpp)
for name in vec3 vec3_utils; do
    gcc -std=c11 -Wall -Wextra -Werror -O2 -c "src/math/$name.c" -o "build/linux/$name.o"
    x86_64-w64-mingw32-gcc -std=c11 -Wall -Wextra -Werror -O2 -c "src/math/$name.c" -o "build/windows/$name.o"
done
g++ "${common[@]}" tests/tests.cpp "${portable[@]}" build/linux/*.o -o build/slide-tests
./build/slide-tests assets
sources=(src/main.cpp src/app.cpp src/smoke_test.cpp "${portable[@]}" src/platform/windows.cpp src/platform/input.cpp src/audio/audio.cpp src/render/d3d_init.cpp src/render/draw_frame.cpp src/ui/drawing.cpp src/ui/message_cards.cpp src/ui/screens.cpp)
x86_64-w64-mingw32-g++ "${common[@]}" -DUNICODE -D_UNICODE -static -municode -mwindows "${sources[@]}" build/windows/*.o -o dist/Slide.exe -ld3d11 -ldxgi -ld3dcompiler -lwinmm -lgdi32 -luser32
cp -R assets shaders dist/
cp PLAY.txt dist/
cp docs/ASSETS.md dist/ASSETS.md
cp third_party/nlohmann/LICENSE.MIT dist/JSON-LICENSE.txt
cp dist/Slide.exe Slide.exe
echo "Built dist/Slide.exe"
