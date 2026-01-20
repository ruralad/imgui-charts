#pragma once

#include "../chart/candle.h"

// ============================================================================
// TICK DATA - Raw price data with timestamp
// ============================================================================

struct Tick
{
    float price;
    float timestamp;  // Elapsed time since start
    
    Tick() : price(0), timestamp(0) {}
    Tick(float p, float t) : price(p), timestamp(t) {}
};

// ============================================================================
// TICK HISTORY - Ring buffer for storing raw tick data
// ============================================================================

class TickHistory
{
public:
    static const int MAX_TICKS = 36000;  // Store up to 10 minutes at 60fps
    
    TickHistory() : m_count(0), m_start(0) {}
    
    void push(const Tick& tick)
    {
        if (m_count < MAX_TICKS)
        {
            m_ticks[m_count] = tick;
            m_count++;
        }
        else
        {
            m_ticks[m_start] = tick;
            m_start = (m_start + 1) % MAX_TICKS;
        }
    }
    
    const Tick& get(int index) const
    {
        int actualIndex = (m_start + index) % MAX_TICKS;
        return m_ticks[actualIndex];
    }
    
    int count() const { return m_count; }
    
    void clear()
    {
        m_count = 0;
        m_start = 0;
    }
    
    // Get earliest timestamp in history
    float getStartTime() const
    {
        if (m_count == 0) return 0.0f;
        return get(0).timestamp;
    }
    
    // Get latest timestamp in history
    float getEndTime() const
    {
        if (m_count == 0) return 0.0f;
        return get(m_count - 1).timestamp;
    }
    
private:
    Tick m_ticks[MAX_TICKS];
    int m_count;
    int m_start;
};

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
    float getCandleInterval() const { return m_candleInterval; }
    const Candle& getCurrentCandle() const { return m_currentCandle; }
    const CandleBuffer& getCandleBuffer() const { return m_candleBuffer; }
    float getElapsedTime() const { return m_elapsedTime; }
    
    // Configuration
    void setVolatility(float v) { m_volatility = v; }
    
    // Interval change modes
    void setCandleInterval(float interval, bool preserveHistory);
    void clearCandles();  // Clear all candles and start fresh
    
private:
    // Price state
    float m_currentPrice;
    float m_volatility;
    float m_candleInterval;
    bool m_initialized;
    float m_elapsedTime;  // Total elapsed time since start
    
    // Tick history for re-aggregation
    TickHistory m_tickHistory;
    
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
    void reaggregateFromHistory(float newInterval);
};
