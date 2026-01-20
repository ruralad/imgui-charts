# ImGui Web Counter

A simple counter application built with [Dear ImGui](https://github.com/ocornut/imgui), compiled to WebAssembly, and served locally with [Bun](https://bun.sh).

## Quick Start

```bash
# 1. Build for web
make

# 2. Start Bun server
bun serve.js

# 3. Open browser
# Navigate to http://localhost:3000
```

## What This Project Demonstrates

- ImGui immediate mode GUI paradigm
- C++ to WebAssembly compilation
- Cross-platform GUI rendering with SDL2 + OpenGL3
- Bun static file server

## Features

- Increment counter button
- Reset counter button
- Real-time display of counter value
- Responsive canvas (resizes with window)

## Project Files

- `main.cpp` - Counter application logic
- `Makefile` - Build configuration for Emscripten
- `serve.js` - Bun static file server
- `shell.html` - Emscripten HTML template
- `libs/imgui/` - Dear ImGui library source files

## Full Setup Guide

See [SETUP.md](SETUP.md) for:
- Installing Emscripten
- Installing Bun
- Troubleshooting tips
