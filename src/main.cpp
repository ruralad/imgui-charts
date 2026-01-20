// ============================================================================
// MARKET CHART APPLICATION
// A real-time candlestick chart built with ImGui + WebAssembly
// Demonstrates superior performance vs React/JS
// ============================================================================

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#include <emscripten.h>
#include <SDL_opengles2.h>

// Application modules
#include "data/mock_ticker.h"
#include "chart/chart_renderer.h"
#include "perf/perf_monitor.h"

// ============================================================================
// APPLICATION STATE
// ============================================================================

static SDL_Window* g_Window = nullptr;
static SDL_GLContext g_GLContext = nullptr;
static ImVec4 g_ClearColor = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

// Application modules
static MockTicker g_Ticker;
static ChartRenderer g_ChartRenderer;
static PerfMonitor g_PerfMonitor;

// ============================================================================
// MAIN LOOP
// ============================================================================

// Track interval selection changes
static int g_LastIntervalSelection = 0;

void main_loop()
{
    ImGuiIO& io = ImGui::GetIO();
    
    // Check if interval selection changed
    int currentInterval = g_ChartRenderer.getSettings().selectedInterval;
    if (currentInterval != g_LastIntervalSelection)
    {
        bool preserveHistory = g_ChartRenderer.getSettings().preserveHistory;
        g_Ticker.setCandleInterval(ChartRenderer::INTERVALS[currentInterval], preserveHistory);
        g_LastIntervalSelection = currentInterval;
    }
    
    // Update performance monitoring
    g_PerfMonitor.beginFrame(io.DeltaTime);
    
    // Update price data
    g_Ticker.update(io.DeltaTime);
    
    // Poll SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
    }
    
    // Start new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    // Calculate layout: chart takes top 4/5, performance takes bottom 1/5
    float chartHeight = io.DisplaySize.y * 0.8f;
    float perfHeight = io.DisplaySize.y * 0.2f;
    
    // Render chart
    g_ChartRenderer.render(
        "MOCK/USD",
        g_Ticker.getCurrentPrice(),
        g_Ticker.getCandleBuffer(),
        g_Ticker.getCurrentCandle(),
        g_Ticker.getTicksPerSecond(),
        g_Ticker.getCandleInterval(),
        chartHeight
    );
    
    // Render performance panel
    g_PerfMonitor.renderWindow(
        chartHeight,
        perfHeight,
        io.DisplaySize.x,
        g_Ticker.getTicksPerSecond(),
        g_Ticker.getCandleBuffer().count(),
        g_Ticker.getCandleBuffer().maxCandles()
    );
    
    // Finalize ImGui frame
    ImGui::Render();
    
    // Update performance stats with draw data
    g_PerfMonitor.endFrame(ImGui::GetDrawData());
    
    // OpenGL render
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(
        g_ClearColor.x * g_ClearColor.w, 
        g_ClearColor.y * g_ClearColor.w, 
        g_ClearColor.z * g_ClearColor.w, 
        g_ClearColor.w
    );
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(g_Window);
}

// ============================================================================
// INITIALIZATION
// ============================================================================

bool initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("Error: SDL_Init: %s\n", SDL_GetError());
        return false;
    }
    
    // Setup OpenGL ES 3.0 for WebGL 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    
    g_Window = SDL_CreateWindow(
        "Market Chart", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        1280, 720, 
        window_flags
    );
    
    if (g_Window == nullptr)
    {
        printf("Error: SDL_CreateWindow: %s\n", SDL_GetError());
        return false;
    }
    
    g_GLContext = SDL_GL_CreateContext(g_Window);
    if (g_GLContext == nullptr)
    {
        printf("Error: SDL_GL_CreateContext: %s\n", SDL_GetError());
        return false;
    }
    
    SDL_GL_MakeCurrent(g_Window, g_GLContext);
    SDL_GL_SetSwapInterval(1);  // Enable vsync
    
    return true;
}

bool initImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    
    const char* glsl_version = "#version 300 es";
    ImGui_ImplSDL2_InitForOpenGL(g_Window, g_GLContext);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    return true;
}

void shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_GL_DeleteContext(g_GLContext);
    SDL_DestroyWindow(g_Window);
    SDL_Quit();
}

// ============================================================================
// ENTRY POINT
// ============================================================================

int main(int, char**)
{
    if (!initSDL())
        return 1;
    
    if (!initImGui())
        return 1;
    
    // Start main loop (Emscripten will handle the loop)
    // 0 = use requestAnimationFrame, 1 = simulate infinite loop
    emscripten_set_main_loop(main_loop, 0, 1);
    
    // Cleanup (won't be reached in Emscripten, but good practice)
    shutdown();
    
    return 0;
}
