#include "chart_renderer.h"
#include <stdio.h>
#include <math.h>

ChartRenderer::ChartRenderer()
    : m_gridLines(5)
    , m_zoomLevel(1.0f)
    , m_scrollOffset(1.0f)  // Start at the end (most recent candles)
    , m_hoveredCandleIndex(-1)
    , m_lastCanvasPos(0, 0)
    , m_lastCanvasSize(0, 0)
    , m_lastMinPrice(0)
    , m_lastPriceRange(1.0f)
{
}

// ============================================================================
// ZOOM CONTROLS
// ============================================================================

void ChartRenderer::zoomIn()
{
    m_zoomLevel *= (1.0f + ZOOM_STEP);
    if (m_zoomLevel > MAX_ZOOM) m_zoomLevel = MAX_ZOOM;
}

void ChartRenderer::zoomOut()
{
    m_zoomLevel *= (1.0f - ZOOM_STEP);
    if (m_zoomLevel < MIN_ZOOM) m_zoomLevel = MIN_ZOOM;
}

void ChartRenderer::resetZoom()
{
    m_zoomLevel = 1.0f;
    m_scrollOffset = 1.0f;  // Back to end
}

void ChartRenderer::adjustZoom(float delta)
{
    m_zoomLevel *= (1.0f + delta);
    if (m_zoomLevel < MIN_ZOOM) m_zoomLevel = MIN_ZOOM;
    if (m_zoomLevel > MAX_ZOOM) m_zoomLevel = MAX_ZOOM;
}

void ChartRenderer::setScrollOffset(float offset)
{
    m_scrollOffset = offset;
    if (m_scrollOffset < 0.0f) m_scrollOffset = 0.0f;
    if (m_scrollOffset > 1.0f) m_scrollOffset = 1.0f;
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
    
    // Reset hover state at start of frame
    m_hoveredCandleIndex = -1;
    
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
    bool isHovered = ImGui::IsItemHovered();
    ImVec2 mousePos = ImGui::GetMousePos();
    
    // Handle zoom with mouse scroll wheel (when hovering over chart)
    if (isHovered)
    {
        float wheel = io.MouseWheel;
        if (wheel != 0.0f)
        {
            if (io.KeyShift)
            {
                // Shift + scroll = horizontal pan
                m_scrollOffset -= wheel * 0.05f;
                if (m_scrollOffset < 0.0f) m_scrollOffset = 0.0f;
                if (m_scrollOffset > 1.0f) m_scrollOffset = 1.0f;
            }
            else
            {
                // Regular scroll = zoom (also handles touchpad pinch)
                adjustZoom(wheel * ZOOM_STEP);
            }
        }
    }
    
    // Handle drag to pan when zoomed in
    if (isHovered && m_zoomLevel > 1.0f && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        if (delta.x != 0.0f)
        {
            // Convert pixel drag to scroll offset
            float dragSensitivity = 0.002f / m_zoomLevel;
            m_scrollOffset -= delta.x * dragSensitivity;
            if (m_scrollOffset < 0.0f) m_scrollOffset = 0.0f;
            if (m_scrollOffset > 1.0f) m_scrollOffset = 1.0f;
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }
    }
    
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
    
    // Render candles and price elements (also handles hit detection)
    renderCandles(drawList, canvasPos, canvasSize, candleBuffer, currentCandle, currentPrice, isHovered, mousePos);
    
    // Render crosshair if enabled and hovering
    if (m_settings.crosshairEnabled && isHovered)
    {
        renderCrosshair(drawList, canvasPos, canvasSize, mousePos, m_lastMinPrice, m_lastPriceRange);
    }
    
    // Render tooltip if enabled and hovering a candle
    if (m_settings.tooltipEnabled && m_hoveredCandleIndex >= 0)
    {
        renderTooltip(m_hoveredCandle, m_hoveredCandleIndex);
    }
    
    ImGui::End();
}

void ChartRenderer::renderHeader(const char* symbol, float currentPrice,
                                  const CandleBuffer& candleBuffer,
                                  float ticksPerSecond, float candleInterval)
{
    ImGuiIO& io = ImGui::GetIO();
    
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
    ImGui::SameLine(520);
    ImGui::Text("Ticks/s: %.0f", ticksPerSecond);
    
    // Second row: interval selector and toggles
    ImGui::Text("Interval:");
    ImGui::SameLine();
    for (int i = 0; i < NUM_INTERVALS; i++)
    {
        if (i > 0) ImGui::SameLine();
        ImGui::RadioButton(INTERVAL_LABELS[i], &m_settings.selectedInterval, i);
    }
    
    // Preserve history checkbox
    ImGui::SameLine();
    ImGui::Checkbox("Preserve", &m_settings.preserveHistory);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("When enabled, changing interval re-aggregates");
        ImGui::Text("existing data. When disabled, clears candles.");
        ImGui::EndTooltip();
    }
    
    // Zoom controls
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    if (ImGui::SmallButton("-##zoom")) zoomOut();
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Zoom out (show more candles)");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    ImGui::Text("%.0f%%", m_zoomLevel * 100.0f);
    ImGui::SameLine();
    if (ImGui::SmallButton("+##zoom")) zoomIn();
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Zoom in (show fewer candles)");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Reset##zoom")) resetZoom();
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Reset zoom to 100%%");
        ImGui::EndTooltip();
    }
    
    // Toggle checkboxes for crosshair and tooltip (same row)
    ImGui::SameLine(io.DisplaySize.x - 220);
    ImGui::Checkbox("Crosshair", &m_settings.crosshairEnabled);
    ImGui::SameLine();
    ImGui::Checkbox("Tooltip", &m_settings.tooltipEnabled);
}

void ChartRenderer::renderCandles(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                                   const CandleBuffer& candleBuffer, const Candle& currentCandle,
                                   float currentPrice, bool isHovered, ImVec2 mousePos)
{
    // Total candles including current forming candle
    int totalCandles = candleBuffer.count() + (currentCandle.valid ? 1 : 0);
    if (totalCandles == 0)
    {
        // No candles to render
        m_lastCanvasPos = canvasPos;
        m_lastCanvasSize = canvasSize;
        m_lastMinPrice = currentPrice - 1.0f;
        m_lastPriceRange = 2.0f;
        return;
    }
    
    // Calculate visible candle range based on zoom
    int visibleCount = (int)(totalCandles / m_zoomLevel);
    if (visibleCount < 1) visibleCount = 1;
    if (visibleCount > totalCandles) visibleCount = totalCandles;
    
    // Calculate start index based on scroll offset
    int maxStartIndex = totalCandles - visibleCount;
    if (maxStartIndex < 0) maxStartIndex = 0;
    int startIndex = (int)(m_scrollOffset * maxStartIndex);
    int endIndex = startIndex + visibleCount;
    if (endIndex > totalCandles) endIndex = totalCandles;
    
    // Calculate price range only for visible candles
    float minPrice = currentPrice;
    float maxPrice = currentPrice;
    
    for (int i = startIndex; i < endIndex; i++)
    {
        if (i < candleBuffer.count())
        {
            const Candle& c = candleBuffer.get(i);
            if (c.low < minPrice) minPrice = c.low;
            if (c.high > maxPrice) maxPrice = c.high;
        }
        else if (currentCandle.valid)
        {
            // Current forming candle
            if (currentCandle.low < minPrice) minPrice = currentCandle.low;
            if (currentCandle.high > maxPrice) maxPrice = currentCandle.high;
        }
    }
    
    // Add padding
    float priceRange = maxPrice - minPrice;
    if (priceRange < 1.0f) priceRange = 1.0f;
    float pricePadding = priceRange * 0.1f;
    minPrice -= pricePadding;
    maxPrice += pricePadding;
    priceRange = maxPrice - minPrice;
    
    // Store for crosshair use
    m_lastCanvasPos = canvasPos;
    m_lastCanvasSize = canvasSize;
    m_lastMinPrice = minPrice;
    m_lastPriceRange = priceRange;
    
    // Calculate candle dimensions based on visible count
    float candleWidth = (canvasSize.x - 70.0f) / (float)visibleCount;
    if (candleWidth > 40.0f) candleWidth = 40.0f;
    if (candleWidth < 3.0f) candleWidth = 3.0f;
    float bodyWidth = candleWidth * 0.7f;
    
    // Price to Y coordinate helper
    auto priceToY = [&](float price) -> float {
        return canvasPos.y + canvasSize.y - ((price - minPrice) / priceRange) * canvasSize.y;
    };
    
    float xOffset = canvasPos.x + 10.0f;
    
    // Draw visible candles
    for (int i = startIndex; i < endIndex; i++)
    {
        int displayIndex = i - startIndex;  // Position in visible area
        float x = xOffset + displayIndex * candleWidth + candleWidth * 0.5f;
        
        bool isCurrentCandle = (i >= candleBuffer.count());
        const Candle& c = isCurrentCandle ? currentCandle : candleBuffer.get(i);
        
        if (!c.valid) continue;
        
        float yHigh = priceToY(c.high);
        float yLow = priceToY(c.low);
        float yOpen = priceToY(c.open);
        float yClose = priceToY(c.close);
        
        bool bullish = c.isBullish();
        ImU32 color;
        if (isCurrentCandle)
            color = bullish ? m_colors.bullishLive : m_colors.bearishLive;
        else
            color = bullish ? m_colors.bullish : m_colors.bearish;
        
        // Hit detection for this candle
        if (isHovered)
        {
            float candleLeft = x - candleWidth * 0.5f;
            float candleRight = x + candleWidth * 0.5f;
            if (mousePos.x >= candleLeft && mousePos.x <= candleRight)
            {
                m_hoveredCandleIndex = i;
                m_hoveredCandle = c;
            }
        }
        
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

void ChartRenderer::drawDottedLine(ImDrawList* drawList, ImVec2 p1, ImVec2 p2, ImU32 color,
                                    float segmentLength, float gapLength)
{
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float length = sqrtf(dx * dx + dy * dy);
    
    if (length < 0.001f) return;
    
    // Normalize direction
    float nx = dx / length;
    float ny = dy / length;
    
    float totalStep = segmentLength + gapLength;
    float currentPos = 0.0f;
    
    while (currentPos < length)
    {
        float segStart = currentPos;
        float segEnd = currentPos + segmentLength;
        if (segEnd > length) segEnd = length;
        
        ImVec2 start(p1.x + nx * segStart, p1.y + ny * segStart);
        ImVec2 end(p1.x + nx * segEnd, p1.y + ny * segEnd);
        
        drawList->AddLine(start, end, color, 1.0f);
        
        currentPos += totalStep;
    }
}

void ChartRenderer::renderCrosshair(ImDrawList* drawList, ImVec2 canvasPos, ImVec2 canvasSize,
                                     ImVec2 mousePos, float minPrice, float priceRange)
{
    // Clamp mouse position to canvas bounds
    float clampedX = mousePos.x;
    float clampedY = mousePos.y;
    
    if (clampedX < canvasPos.x) clampedX = canvasPos.x;
    if (clampedX > canvasPos.x + canvasSize.x) clampedX = canvasPos.x + canvasSize.x;
    if (clampedY < canvasPos.y) clampedY = canvasPos.y;
    if (clampedY > canvasPos.y + canvasSize.y) clampedY = canvasPos.y + canvasSize.y;
    
    // Draw vertical dotted line
    drawDottedLine(drawList, 
                   ImVec2(clampedX, canvasPos.y), 
                   ImVec2(clampedX, canvasPos.y + canvasSize.y), 
                   m_colors.crosshair, 4.0f, 4.0f);
    
    // Draw horizontal dotted line
    drawDottedLine(drawList,
                   ImVec2(canvasPos.x, clampedY),
                   ImVec2(canvasPos.x + canvasSize.x - 55, clampedY),
                   m_colors.crosshair, 4.0f, 4.0f);
    
    // Calculate and display price at cursor Y position
    float normalizedY = (canvasPos.y + canvasSize.y - clampedY) / canvasSize.y;
    float priceAtCursor = minPrice + normalizedY * priceRange;
    
    // Price label at crosshair intersection with right edge
    char priceLabel[32];
    snprintf(priceLabel, sizeof(priceLabel), "%.2f", priceAtCursor);
    
    // Small label background
    ImVec2 labelPos(canvasPos.x + canvasSize.x - 55, clampedY - 8);
    drawList->AddRectFilled(
        labelPos,
        ImVec2(labelPos.x + 55, labelPos.y + 16),
        IM_COL32(60, 60, 70, 220)
    );
    drawList->AddText(
        ImVec2(labelPos.x + 5, labelPos.y + 2),
        IM_COL32(220, 220, 220, 255),
        priceLabel
    );
}

void ChartRenderer::renderTooltip(const Candle& candle, int candleIndex)
{
    ImGui::BeginTooltip();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));
    
    // Title with candle index
    if (candleIndex >= 0)
    {
        ImGui::Text("Candle #%d", candleIndex + 1);
        ImGui::Separator();
    }
    
    // OHLC data with colors
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Open:");
    ImGui::SameLine(60);
    ImGui::Text("$%.2f", candle.open);
    
    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "High:");
    ImGui::SameLine(60);
    ImGui::Text("$%.2f", candle.high);
    
    ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Low:");
    ImGui::SameLine(60);
    ImGui::Text("$%.2f", candle.low);
    
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Close:");
    ImGui::SameLine(60);
    ImGui::Text("$%.2f", candle.close);
    
    // Show change
    float change = candle.close - candle.open;
    float changePct = (candle.open > 0) ? (change / candle.open * 100.0f) : 0.0f;
    
    ImGui::Separator();
    if (change >= 0)
        ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "Change: +%.2f (+%.2f%%)", change, changePct);
    else
        ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Change: %.2f (%.2f%%)", change, changePct);
    
    ImGui::PopStyleVar();
    ImGui::EndTooltip();
}
