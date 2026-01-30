#pragma once

#include <vector>
#include <glm/glm.hpp>

class Mesh {
public:
    Mesh() = default;
    ~Mesh();
    
    // Prevent copying
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    
    // Allow moving
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;
    
    // Create primitive meshes
    void createSphere(float radius, int sectors, int stacks);
    void createCone(float radius, float height, int sectors);
    void createArrow(float length, float radius);
    
    // Create from raw data
    void create(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
    
    // Create line strip for orbit path
    void createLineStrip(const std::vector<glm::vec3>& points);
    void updateLineStrip(const std::vector<glm::vec3>& points);
    
    void draw() const;
    void drawLines() const;
    
    bool isValid() const { return m_vao != 0; }
    
private:
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ebo = 0;
    unsigned int m_indexCount = 0;
    unsigned int m_vertexCount = 0;
    bool m_isLineStrip = false;
    
    void cleanup();
};
