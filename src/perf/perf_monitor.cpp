#include "perf_monitor.h"
#include <math.h>
#include <emscripten/heap.h>

PerfMonitor::PerfMonitor()
    : m_frameTimeSum(0.0f)
    , m_frameTimeHistoryIdx(0)
    , m_initialized(false)
{
    m_stats = {};
    m_stats.frameTimeMin = 1000.0f;
    m_stats.frameTimeMax = 0.0f;
    
    for (int i = 0; i < HISTORY_SIZE; i++)
    {
        m_frameTimeHistory[i] = 16.667f;
    }
}

void PerfMonitor::beginFrame(float deltaTime)
{
    m_initialized = true;
    
    float frameTimeMs = deltaTime * 1000.0f;
    
    // Sanity check
    if (frameTimeMs <= 0.0f || frameTimeMs >= 1000.0f)
        return;
    
    // Update min/max
    if (frameTimeMs < m_stats.frameTimeMin) m_stats.frameTimeMin = frameTimeMs;
    if (frameTimeMs > m_stats.frameTimeMax) m_stats.frameTimeMax = frameTimeMs;
    
    // Update sum and sample count
    m_frameTimeSum += frameTimeMs;
    m_stats.frameTimeSamples++;
    
    // Track spikes (>20ms would typically be GC pause in JS/React)
    if (frameTimeMs > 20.0f) m_stats.frameSpikes++;
    
    // Rolling history for jitter calculation
    m_frameTimeHistory[m_frameTimeHistoryIdx] = frameTimeMs;
    m_frameTimeHistoryIdx = (m_frameTimeHistoryIdx + 1) % HISTORY_SIZE;
    
    // Update derived stats
    m_stats.frameTimeAvg = m_stats.frameTimeSamples > 0 
        ? m_frameTimeSum / m_stats.frameTimeSamples 
        : 16.667f;
    
    m_stats.spikeRate = m_stats.frameTimeSamples > 0 
        ? (float)m_stats.frameSpikes / m_stats.frameTimeSamples * 100.0f 
        : 0.0f;
    
    updateJitter();
    
    // Update memory stats
    m_stats.heapSizeBytes = emscripten_get_heap_size();
    m_stats.heapSizeMB = (float)m_stats.heapSizeBytes / (1024.0f * 1024.0f);
}

void PerfMonitor::endFrame(ImDrawData* drawData)
{
    if (drawData)
    {
        m_stats.drawLists = drawData->CmdListsCount;
        m_stats.drawCalls = 0;
        for (int i = 0; i < drawData->CmdListsCount; i++)
        {
            m_stats.drawCalls += drawData->CmdLists[i]->CmdBuffer.Size;
        }
    }
    
    // Get render stats from ImGui
    ImGuiIO& io = ImGui::GetIO();
    m_stats.vertices = io.MetricsRenderVertices;
    m_stats.indices = io.MetricsRenderIndices;
    m_stats.triangles = m_stats.indices / 3;
}

void PerfMonitor::updateJitter()
{
    float jitterSum = 0.0f;
    for (int i = 0; i < HISTORY_SIZE; i++)
    {
        float diff = m_frameTimeHistory[i] - m_stats.frameTimeAvg;
        jitterSum += diff * diff;
    }
    m_stats.frameTimeJitter = sqrtf(jitterSum / HISTORY_SIZE);
}

void PerfMonitor::renderWindow(float windowPosY, float windowHeight, float windowWidth,
                                float ticksPerSecond, int candleCount, int maxCandles)
{
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::SetNextWindowPos(ImVec2(0, windowPosY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
    ImGui::Begin("Performance", nullptr, 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
    // Horizontal layout for performance stats
    ImGui::Columns(6, nullptr, false);
    
    // Column 1: FPS and frame timing
    ImGui::Text("FPS: %.1f", io.Framerate);
    float frameMs = (io.Framerate > 0.0f) ? (1000.0f / io.Framerate) : 0.0f;
    ImGui::Text("Frame: %.2f ms", frameMs);
    float budgetUsage = (frameMs / 16.667f) * 100.0f;
    ImGui::Text("Budget: %.1f%%", budgetUsage);
    
    ImGui::NextColumn();
    
    // Column 2: Frame consistency (KEY WASM ADVANTAGE - no GC pauses!)
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Frame Consistency");
    ImGui::Text("Min: %.2f ms", m_stats.frameTimeMin);
    ImGui::Text("Max: %.2f ms", m_stats.frameTimeMax);
    
    ImGui::NextColumn();
    
    // Column 3: Jitter and spikes (LOW = good, shows no GC pauses)
    ImGui::Text("Jitter: %.3f ms", m_stats.frameTimeJitter);
    if (m_stats.spikeRate < 1.0f)
        ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "Spikes: %.2f%%", m_stats.spikeRate);
    else
        ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Spikes: %.2f%%", m_stats.spikeRate);
    ImGui::Text("Samples: %d", m_stats.frameTimeSamples);
    
    ImGui::NextColumn();
    
    // Column 4: Render efficiency
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Render Stats");
    ImGui::Text("Triangles: %d", m_stats.triangles);
    ImGui::Text("Draw Calls: %d", m_stats.drawCalls);
    
    ImGui::NextColumn();
    
    // Column 5: Memory (WASM has predictable memory, no GC churn)
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Memory");
    ImGui::Text("Heap: %.1f MB", m_stats.heapSizeMB);
    ImGui::Text("Verts: %d", m_stats.vertices);
    
    ImGui::NextColumn();
    
    // Column 6: Update stats
    ImGui::Text("Ticks/s: %.0f", ticksPerSecond);
    ImGui::Text("Candles: %d/%d", candleCount, maxCandles);
    ImGui::Text("Frames: %d", ImGui::GetFrameCount());
    
    ImGui::Columns(1);
    ImGui::End();
}
