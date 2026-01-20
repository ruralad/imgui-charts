#include "mock_ticker.h"
#include <stdlib.h>

MockTicker::MockTicker()
    : m_currentPrice(100.0f)
    , m_volatility(0.5f)
    , m_candleInterval(1.0f)
    , m_initialized(false)
    , m_elapsedTime(0.0f)
    , m_candleTimer(0.0f)
    , m_tickCount(0)
    , m_tickRateTimer(0.0f)
    , m_ticksPerSecond(0.0f)
{
}

float MockTicker::randomWalk()
{
    return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

void MockTicker::update(float deltaTime)
{
    // Initialize on first call
    if (!m_initialized)
    {
        srand(42);  // Fixed seed for reproducibility
        m_currentPrice = 100.0f;
        m_currentCandle = Candle(m_currentPrice, m_currentPrice, m_currentPrice, m_currentPrice);
        m_initialized = true;
    }
    
    // Update elapsed time
    m_elapsedTime += deltaTime;
    
    // Track tick rate
    m_tickCount++;
    m_tickRateTimer += deltaTime;
    if (m_tickRateTimer >= 1.0f)
    {
        m_ticksPerSecond = (float)m_tickCount / m_tickRateTimer;
        m_tickCount = 0;
        m_tickRateTimer = 0.0f;
    }
    
    // Random walk price update (volatility scaled by time)
    float priceChange = randomWalk() * m_volatility * deltaTime * 60.0f;
    m_currentPrice += priceChange;
    
    // Clamp price to reasonable range
    if (m_currentPrice < 10.0f) m_currentPrice = 10.0f;
    if (m_currentPrice > 500.0f) m_currentPrice = 500.0f;
    
    // Store tick in history for potential re-aggregation
    m_tickHistory.push(Tick(m_currentPrice, m_elapsedTime));
    
    // Update current forming candle
    m_currentCandle.close = m_currentPrice;
    if (m_currentPrice > m_currentCandle.high) m_currentCandle.high = m_currentPrice;
    if (m_currentPrice < m_currentCandle.low) m_currentCandle.low = m_currentPrice;
    
    // Check if candle interval elapsed
    m_candleTimer += deltaTime;
    if (m_candleTimer >= m_candleInterval)
    {
        finalizeCandle();
    }
}

void MockTicker::setCandleInterval(float interval, bool preserveHistory)
{
    if (interval <= 0.0f) return;
    if (interval == m_candleInterval) return;
    
    if (preserveHistory)
    {
        // Re-aggregate all candles from tick history
        reaggregateFromHistory(interval);
    }
    else
    {
        // Clear mode: just clear candles and start fresh
        clearCandles();
        m_candleInterval = interval;
    }
}

void MockTicker::clearCandles()
{
    // Clear the candle buffer by creating a new one
    m_candleBuffer = CandleBuffer();
    
    // Reset current candle
    m_currentCandle = Candle(m_currentPrice, m_currentPrice, m_currentPrice, m_currentPrice);
    m_candleTimer = 0.0f;
}

void MockTicker::reaggregateFromHistory(float newInterval)
{
    // Clear existing candles
    m_candleBuffer = CandleBuffer();
    
    int tickCount = m_tickHistory.count();
    if (tickCount == 0)
    {
        m_candleInterval = newInterval;
        m_candleTimer = 0.0f;
        m_currentCandle = Candle(m_currentPrice, m_currentPrice, m_currentPrice, m_currentPrice);
        return;
    }
    
    // Get the start time of our history
    float startTime = m_tickHistory.getStartTime();
    float currentCandleStart = startTime;
    
    // Initialize first candle from first tick
    Tick firstTick = m_tickHistory.get(0);
    Candle candle(firstTick.price, firstTick.price, firstTick.price, firstTick.price);
    
    // Iterate through all ticks and aggregate into candles
    for (int i = 1; i < tickCount; i++)
    {
        const Tick& tick = m_tickHistory.get(i);
        
        // Check if this tick belongs to the current candle or starts a new one
        float tickCandleStart = startTime + ((int)((tick.timestamp - startTime) / newInterval)) * newInterval;
        
        if (tickCandleStart > currentCandleStart)
        {
            // Finalize current candle
            m_candleBuffer.push(candle);
            
            // Start new candle
            currentCandleStart = tickCandleStart;
            candle = Candle(tick.price, tick.price, tick.price, tick.price);
        }
        else
        {
            // Update current candle with this tick
            candle.close = tick.price;
            if (tick.price > candle.high) candle.high = tick.price;
            if (tick.price < candle.low) candle.low = tick.price;
        }
    }
    
    // The last candle becomes the current forming candle
    m_currentCandle = candle;
    m_candleInterval = newInterval;
    
    // Calculate how much time has passed in the current candle
    float lastTickTime = m_tickHistory.get(tickCount - 1).timestamp;
    m_candleTimer = lastTickTime - currentCandleStart;
}

void MockTicker::finalizeCandle()
{
    m_candleTimer -= m_candleInterval;
    
    // Push completed candle to buffer
    m_candleBuffer.push(m_currentCandle);
    
    // Start new candle
    m_currentCandle = Candle(m_currentPrice, m_currentPrice, m_currentPrice, m_currentPrice);
}
