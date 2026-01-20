#pragma once

// ============================================================================
// CANDLE DATA STRUCTURES
// Represents OHLC (Open, High, Low, Close) candlestick data
// ============================================================================

struct Candle
{
    float open;
    float high;
    float low;
    float close;
    bool valid;
    
    Candle() : open(0), high(0), low(0), close(0), valid(false) {}
    Candle(float o, float h, float l, float c) 
        : open(o), high(h), low(l), close(c), valid(true) {}
    
    bool isBullish() const { return close >= open; }
};

// ============================================================================
// CANDLE BUFFER - Ring buffer for storing candle history
// ============================================================================

class CandleBuffer
{
public:
    static const int MAX_CANDLES = 100;
    
    CandleBuffer() : m_count(0), m_start(0) {}
    
    void push(const Candle& candle)
    {
        if (m_count < MAX_CANDLES)
        {
            m_candles[m_count] = candle;
            m_count++;
        }
        else
        {
            // Ring buffer full, overwrite oldest
            m_candles[m_start] = candle;
            m_start = (m_start + 1) % MAX_CANDLES;
        }
    }
    
    const Candle& get(int index) const
    {
        int actualIndex = (m_start + index) % MAX_CANDLES;
        return m_candles[actualIndex];
    }
    
    int count() const { return m_count; }
    int maxCandles() const { return MAX_CANDLES; }
    
    // Calculate price range across all candles
    void getPriceRange(float& minPrice, float& maxPrice) const
    {
        if (m_count == 0)
        {
            minPrice = maxPrice = 100.0f;
            return;
        }
        
        minPrice = m_candles[m_start].low;
        maxPrice = m_candles[m_start].high;
        
        for (int i = 0; i < m_count; i++)
        {
            const Candle& c = get(i);
            if (c.low < minPrice) minPrice = c.low;
            if (c.high > maxPrice) maxPrice = c.high;
        }
    }
    
private:
    Candle m_candles[MAX_CANDLES];
    int m_count;
    int m_start;
};
