#include "Camera.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

void Camera::init() {
    m_position = glm::vec3(0.0f, 0.0f, 5000.0f);
    m_front = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_worldUp = m_up;
    m_yaw = -90.0f;
    m_pitch = 0.0f;
    m_mode = Mode::OrbitAroundMoon;
    m_orbitDistance = 5000.0f;
    m_orbitYaw = 0.0f;
    m_orbitPitch = 30.0f;
    updateCameraVectors();
}

void Camera::update(float deltaTime) {
    switch (m_mode) {
        case Mode::FreeFly:
            updateFreeFly(deltaTime);
            break;
        case Mode::Chase:
            updateChase(deltaTime);
            break;
        case Mode::OrbitAroundMoon:
            updateOrbit(deltaTime);
            break;
        case Mode::TopDown:
            updateTopDown(deltaTime);
            break;
    }
}

void Camera::updateFreeFly(float deltaTime) {
    float velocity = m_moveSpeed * deltaTime;
    
    if (m_moveForward) m_position += m_front * velocity;
    if (m_moveBackward) m_position -= m_front * velocity;
    if (m_moveLeft) m_position -= m_right * velocity;
    if (m_moveRight) m_position += m_right * velocity;
    if (m_moveUp) m_position += m_up * velocity;
    if (m_moveDown) m_position -= m_up * velocity;
}

void Camera::updateChase(float deltaTime) {
    (void)deltaTime;
    
    // Position camera behind and above the target
    glm::vec3 velocityDir = glm::normalize(m_targetVelocity);
    if (glm::length(m_targetVelocity) < 0.001f) {
        velocityDir = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    glm::vec3 up = glm::normalize(m_target);
    glm::vec3 right = glm::cross(velocityDir, up);
    if (glm::length(right) > 0.001f) {
        right = glm::normalize(right);
    } else {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    m_position = m_target - velocityDir * m_chaseDistance + up * (m_chaseDistance * 0.3f);
    m_front = glm::normalize(m_target - m_position);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::updateOrbit(float deltaTime) {
    (void)deltaTime;
    
    // Calculate position on orbit around origin
    float pitchRad = glm::radians(m_orbitPitch);
    float yawRad = glm::radians(m_orbitYaw);
    
    float x = m_orbitDistance * std::cos(pitchRad) * std::cos(yawRad);
    float y = m_orbitDistance * std::sin(pitchRad);
    float z = m_orbitDistance * std::cos(pitchRad) * std::sin(yawRad);
    
    m_position = glm::vec3(x, y, z);
    m_front = glm::normalize(-m_position);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::updateTopDown(float deltaTime) {
    (void)deltaTime;
    
    m_position = glm::vec3(0.0f, m_orbitDistance, 0.0f);
    m_front = glm::vec3(0.0f, -1.0f, 0.0f);
    m_right = glm::vec3(1.0f, 0.0f, 0.0f);
    m_up = glm::vec3(0.0f, 0.0f, -1.0f);
}

void Camera::processKeyboard(int key, bool pressed) {
    switch (key) {
        case GLFW_KEY_W: m_moveForward = pressed; break;
        case GLFW_KEY_S: m_moveBackward = pressed; break;
        case GLFW_KEY_A: m_moveLeft = pressed; break;
        case GLFW_KEY_D: m_moveRight = pressed; break;
        case GLFW_KEY_Q: m_moveDown = pressed; break;
        case GLFW_KEY_E: m_moveUp = pressed; break;
    }
}

void Camera::processMouse(float xOffset, float yOffset, bool rightButtonDown) {
    if (!rightButtonDown) return;
    
    xOffset *= m_mouseSensitivity;
    yOffset *= m_mouseSensitivity;
    
    switch (m_mode) {
        case Mode::FreeFly:
            m_yaw += xOffset;
            m_pitch += yOffset;
            m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
            updateCameraVectors();
            break;
            
        case Mode::OrbitAroundMoon:
            m_orbitYaw += xOffset;
            m_orbitPitch += yOffset;
            m_orbitPitch = std::clamp(m_orbitPitch, -89.0f, 89.0f);
            break;
            
        case Mode::Chase:
            // Chase mode uses target-relative camera, not direct mouse control
            break;
            
        case Mode::TopDown:
            // Top down doesn't rotate
            break;
    }
}

void Camera::processScroll(float yOffset) {
    switch (m_mode) {
        case Mode::FreeFly:
            m_fov -= yOffset * 2.0f;
            m_fov = std::clamp(m_fov, 10.0f, 90.0f);
            break;
            
        case Mode::OrbitAroundMoon:
        case Mode::TopDown:
            m_orbitDistance -= yOffset * m_zoomSpeed;
            m_orbitDistance = std::clamp(m_orbitDistance, 100.0f, 50000.0f);
            break;
            
        case Mode::Chase:
            m_chaseDistance -= yOffset * 10.0f;
            m_chaseDistance = std::clamp(m_chaseDistance, 10.0f, 500.0f);
            break;
    }
}

void Camera::setMode(Mode mode) {
    m_mode = mode;
    
    // Reset appropriate values for the new mode
    switch (mode) {
        case Mode::FreeFly:
            updateCameraVectors();
            break;
        case Mode::OrbitAroundMoon:
            m_orbitYaw = 0.0f;
            m_orbitPitch = 30.0f;
            break;
        case Mode::TopDown:
            break;
        case Mode::Chase:
            m_chaseDistance = 50.0f;
            break;
    }
}

void Camera::setFov(float fov) {
    m_fov = std::clamp(fov, 10.0f, 120.0f);
}

void Camera::setOrbitDistance(float distance) {
    m_orbitDistance = std::clamp(distance, 100.0f, 50000.0f);
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = std::cos(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    front.y = std::sin(glm::radians(m_pitch));
    front.z = std::sin(glm::radians(m_yaw)) * std::cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    if (m_mode == Mode::TopDown) {
        float orthoSize = m_orbitDistance;
        return glm::ortho(-orthoSize * aspectRatio, orthoSize * aspectRatio,
                         -orthoSize, orthoSize, m_nearPlane, m_farPlane);
    }
    return glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
}
