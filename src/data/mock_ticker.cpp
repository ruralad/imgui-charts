#include "mock_ticker.h"
#include <stdlib.h>

MockTicker::MockTicker()
    : m_currentPrice(100.0f)
    , m_volatility(0.5f)
    , m_initialized(false)
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
    
    // Update current forming candle
    m_currentCandle.close = m_currentPrice;
    if (m_currentPrice > m_currentCandle.high) m_currentCandle.high = m_currentPrice;
    if (m_currentPrice < m_currentCandle.low) m_currentCandle.low = m_currentPrice;
    
    // Check if candle interval elapsed
    m_candleTimer += deltaTime;
    if (m_candleTimer >= CANDLE_INTERVAL)
    {
        finalizeCandle();
    }
}

void MockTicker::finalizeCandle()
{
    m_candleTimer -= CANDLE_INTERVAL;
    
    // Push completed candle to buffer
    m_candleBuffer.push(m_currentCandle);
    
    // Start new candle
    m_currentCandle = Candle(m_currentPrice, m_currentPrice, m_currentPrice, m_currentPrice);
}
