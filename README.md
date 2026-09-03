# Color Picker

## Build
- Needs CMake 3.16+ and a C++17 compiler. 
- GLFW and ImGui are fetched automatically by CMake
- no manual setup, but you need internet access on first configure.

Linux (needs X11/OpenGL dev headers, e.g. `sudo apt install libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`):
```
cmake -B build
cmake --build build -j
./build/color_picker
```

Windows (MSVC) / macOS: same two `cmake` commands, run from a dev shell (VS "x64 Native Tools" on Windows).

## Usage
- Type a hex code (e.g. `357AB7`) or drag the RGB sliders
- "Max colors to compare" sets the cap (min 2).
- "Add to comparison" pushes the current color into the palette list below.
- "Compare side by side" opens a window showing all saved colors with hex + RGB.
- "remove" next to a palette entry deletes it
