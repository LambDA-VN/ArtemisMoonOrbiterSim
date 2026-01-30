#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader() = default;
    ~Shader();
    
    // Prevent copying
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    
    // Allow moving
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;
    
    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    void use() const;
    
    // Uniform setters
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;
    
    unsigned int getProgram() const { return m_program; }
    bool isValid() const { return m_program != 0; }
    
private:
    unsigned int m_program = 0;
    
    bool compileShader(unsigned int shader, const std::string& source);
    bool linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
};
