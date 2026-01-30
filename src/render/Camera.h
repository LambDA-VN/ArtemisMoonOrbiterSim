#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    enum class Mode {
        FreeFly,
        Chase,
        OrbitAroundMoon,
        TopDown
    };
    
    void init();
    void update(float deltaTime);
    
    // Input handling
    void processKeyboard(int key, bool pressed);
    void processMouse(float xOffset, float yOffset, bool rightButtonDown);
    void processScroll(float yOffset);
    
    // Camera properties
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;
    
    const glm::vec3& getPosition() const { return m_position; }
    void setPosition(const glm::vec3& position) { m_position = position; }
    
    Mode getMode() const { return m_mode; }
    void setMode(Mode mode);
    
    float getFov() const { return m_fov; }
    void setFov(float fov);
    
    // Target tracking (for chase mode)
    void setTarget(const glm::vec3& target) { m_target = target; }
    void setTargetVelocity(const glm::vec3& velocity) { m_targetVelocity = velocity; }
    
    // Control settings
    float getMoveSpeed() const { return m_moveSpeed; }
    void setMoveSpeed(float speed) { m_moveSpeed = speed; }
    
    float getMouseSensitivity() const { return m_mouseSensitivity; }
    void setMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }
    
    // Orbit distance for OrbitAroundMoon mode
    float getOrbitDistance() const { return m_orbitDistance; }
    void setOrbitDistance(float distance);
    
private:
    void updateFreeFly(float deltaTime);
    void updateChase(float deltaTime);
    void updateOrbit(float deltaTime);
    void updateTopDown(float deltaTime);
    
    void updateCameraVectors();
    
    glm::vec3 m_position{0.0f, 0.0f, 5000.0f};
    glm::vec3 m_front{0.0f, 0.0f, -1.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_worldUp{0.0f, 1.0f, 0.0f};
    
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    
    float m_fov = 45.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 100000.0f;
    
    float m_moveSpeed = 500.0f;
    float m_mouseSensitivity = 0.1f;
    float m_zoomSpeed = 200.0f;
    
    Mode m_mode = Mode::OrbitAroundMoon;
    
    // Chase mode
    glm::vec3 m_target{0.0f};
    glm::vec3 m_targetVelocity{0.0f};
    float m_chaseDistance = 50.0f;
    
    // Orbit mode
    float m_orbitDistance = 5000.0f;
    float m_orbitYaw = 0.0f;
    float m_orbitPitch = 30.0f;
    
    // Input state
    bool m_moveForward = false;
    bool m_moveBackward = false;
    bool m_moveLeft = false;
    bool m_moveRight = false;
    bool m_moveUp = false;
    bool m_moveDown = false;
};
