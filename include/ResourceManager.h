#pragma once
#include "GLShader.h"
#include <string>
#include <unordered_map>
#include <memory>

class ResourceManager {
public:
    static ResourceManager& Get();  // Declaration only

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    bool LoadShader(const std::string& name, const char* vertexPath, const char* fragmentPath);
    GLShader* GetShader(const std::string& name);
    void Clear();

private:
    ResourceManager() = default;
    ~ResourceManager();

    std::unordered_map<std::string, std::unique_ptr<GLShader>> m_Shaders;
};
