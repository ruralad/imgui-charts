#pragma once

#include "imgui.h"

// ============================================================================
// PERFORMANCE MONITOR
// Tracks frame timing, memory usage, and render stats
// Demonstrates WASM/ImGui advantages over React/JS
// ============================================================================

struct PerfStats
{
    // Frame timing
    float frameTimeMin;
    float frameTimeMax;
    float frameTimeAvg;
    float frameTimeJitter;
    int frameTimeSamples;
    int frameSpikes;        // Frames > 20ms
    float spikeRate;        // Percentage of spiked frames
    
    // Memory
    float heapSizeMB;
    size_t heapSizeBytes;
    
    // Render
    int triangles;
    int drawCalls;
    int drawLists;
    int vertices;
    int indices;
};

class PerfMonitor
{
public:
    static const int HISTORY_SIZE = 60;
    
    PerfMonitor();
    
    // Call at start of each frame with delta time
    void beginFrame(float deltaTime);
    
    // Call after ImGui::Render() to capture draw stats
    void endFrame(ImDrawData* drawData);
    
    // Get current stats
    const PerfStats& getStats() const { return m_stats; }
    
    // Render the performance window
    void renderWindow(float windowPosY, float windowHeight, float windowWidth, 
                      float ticksPerSecond, int candleCount, int maxCandles);
    
private:
    PerfStats m_stats;
    
    // Frame time tracking
    float m_frameTimeSum;
    float m_frameTimeHistory[HISTORY_SIZE];
    int m_frameTimeHistoryIdx;
    bool m_initialized;
    
    void updateJitter();
};
