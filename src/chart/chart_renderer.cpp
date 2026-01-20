#include "chart_renderer.h"
#include <stdio.h>

ChartRenderer::ChartRenderer()
    : m_gridLines(5)
{
}

void ChartRenderer::render(const char* symbol,
                           float currentPrice,
                           const CandleBuffer& candleBuffer,
                           const Candle& currentCandle,
                           float ticksPerSecond,
                           float candleInterval,
                           float windowHeight)
{
    ImGuiIO& io = ImGui::GetIO();
    
    // Setup window
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, windowHeight), ImGuiCond_Always);
    
    char windowTitle[64];
    snprintf(windowTitle, sizeof(windowTitle), "Market - %s", symbol);
    
    ImGui::Begin(windowTitle, nullptr, 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
    renderHeader(symbol, currentPrice, candleBuffer, ticksPerSecond, candleInterval);
    
    ImGui::Separator();
    
    // Chart canvas
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 100.0f) canvasSize.x = 100.0f;
    if (canvasSize.y < 100.0f) canvasSize.y = 100.0f;
    
    // Reserve the canvas area
    ImGui::InvisibleButton("chart_canvas", canvasSize);
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    
    // Draw background
    drawList->AddRectFilled(canvasPos, 
                            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), 
                            m_colors.background);
    
    // Draw grid
    for (int i = 1; i < m_gridLines; i++)
    {
        float y = canvasPos.y + canvasSize.y * i / m_gridLines;
        drawList->AddLine(ImVec2(canvasPos.x, y), 
                         ImVec2(canvasPos.x + canvasSize.x, y), 
                         m_colors.grid);
    }
    
    // Render candles and price elements
    renderCandles(drawList, canvasPos, canvasSize, candleBuffer, currentCandle, currentPrice);
    
    ImGui::End();
}

void ChartRenderer::renderHeader(const char* symbol, float currentPrice,
                                  const CandleBuffer& candleBuffer,
                                  float ticksPerSecond, float candleInterval)
{
    // Calculate price change
    float priceChange = 0.0f;
    float priceChangePct = 0.0f;
    if (candleBuffer.count() > 0)
    {
        float openPrice = candleBuffer.get(0).open;
        priceChange = currentPrice - openPrice;
        priceChangePct = (priceChange / openPrice) * 100.0f;
    }
    
    ImGui::Text("%s", symbol);
    ImGui::SameLine(120);
    ImGui::Text("$%.2f", currentPrice);
    ImGui::SameLine(220);
    
    if (priceChange >= 0)
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "+%.2f (+%.2f%%)", priceChange, priceChangePct);
    else
        ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "%.2f (%.2f%%)", priceChange, priceChangePct);
    
    ImGui::SameLine(420);
    ImGui::Text("Candles: %d", candleBuffer.count());
    ImGui::SameLine(540);
    ImGui::Text("Interval: %.0fs", candleInterval);
    ImGui::SameLine(660);
    ImGui::Text("Ticks/s: %.0f", ticksPerSecond);
}

void ChartRenderer::renderCandles(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                                   const CandleBuffer& candleBuffer, const Candle& currentCandle,
                                   float currentPrice)
{
    // Calculate price range
    float minPrice = currentPrice;
    float maxPrice = currentPrice;
    
    // Include current candle
    if (currentCandle.valid)
    {
        if (currentCandle.low < minPrice) minPrice = currentCandle.low;
        if (currentCandle.high > maxPrice) maxPrice = currentCandle.high;
    }
    
    // Include completed candles
    for (int i = 0; i < candleBuffer.count(); i++)
    {
        const Candle& c = candleBuffer.get(i);
        if (c.low < minPrice) minPrice = c.low;
        if (c.high > maxPrice) maxPrice = c.high;
    }
    
    // Add padding
    float priceRange = maxPrice - minPrice;
    if (priceRange < 1.0f) priceRange = 1.0f;
    float pricePadding = priceRange * 0.1f;
    minPrice -= pricePadding;
    maxPrice += pricePadding;
    priceRange = maxPrice - minPrice;
    
    // Calculate candle dimensions
    int visibleCandles = candleBuffer.count() + 1;
    float candleWidth = (canvasSize.x - 20.0f) / (float)(visibleCandles > 0 ? visibleCandles : 1);
    if (candleWidth > 20.0f) candleWidth = 20.0f;
    if (candleWidth < 3.0f) candleWidth = 3.0f;
    float bodyWidth = candleWidth * 0.7f;
    
    // Price to Y coordinate helper
    auto priceToY = [&](float price) -> float {
        return canvasPos.y + canvasSize.y - ((price - minPrice) / priceRange) * canvasSize.y;
    };
    
    float xOffset = canvasPos.x + 10.0f;
    
    // Draw completed candles
    for (int i = 0; i < candleBuffer.count(); i++)
    {
        const Candle& c = candleBuffer.get(i);
        float x = xOffset + i * candleWidth + candleWidth * 0.5f;
        
        float yHigh = priceToY(c.high);
        float yLow = priceToY(c.low);
        float yOpen = priceToY(c.open);
        float yClose = priceToY(c.close);
        
        bool bullish = c.isBullish();
        ImU32 color = bullish ? m_colors.bullish : m_colors.bearish;
        
        // Wick
        drawList->AddLine(ImVec2(x, yHigh), ImVec2(x, yLow), color, 1.0f);
        
        // Body
        float bodyTop = bullish ? yClose : yOpen;
        float bodyBottom = bullish ? yOpen : yClose;
        if (bodyBottom - bodyTop < 1.0f) bodyBottom = bodyTop + 1.0f;
        
        drawList->AddRectFilled(
            ImVec2(x - bodyWidth * 0.5f, bodyTop),
            ImVec2(x + bodyWidth * 0.5f, bodyBottom),
            color
        );
    }
    
    // Draw current forming candle
    if (currentCandle.valid)
    {
        float x = xOffset + candleBuffer.count() * candleWidth + candleWidth * 0.5f;
        
        float yHigh = priceToY(currentCandle.high);
        float yLow = priceToY(currentCandle.low);
        float yOpen = priceToY(currentCandle.open);
        float yClose = priceToY(currentCandle.close);
        
        bool bullish = currentCandle.isBullish();
        ImU32 color = bullish ? m_colors.bullishLive : m_colors.bearishLive;
        
        drawList->AddLine(ImVec2(x, yHigh), ImVec2(x, yLow), color, 1.0f);
        
        float bodyTop = bullish ? yClose : yOpen;
        float bodyBottom = bullish ? yOpen : yClose;
        if (bodyBottom - bodyTop < 1.0f) bodyBottom = bodyTop + 1.0f;
        
        drawList->AddRectFilled(
            ImVec2(x - bodyWidth * 0.5f, bodyTop),
            ImVec2(x + bodyWidth * 0.5f, bodyBottom),
            color
        );
    }
    
    // Render price scale and current price line
    renderPriceScale(drawList, canvasPos, canvasSize, minPrice, maxPrice);
    renderCurrentPriceLine(drawList, canvasPos, canvasSize, currentPrice, minPrice, priceRange);
}

void ChartRenderer::renderPriceScale(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                                      float minPrice, float maxPrice)
{
    float priceRange = maxPrice - minPrice;
    char priceLabel[32];
    
    for (int i = 0; i <= m_gridLines; i++)
    {
        float price = maxPrice - (priceRange * i / m_gridLines);
        float y = canvasPos.y + canvasSize.y * i / m_gridLines;
        snprintf(priceLabel, sizeof(priceLabel), "%.2f", price);
        drawList->AddText(ImVec2(canvasPos.x + canvasSize.x - 50, y - 6), m_colors.text, priceLabel);
    }
}

void ChartRenderer::renderCurrentPriceLine(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                                            float currentPrice, float minPrice, float priceRange)
{
    float currentPriceY = canvasPos.y + canvasSize.y - ((currentPrice - minPrice) / priceRange) * canvasSize.y;
    
    // Price line
    drawList->AddLine(
        ImVec2(canvasPos.x, currentPriceY),
        ImVec2(canvasPos.x + canvasSize.x - 55, currentPriceY),
        m_colors.priceLine, 1.0f
    );
    
    // Price label background
    drawList->AddRectFilled(
        ImVec2(canvasPos.x + canvasSize.x - 55, currentPriceY - 8),
        ImVec2(canvasPos.x + canvasSize.x, currentPriceY + 8),
        m_colors.priceLabel
    );
    
    // Price label text
    char priceLabel[32];
    snprintf(priceLabel, sizeof(priceLabel), "%.2f", currentPrice);
    drawList->AddText(
        ImVec2(canvasPos.x + canvasSize.x - 50, currentPriceY - 6), 
        m_colors.priceLabelText, 
        priceLabel
    );
}
