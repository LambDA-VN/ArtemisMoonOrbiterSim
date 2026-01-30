#pragma once

#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include "physics/Spacecraft.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

class Renderer {
public:
    bool init(int width, int height);
    void shutdown();
    
    void beginFrame();
    void endFrame();
    void resize(int width, int height);
    
    // Rendering
    void renderMoon();
    void renderSpacecraft(const SpacecraftState& state, float throttle);
    void renderOrbitPath(const std::vector<glm::dvec3>& trajectory, const glm::vec3& color);
    void renderVector(const glm::dvec3& origin, const glm::dvec3& direction, 
                     float length, const glm::vec3& color);
    
    // Camera
    Camera& getCamera() { return m_camera; }
    const Camera& getCamera() const { return m_camera; }
    
    // Settings
    bool getShowOrbitPath() const { return m_showOrbitPath; }
    void setShowOrbitPath(bool show) { m_showOrbitPath = show; }
    
    bool getShowVelocityVector() const { return m_showVelocityVector; }
    void setShowVelocityVector(bool show) { m_showVelocityVector = show; }
    
    bool getShowThrustVector() const { return m_showThrustVector; }
    void setShowThrustVector(bool show) { m_showThrustVector = show; }
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
private:
    bool initShaders();
    void createMeshes();
    
    int m_width = 1280;
    int m_height = 720;
    
    Camera m_camera;
    
    // Shaders
    Shader m_litShader;
    Shader m_unlitShader;
    Shader m_lineShader;
    
    // Meshes
    Mesh m_moonMesh;
    Mesh m_spacecraftMesh;
    Mesh m_thrustConeMesh;
    Mesh m_arrowMesh;
    Mesh m_orbitPathMesh;
    
    // Rendering options
    bool m_showOrbitPath = true;
    bool m_showVelocityVector = false;
    bool m_showThrustVector = true;
    
    // Moon texture
    unsigned int m_moonTexture = 0;
    bool m_hasMoonTexture = false;
};
