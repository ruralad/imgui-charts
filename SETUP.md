# ImGui Web Counter - Setup Guide

## Prerequisites

### 1. Install Emscripten

Emscripten is required to compile C++ to WebAssembly.

#### On macOS (Homebrew)
```bash
brew install emscripten
```

#### On macOS (Manual Install)
```bash
# Clone Emscripten SDK
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Install and activate
./emsdk install latest
./emsdk activate latest

# Source the environment (add to your .zshrc or .bashrc)
source ./emsdk_env.sh
```

#### Verify Installation
```bash
emcc --version
```

### 2. Install Bun

Bun is used to serve static files locally.

#### On macOS (Homebrew)
```bash
brew tap oven-sh/bun
brew install bun
```

#### With Curl
```bash
curl -fsSL https://bun.sh/install | bash
```

#### Verify Installation
```bash
bun --version
```

### 3. SDL2 (for Emscripten)

SDL2 is automatically handled by Emscripten's `-s USE_SDL=2` flag. No manual installation required for web builds.

## Project Structure

```
imgui-web-counter/
├── main.cpp              # Counter app with ImGui
├── Makefile              # Build script for web
├── serve.js              # Bun static file server
├── shell.html            # Emscripten HTML shell
├── SETUP.md             # This file
└── libs/
    └── imgui/            # ImGui source files
        ├── imgui.h, imgui.cpp
        ├── imgui_demo.cpp
        ├── imgui_draw.cpp
        ├── imgui_tables.cpp
        ├── imgui_widgets.cpp
        └── backends/
            ├── imgui_impl_sdl2.cpp/.h
            └── imgui_impl_opengl3.cpp/.h
```

## Build and Run

### Build for Web
```bash
make
```

This creates a `web/` directory with:
- `index.html` - Main HTML file
- `index.js` - JavaScript glue code
- `index.wasm` - WebAssembly module

### Start Bun Server
```bash
bun serve.js
```

### Open in Browser
Navigate to: `http://localhost:3000`

## Development Tips

1. **Rebuild on changes**: Run `make` after editing C++ code
2. **Refresh browser**: After rebuild, refresh your browser (Cmd+R)
3. **Console debugging**: Open browser DevTools (F12) to see errors

## Troubleshooting

### "emcc: command not found"
- Ensure Emscripten is installed and sourced: `source ./emsdk_env.sh`
- Add to your shell config (.zshrc or .bashrc)

### Build fails with linker errors
- Ensure all ImGui source files are downloaded
- Check that `libs/imgui/` directory exists with all required files

### White screen in browser
- Open browser DevTools console for errors
- Check that WASM file is being served with correct MIME type (`application/wasm`)

### Server not starting
- Verify Bun is installed: `bun --version`
- Check port 3000 is not already in use

## Clean Build

```bash
make clean
make
```

This removes all build artifacts and rebuilds from scratch.
