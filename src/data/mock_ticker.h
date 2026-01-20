#pragma once

#include "../chart/candle.h"

// ============================================================================
// MOCK TICKER - Simulated price data generator
// Generates random walk price movements and aggregates into candles
// ============================================================================

class MockTicker
{
public:
    MockTicker();
    
    // Update the ticker with delta time (call every frame)
    void update(float deltaTime);
    
    // Getters
    float getCurrentPrice() const { return m_currentPrice; }
    float getTicksPerSecond() const { return m_ticksPerSecond; }
    float getCandleInterval() const { return CANDLE_INTERVAL; }
    const Candle& getCurrentCandle() const { return m_currentCandle; }
    const CandleBuffer& getCandleBuffer() const { return m_candleBuffer; }
    
    // Configuration
    void setVolatility(float v) { m_volatility = v; }
    void setCandleInterval(float interval) { /* TODO: implement */ }
    
private:
    static constexpr float CANDLE_INTERVAL = 1.0f;  // New candle every 1 second
    
    // Price state
    float m_currentPrice;
    float m_volatility;
    bool m_initialized;
    
    // Candle aggregation
    CandleBuffer m_candleBuffer;
    Candle m_currentCandle;
    float m_candleTimer;
    
    // Tick rate tracking
    int m_tickCount;
    float m_tickRateTimer;
    float m_ticksPerSecond;
    
    // Helpers
    float randomWalk();
    void finalizeCandle();
};
