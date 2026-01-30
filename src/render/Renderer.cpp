#include "Renderer.h"
#include "core/Constants.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Shader sources
static const char* litVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = normalMatrix * aNormal;
    TexCoord = aTexCoord;
    gl_Position = projection * view * worldPos;
}
)";

static const char* litFragmentShader = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightDir;
uniform vec3 objectColor;
uniform float ambient;
uniform float diffuseStrength;
uniform sampler2D textureSampler;
uniform bool useTexture;

out vec4 FragColor;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir);
    float diff = max(dot(norm, lightDirection), 0.0) * diffuseStrength;
    
    vec3 baseColor;
    if (useTexture) {
        baseColor = texture(textureSampler, TexCoord).rgb;
    } else {
        baseColor = objectColor;
    }
    
    vec3 result = (ambient + diff) * baseColor;
    FragColor = vec4(result, 1.0);
}
)";

static const char* unlitVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

static const char* unlitFragmentShader = R"(
#version 330 core
uniform vec3 objectColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(objectColor, 1.0);
}
)";

static const char* lineVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

static const char* lineFragmentShader = R"(
#version 330 core
uniform vec3 lineColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(lineColor, 1.0);
}
)";

bool Renderer::init(int width, int height) {
    m_width = width;
    m_height = height;
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Enable blending for transparent elements
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable line smoothing
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0f);
    
    if (!initShaders()) {
        return false;
    }
    
    createMeshes();
    m_camera.init();
    
    // Try to load moon texture
    stbi_set_flip_vertically_on_load(true);
    int texWidth, texHeight, nrChannels;
    unsigned char* data = stbi_load("assets/textures/moon_albedo.png", 
                                    &texWidth, &texHeight, &nrChannels, 0);
    if (data) {
        glGenTextures(1, &m_moonTexture);
        glBindTexture(GL_TEXTURE_2D, m_moonTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, 
                    format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        stbi_image_free(data);
        m_hasMoonTexture = true;
        std::cout << "Loaded moon texture: " << texWidth << "x" << texHeight << std::endl;
    } else {
        std::cout << "No moon texture found, using procedural color" << std::endl;
    }
    
    return true;
}

void Renderer::shutdown() {
    if (m_moonTexture != 0) {
        glDeleteTextures(1, &m_moonTexture);
        m_moonTexture = 0;
    }
}

bool Renderer::initShaders() {
    if (!m_litShader.loadFromSource(litVertexShader, litFragmentShader)) {
        std::cerr << "Failed to load lit shader" << std::endl;
        return false;
    }
    
    if (!m_unlitShader.loadFromSource(unlitVertexShader, unlitFragmentShader)) {
        std::cerr << "Failed to load unlit shader" << std::endl;
        return false;
    }
    
    if (!m_lineShader.loadFromSource(lineVertexShader, lineFragmentShader)) {
        std::cerr << "Failed to load line shader" << std::endl;
        return false;
    }
    
    return true;
}

void Renderer::createMeshes() {
    // Moon sphere (radius in render units = km)
    float moonRadiusKm = static_cast<float>(Constants::MOON_RADIUS / Constants::RENDER_SCALE);
    m_moonMesh.createSphere(moonRadiusKm, 64, 32);
    
    // Spacecraft (simple arrow shape)
    m_spacecraftMesh.createArrow(20.0f, 5.0f);  // 20km long arrow for visibility
    
    // Thrust cone
    m_thrustConeMesh.createCone(3.0f, 15.0f, 16);
    
    // Direction arrow for vectors
    m_arrowMesh.createArrow(1.0f, 0.1f);
}

void Renderer::beginFrame() {
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);  // Dark space background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::endFrame() {
    // Nothing special needed
}

void Renderer::resize(int width, int height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
}

void Renderer::renderMoon() {
    m_litShader.use();
    
    float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    glm::mat4 view = m_camera.getViewMatrix();
    glm::mat4 projection = m_camera.getProjectionMatrix(aspectRatio);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    
    m_litShader.setMat4("model", model);
    m_litShader.setMat4("view", view);
    m_litShader.setMat4("projection", projection);
    m_litShader.setMat4("normalMatrix", glm::mat4(normalMatrix));
    
    // Sun direction (normalized)
    glm::vec3 sunDir = glm::normalize(glm::vec3(1.0f, 0.2f, 0.1f));
    m_litShader.setVec3("lightDir", sunDir);
    m_litShader.setFloat("ambient", 0.15f);
    m_litShader.setFloat("diffuseStrength", 0.85f);
    
    if (m_hasMoonTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_moonTexture);
        m_litShader.setInt("textureSampler", 0);
        m_litShader.setInt("useTexture", 1);
    } else {
        m_litShader.setInt("useTexture", 0);
        m_litShader.setVec3("objectColor", glm::vec3(0.7f, 0.7f, 0.7f));  // Gray moon
    }
    
    m_moonMesh.draw();
}

void Renderer::renderSpacecraft(const SpacecraftState& state, float throttle) {
    m_litShader.use();
    
    float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    glm::mat4 view = m_camera.getViewMatrix();
    glm::mat4 projection = m_camera.getProjectionMatrix(aspectRatio);
    
    // Convert position to render units (meters to km)
    glm::vec3 pos = glm::vec3(state.position / Constants::RENDER_SCALE);
    
    // Create model matrix with position and orientation
    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
    
    // Apply attitude quaternion
    glm::mat4 rotation = glm::mat4_cast(glm::quat(
        static_cast<float>(state.attitude.w),
        static_cast<float>(state.attitude.x),
        static_cast<float>(state.attitude.y),
        static_cast<float>(state.attitude.z)
    ));
    
    // Rotate to align with velocity direction for prograde orientation
    glm::vec3 velocity = glm::normalize(glm::vec3(state.velocity));
    if (glm::length(glm::vec3(state.velocity)) > 0.01f) {
        glm::vec3 up = glm::normalize(pos);
        glm::vec3 right = glm::cross(velocity, up);
        if (glm::length(right) > 0.001f) {
            right = glm::normalize(right);
            up = glm::normalize(glm::cross(right, velocity));
            
            glm::mat4 orientMat(1.0f);
            orientMat[0] = glm::vec4(right, 0.0f);
            orientMat[1] = glm::vec4(velocity, 0.0f);
            orientMat[2] = glm::vec4(up, 0.0f);
            model = model * orientMat;
        }
    }
    
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
    
    m_litShader.setMat4("model", model);
    m_litShader.setMat4("view", view);
    m_litShader.setMat4("projection", projection);
    m_litShader.setMat4("normalMatrix", glm::mat4(normalMatrix));
    
    glm::vec3 sunDir = glm::normalize(glm::vec3(1.0f, 0.2f, 0.1f));
    m_litShader.setVec3("lightDir", sunDir);
    m_litShader.setFloat("ambient", 0.3f);
    m_litShader.setFloat("diffuseStrength", 0.7f);
    m_litShader.setInt("useTexture", 0);
    m_litShader.setVec3("objectColor", glm::vec3(0.9f, 0.9f, 0.95f));  // White spacecraft
    
    m_spacecraftMesh.draw();
    
    // Render thrust cone if throttle > 0
    if (throttle > 0.01f && m_showThrustVector) {
        // Position thrust cone behind spacecraft
        glm::mat4 thrustModel = model;
        thrustModel = glm::translate(thrustModel, glm::vec3(0.0f, -10.0f, 0.0f));
        thrustModel = glm::scale(thrustModel, glm::vec3(1.0f, throttle, 1.0f));
        
        m_unlitShader.use();
        m_unlitShader.setMat4("model", thrustModel);
        m_unlitShader.setMat4("view", view);
        m_unlitShader.setMat4("projection", projection);
        m_unlitShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f + throttle * 0.3f, 0.2f));  // Orange flame
        
        glDisable(GL_CULL_FACE);
        m_thrustConeMesh.draw();
        glEnable(GL_CULL_FACE);
    }
}

void Renderer::renderOrbitPath(const std::vector<glm::dvec3>& trajectory, const glm::vec3& color) {
    if (trajectory.empty() || !m_showOrbitPath) return;
    
    // Convert trajectory to render units
    std::vector<glm::vec3> renderPoints;
    renderPoints.reserve(trajectory.size());
    for (const auto& point : trajectory) {
        renderPoints.push_back(glm::vec3(point / Constants::RENDER_SCALE));
    }
    
    m_orbitPathMesh.updateLineStrip(renderPoints);
    
    m_lineShader.use();
    
    float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    glm::mat4 view = m_camera.getViewMatrix();
    glm::mat4 projection = m_camera.getProjectionMatrix(aspectRatio);
    
    m_lineShader.setMat4("view", view);
    m_lineShader.setMat4("projection", projection);
    m_lineShader.setVec3("lineColor", color);
    
    glDisable(GL_DEPTH_TEST);
    m_orbitPathMesh.drawLines();
    glEnable(GL_DEPTH_TEST);
}

void Renderer::renderVector(const glm::dvec3& origin, const glm::dvec3& direction, 
                           float length, const glm::vec3& color) {
    if (glm::length(direction) < 0.001) return;
    
    glm::vec3 pos = glm::vec3(origin / Constants::RENDER_SCALE);
    glm::vec3 dir = glm::normalize(glm::vec3(direction));
    
    // Create rotation to align Y-axis with direction
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 axis = glm::cross(up, dir);
    float angle = std::acos(std::clamp(glm::dot(up, dir), -1.0f, 1.0f));
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
    if (glm::length(axis) > 0.001f) {
        model = glm::rotate(model, angle, glm::normalize(axis));
    } else if (glm::dot(up, dir) < 0.0f) {
        model = glm::rotate(model, glm::pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    model = glm::scale(model, glm::vec3(length));
    
    m_unlitShader.use();
    
    float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    glm::mat4 view = m_camera.getViewMatrix();
    glm::mat4 projection = m_camera.getProjectionMatrix(aspectRatio);
    
    m_unlitShader.setMat4("model", model);
    m_unlitShader.setMat4("view", view);
    m_unlitShader.setMat4("projection", projection);
    m_unlitShader.setVec3("objectColor", color);
    
    m_arrowMesh.draw();
}
