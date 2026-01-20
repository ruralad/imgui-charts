CC = emcc
CXX = em++
WEB_DIR = web
EXE = $(WEB_DIR)/index.html

IMGUI_DIR = libs/imgui
SRC_DIR = src

# Application sources
SOURCES = $(SRC_DIR)/main.cpp
SOURCES += $(SRC_DIR)/data/mock_ticker.cpp
SOURCES += $(SRC_DIR)/chart/chart_renderer.cpp
SOURCES += $(SRC_DIR)/perf/perf_monitor.cpp

# ImGui sources
SOURCES += $(IMGUI_DIR)/imgui.cpp
SOURCES += $(IMGUI_DIR)/imgui_demo.cpp
SOURCES += $(IMGUI_DIR)/imgui_draw.cpp
SOURCES += $(IMGUI_DIR)/imgui_tables.cpp
SOURCES += $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

# Include paths
CFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(SRC_DIR)
CFLAGS += -Wall -Wformat -Os

# Emscripten flags
LDFLAGS = -s USE_SDL=2
LDFLAGS += -s WASM=1
LDFLAGS += -s ALLOW_MEMORY_GROWTH=1
LDFLAGS += -s NO_EXIT_RUNTIME=1
LDFLAGS += -s ASSERTIONS=1
LDFLAGS += -s USE_WEBGL2=1
LDFLAGS += -s FULL_ES3=1
LDFLAGS += -DIMGUI_IMPL_OPENGL_ES3

# Use custom shell template
LDFLAGS += --shell-file shell.html

all: $(EXE)

$(WEB_DIR):
	mkdir -p $(WEB_DIR)

$(EXE): $(WEB_DIR) $(SOURCES) shell.html
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $(EXE)

serve: $(EXE)
	bun serve.js

clean:
	rm -rf $(WEB_DIR)

.PHONY: all serve clean
