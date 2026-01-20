# ImGui Market Chart

Real-time candlestick chart built with Dear ImGui + WebAssembly. Demonstrates high-performance UI rendering without JavaScript frameworks.

## Quick Start

```bash
make          # Build WASM
bun serve.js  # Start server
# Open http://localhost:3000
```

## Features

- **Live candlestick chart** with OHLC data
- **Multiple intervals**: 1s, 30s, 1m, 5m
- **Zoom & pan**: Scroll wheel, buttons, drag
- **Crosshair & tooltips**: Hover for price details
- **History preservation**: Re-aggregate candles on interval change

## Controls

| Action | Input |
|--------|-------|
| Zoom | Scroll wheel / +/- buttons |
| Pan | Shift+scroll / Drag |
| Tooltip | Hover candle |

## Project Structure

```
src/
├── main.cpp              # Entry point
├── chart/                # Chart rendering
├── data/                 # Price simulation
└── perf/                 # Performance monitoring
```

## Requirements

- Emscripten SDK
- Bun (or any static server)

See [SETUP.md](SETUP.md) for installation.
