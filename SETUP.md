# Setup Guide

## Prerequisites

### 1. Emscripten

```bash
# macOS
brew install emscripten

# Or manual install
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh

# Verify
emcc --version
```

### 2. Bun

```bash
# macOS
brew install bun

# Or curl
curl -fsSL https://bun.sh/install | bash

# Verify
bun --version
```

## Build & Run

```bash
make          # Build WASM
bun serve.js  # Start server at http://localhost:3000
```

## Project Structure

```
src/
├── main.cpp                 # App entry, main loop
├── chart/
│   ├── candle.h             # Candle data structures
│   ├── chart_renderer.h/cpp # Chart rendering, zoom, crosshair
├── data/
│   └── mock_ticker.h/cpp    # Price simulation, tick history
└── perf/
    └── perf_monitor.h/cpp   # Performance stats

libs/imgui/                  # Dear ImGui library
web/                         # Build output (generated)
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `emcc: command not found` | Run `source ./emsdk_env.sh` |
| White screen | Check browser console (F12) |
| Port 3000 in use | Kill existing process or change port in serve.js |

## Clean Rebuild

```bash
make clean && make
```
