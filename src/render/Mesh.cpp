#include "Mesh.h"
#include <glad/gl.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Mesh::~Mesh() {
    cleanup();
}

Mesh::Mesh(Mesh&& other) noexcept 
    : m_vao(other.m_vao), m_vbo(other.m_vbo), m_ebo(other.m_ebo),
      m_indexCount(other.m_indexCount), m_vertexCount(other.m_vertexCount),
      m_isLineStrip(other.m_isLineStrip) {
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        cleanup();
        m_vao = other.m_vao;
        m_vbo = other.m_vbo;
        m_ebo = other.m_ebo;
        m_indexCount = other.m_indexCount;
        m_vertexCount = other.m_vertexCount;
        m_isLineStrip = other.m_isLineStrip;
        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ebo = 0;
    }
    return *this;
}

void Mesh::cleanup() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ebo != 0) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
}

void Mesh::createSphere(float radius, int sectors, int stacks) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    float sectorStep = 2.0f * static_cast<float>(M_PI) / sectors;
    float stackStep = static_cast<float>(M_PI) / stacks;
    
    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = static_cast<float>(M_PI) / 2.0f - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);
        
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            
            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // Normal
            float nx = x / radius;
            float ny = y / radius;
            float nz = z / radius;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
            
            // UV
            float u = static_cast<float>(j) / sectors;
            float v = static_cast<float>(i) / stacks;
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }
    
    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        
        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != stacks - 1) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
    
    create(vertices, indices);
}

void Mesh::createCone(float radius, float height, int sectors) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    float sectorStep = 2.0f * static_cast<float>(M_PI) / sectors;
    
    // Tip vertex
    vertices.push_back(0.0f);   // x
    vertices.push_back(height); // y
    vertices.push_back(0.0f);   // z
    vertices.push_back(0.0f);   // nx
    vertices.push_back(1.0f);   // ny
    vertices.push_back(0.0f);   // nz
    vertices.push_back(0.5f);   // u
    vertices.push_back(1.0f);   // v
    
    // Base vertices
    for (int i = 0; i <= sectors; ++i) {
        float angle = i * sectorStep;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        
        // Calculate normal for cone side
        float sideNormalY = radius / height;
        float sideNormalXZ = 1.0f;
        float normalLen = sqrtf(sideNormalY * sideNormalY + sideNormalXZ * sideNormalXZ);
        
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
        vertices.push_back(cosf(angle) / normalLen);
        vertices.push_back(sideNormalY / normalLen);
        vertices.push_back(sinf(angle) / normalLen);
        vertices.push_back(static_cast<float>(i) / sectors);
        vertices.push_back(0.0f);
    }
    
    // Side triangles
    for (int i = 1; i <= sectors; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    
    create(vertices, indices);
}

void Mesh::createArrow(float length, float radius) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    int sectors = 8;
    float sectorStep = 2.0f * static_cast<float>(M_PI) / sectors;
    float shaftLength = length * 0.7f;
    float shaftRadius = radius * 0.3f;
    float headLength = length * 0.3f;
    float headRadius = radius;
    
    // Shaft base center
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
    vertices.push_back(0.5f); vertices.push_back(0.0f);
    
    // Shaft base ring
    for (int i = 0; i <= sectors; ++i) {
        float angle = i * sectorStep;
        float x = shaftRadius * cosf(angle);
        float z = shaftRadius * sinf(angle);
        vertices.push_back(x); vertices.push_back(0.0f); vertices.push_back(z);
        vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
        vertices.push_back(static_cast<float>(i) / sectors); vertices.push_back(0.0f);
    }
    
    // Shaft top ring
    for (int i = 0; i <= sectors; ++i) {
        float angle = i * sectorStep;
        float x = shaftRadius * cosf(angle);
        float z = shaftRadius * sinf(angle);
        vertices.push_back(x); vertices.push_back(shaftLength); vertices.push_back(z);
        vertices.push_back(cosf(angle)); vertices.push_back(0.0f); vertices.push_back(sinf(angle));
        vertices.push_back(static_cast<float>(i) / sectors); vertices.push_back(0.5f);
    }
    
    // Head base ring
    for (int i = 0; i <= sectors; ++i) {
        float angle = i * sectorStep;
        float x = headRadius * cosf(angle);
        float z = headRadius * sinf(angle);
        vertices.push_back(x); vertices.push_back(shaftLength); vertices.push_back(z);
        vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);
        vertices.push_back(static_cast<float>(i) / sectors); vertices.push_back(0.5f);
    }
    
    // Head tip
    vertices.push_back(0.0f); vertices.push_back(length); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
    vertices.push_back(0.5f); vertices.push_back(1.0f);
    
    // Indices
    // Shaft base
    for (int i = 0; i < sectors; ++i) {
        indices.push_back(0);
        indices.push_back(i + 2);
        indices.push_back(i + 1);
    }
    
    // Shaft sides
    int shaftBaseStart = 1;
    int shaftTopStart = shaftBaseStart + sectors + 1;
    for (int i = 0; i < sectors; ++i) {
        indices.push_back(shaftBaseStart + i);
        indices.push_back(shaftBaseStart + i + 1);
        indices.push_back(shaftTopStart + i);
        
        indices.push_back(shaftBaseStart + i + 1);
        indices.push_back(shaftTopStart + i + 1);
        indices.push_back(shaftTopStart + i);
    }
    
    // Head cone
    int headBaseStart = shaftTopStart + sectors + 1;
    int tipIndex = headBaseStart + sectors + 1;
    for (int i = 0; i < sectors; ++i) {
        indices.push_back(headBaseStart + i);
        indices.push_back(headBaseStart + i + 1);
        indices.push_back(tipIndex);
    }
    
    create(vertices, indices);
}

void Mesh::create(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    cleanup();
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    
    glBindVertexArray(m_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // UV attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    m_indexCount = static_cast<unsigned int>(indices.size());
    m_isLineStrip = false;
}

void Mesh::createLineStrip(const std::vector<glm::vec3>& points) {
    cleanup();
    
    if (points.empty()) return;
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    
    glBindVertexArray(m_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    m_vertexCount = static_cast<unsigned int>(points.size());
    m_isLineStrip = true;
}

void Mesh::updateLineStrip(const std::vector<glm::vec3>& points) {
    if (!m_isLineStrip || m_vbo == 0) {
        createLineStrip(points);
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_DYNAMIC_DRAW);
    m_vertexCount = static_cast<unsigned int>(points.size());
}

void Mesh::draw() const {
    if (m_vao == 0) return;
    
    glBindVertexArray(m_vao);
    if (m_isLineStrip) {
        glDrawArrays(GL_LINE_STRIP, 0, m_vertexCount);
    } else {
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

void Mesh::drawLines() const {
    if (m_vao == 0 || !m_isLineStrip) return;
    
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINE_STRIP, 0, m_vertexCount);
    glBindVertexArray(0);
}
