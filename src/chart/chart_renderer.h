#pragma once

#include "imgui.h"
#include "candle.h"

// ============================================================================
// CHART RENDERER
// Renders candlestick charts using ImGui's ImDrawList
// ============================================================================

class ChartRenderer
{
public:
    // Interval options
    static constexpr int NUM_INTERVALS = 4;
    static constexpr float INTERVALS[NUM_INTERVALS] = {1.0f, 30.0f, 60.0f, 300.0f};
    static constexpr const char* INTERVAL_LABELS[NUM_INTERVALS] = {"1s", "30s", "1m", "5m"};
    
    // Zoom constants
    static constexpr float MIN_ZOOM = 0.5f;    // Show 2x more candles
    static constexpr float MAX_ZOOM = 10.0f;   // Show 10x fewer candles
    static constexpr float ZOOM_STEP = 0.15f;  // Zoom increment per scroll
    
    // Settings for toggleable features
    struct Settings
    {
        bool crosshairEnabled;
        bool tooltipEnabled;
        bool preserveHistory;   // true = re-aggregate on interval change, false = clear
        int selectedInterval;   // 0=1s, 1=30s, 2=1min, 3=5min
        
        Settings() 
            : crosshairEnabled(true)
            , tooltipEnabled(true)
            , preserveHistory(true)
            , selectedInterval(0) 
        {}
    };
    
    // Colors
    struct Colors
    {
        ImU32 background;
        ImU32 grid;
        ImU32 bullish;
        ImU32 bearish;
        ImU32 bullishLive;
        ImU32 bearishLive;
        ImU32 priceLine;
        ImU32 priceLabel;
        ImU32 priceLabelText;
        ImU32 text;
        ImU32 crosshair;  // Low opacity dotted line color
        
        Colors()
            : background(IM_COL32(20, 20, 25, 255))
            , grid(IM_COL32(40, 40, 50, 255))
            , bullish(IM_COL32(46, 204, 113, 255))
            , bearish(IM_COL32(231, 76, 60, 255))
            , bullishLive(IM_COL32(100, 220, 150, 255))
            , bearishLive(IM_COL32(240, 120, 100, 255))
            , priceLine(IM_COL32(255, 200, 100, 200))
            , priceLabel(IM_COL32(255, 200, 100, 255))
            , priceLabelText(IM_COL32(0, 0, 0, 255))
            , text(IM_COL32(180, 180, 180, 255))
            , crosshair(IM_COL32(200, 200, 200, 100))
        {}
    };
    
    ChartRenderer();
    
    // Render the chart window
    void render(const char* symbol,
                float currentPrice,
                const CandleBuffer& candleBuffer,
                const Candle& currentCandle,
                float ticksPerSecond,
                float candleInterval,
                float windowHeight);
    
    // Customization
    void setColors(const Colors& colors) { m_colors = colors; }
    void setGridLines(int lines) { m_gridLines = lines; }
    
    // Feature toggles
    void setCrosshairEnabled(bool enabled) { m_settings.crosshairEnabled = enabled; }
    void setTooltipEnabled(bool enabled) { m_settings.tooltipEnabled = enabled; }
    bool isCrosshairEnabled() const { return m_settings.crosshairEnabled; }
    bool isTooltipEnabled() const { return m_settings.tooltipEnabled; }
    Settings& getSettings() { return m_settings; }
    
    // Zoom controls
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void adjustZoom(float delta);
    void setScrollOffset(float offset);
    float getZoomLevel() const { return m_zoomLevel; }
    float getScrollOffset() const { return m_scrollOffset; }
    
private:
    Colors m_colors;
    Settings m_settings;
    int m_gridLines;
    
    // Zoom state
    float m_zoomLevel;      // 1.0 = default, 2.0 = 2x zoom (fewer candles), etc.
    float m_scrollOffset;   // Horizontal scroll position (0.0 = start, 1.0 = end)
    
    // Hover state tracking
    int m_hoveredCandleIndex;
    Candle m_hoveredCandle;
    ImVec2 m_lastCanvasPos;
    ImVec2 m_lastCanvasSize;
    float m_lastMinPrice;
    float m_lastPriceRange;
    
    void renderHeader(const char* symbol, float currentPrice, 
                      const CandleBuffer& candleBuffer,
                      float ticksPerSecond, float candleInterval);
    
    void renderCandles(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                       const CandleBuffer& candleBuffer, const Candle& currentCandle,
                       float currentPrice, bool isHovered, ImVec2 mousePos);
    
    void renderPriceScale(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                          float minPrice, float maxPrice);
    
    void renderCurrentPriceLine(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                                 float currentPrice, float minPrice, float priceRange);
    
    void renderCrosshair(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                         ImVec2 mousePos, float minPrice, float priceRange);
    
    void renderTooltip(const Candle& candle, int candleIndex);
    
    // Helper for drawing dotted lines
    void drawDottedLine(ImDrawList* drawList, ImVec2 p1, ImVec2 p2, ImU32 color, 
                        float segmentLength = 5.0f, float gapLength = 3.0f);
};
