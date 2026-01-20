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
    
private:
    Colors m_colors;
    int m_gridLines;
    
    void renderHeader(const char* symbol, float currentPrice, 
                      const CandleBuffer& candleBuffer,
                      float ticksPerSecond, float candleInterval);
    
    void renderCandles(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                       const CandleBuffer& candleBuffer, const Candle& currentCandle,
                       float currentPrice);
    
    void renderPriceScale(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                          float minPrice, float maxPrice);
    
    void renderCurrentPriceLine(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                                 float currentPrice, float minPrice, float priceRange);
};
