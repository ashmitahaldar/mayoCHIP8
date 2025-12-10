A CHIP-8 emulator written in C++.

Referenced from the following links:
- https://austinmorlan.com/posts/chip8_emulator/
- https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

## Building

### Prerequisites
- C++17 compatible compiler (clang, g++, MSVC)
- CMake 3.16 or later
- SDL2

### macOS
Install SDL2 via Homebrew:
```bash
brew install sdl2
```

### Linux
```bash
sudo apt-get install libsdl2-dev
```

### Windows
Download SDL2 development libraries from [libsdl.org](https://www.libsdl.org/download-2.0.php)

### Build Steps
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

If CMake can't find SDL2, specify the path:
```bash
cmake -S . -B build -DSDL2_DIR=/opt/homebrew/lib/cmake/SDL2 -DCMAKE_BUILD_TYPE=Release
```

## Running

The emulator requires three command-line arguments:

```bash
./build/mayochip8 <scale> <delay> <rom_path>
```

- `scale`: Window scale factor (e.g., 10 for 10x zoom)
- `delay`: Cycle delay in milliseconds (e.g., 2)
- `rom_path`: Path to a CHIP-8 ROM file

### Example
```bash
./build/mayochip8 10 2 roms/test_opcode.ch8
```

### Controls
- `X` → 0
- `1-3` → 1-3
- `Q-E` → 4-6
- `A-D` → 7-9, A-C
- `Z-C` → D-E, F
- `ESC` → Quit