#pragma once

#include <chrono>

class Time {
public:
    void init();
    void update();
    
    double getDeltaTime() const { return m_deltaTime; }
    double getSimulationTime() const { return m_simulationTime; }
    double getRealTime() const { return m_realTime; }
    int getTimeWarp() const { return m_timeWarp; }
    bool isPaused() const { return m_paused; }
    
    void setTimeWarp(int warp);
    void increaseTimeWarp();
    void decreaseTimeWarp();
    void togglePause();
    void setPaused(bool paused) { m_paused = paused; }
    void reset();
    
    double getFrameTime() const { return m_frameTime; }
    double getPhysicsTime() const { return m_physicsTime; }
    double getRenderTime() const { return m_renderTime; }
    
    void setPhysicsTime(double time) { m_physicsTime = time; }
    void setRenderTime(double time) { m_renderTime = time; }
    
private:
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    
    double m_deltaTime = 0.0;
    double m_simulationTime = 0.0;
    double m_realTime = 0.0;
    double m_accumulator = 0.0;
    int m_timeWarp = 1;
    bool m_paused = false;
    
    double m_frameTime = 0.0;
    double m_physicsTime = 0.0;
    double m_renderTime = 0.0;
    
    static constexpr int TIME_WARP_LEVELS[] = {1, 2, 5, 10, 50, 100};
    static constexpr int NUM_WARP_LEVELS = 6;
    int m_warpLevelIndex = 0;
};
