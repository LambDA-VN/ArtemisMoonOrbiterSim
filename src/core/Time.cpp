#include "Time.h"
#include "Constants.h"
#include <algorithm>

void Time::init() {
    m_startTime = std::chrono::high_resolution_clock::now();
    m_lastFrameTime = m_startTime;
    m_simulationTime = 0.0;
    m_realTime = 0.0;
    m_accumulator = 0.0;
    m_timeWarp = 1;
    m_warpLevelIndex = 0;
    m_paused = false;
}

void Time::update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = currentTime - m_lastFrameTime;
    m_deltaTime = elapsed.count();
    m_lastFrameTime = currentTime;
    
    // Clamp delta time to avoid spiral of death
    m_deltaTime = std::min(m_deltaTime, 0.25);
    
    m_frameTime = m_deltaTime * 1000.0; // Convert to ms
    
    std::chrono::duration<double> totalElapsed = currentTime - m_startTime;
    m_realTime = totalElapsed.count();
    
    if (!m_paused) {
        m_simulationTime += m_deltaTime * m_timeWarp;
    }
}

void Time::setTimeWarp(int warp) {
    m_timeWarp = std::clamp(warp, 1, Constants::MAX_TIME_WARP);
    // Update warp level index
    for (int i = 0; i < NUM_WARP_LEVELS; ++i) {
        if (TIME_WARP_LEVELS[i] == m_timeWarp) {
            m_warpLevelIndex = i;
            break;
        }
    }
}

void Time::increaseTimeWarp() {
    if (m_warpLevelIndex < NUM_WARP_LEVELS - 1) {
        m_warpLevelIndex++;
        m_timeWarp = TIME_WARP_LEVELS[m_warpLevelIndex];
    }
}

void Time::decreaseTimeWarp() {
    if (m_warpLevelIndex > 0) {
        m_warpLevelIndex--;
        m_timeWarp = TIME_WARP_LEVELS[m_warpLevelIndex];
    }
}

void Time::togglePause() {
    m_paused = !m_paused;
}

void Time::reset() {
    m_simulationTime = 0.0;
    m_accumulator = 0.0;
}
