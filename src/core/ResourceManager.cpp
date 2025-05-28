#include "ResourceManager.h"
#include <iostream>

ResourceManager& ResourceManager::Get() {
    static ResourceManager instance;
    return instance;
}

bool ResourceManager::LoadShader(const std::string& name, const char* vertexPath, const char* fragmentPath) {
    auto shader = std::make_unique<GLShader>();
    
    std::string shaderDir = "src/shaders/";
    std::string fullVertexPath = shaderDir + vertexPath;
    std::string fullFragmentPath = shaderDir + fragmentPath;
    
    // Print debug info
    std::cout << "Loading shader '" << name << "'" << std::endl;
    std::cout << "  Vertex shader: " << fullVertexPath << std::endl;
    std::cout << "  Fragment shader: " << fullFragmentPath << std::endl;

    if (!shader->LoadVertexShader(fullVertexPath.c_str())) {
        std::cerr << "Failed to load vertex shader: " << fullVertexPath << std::endl;
        return false;
    }

    if (!shader->LoadFragmentShader(fullFragmentPath.c_str())) {
        std::cerr << "Failed to load fragment shader: " << fullFragmentPath << std::endl;
        return false;
    }

    if (!shader->Create()) {
        std::cerr << "Failed to create shader program: " << name << std::endl;
        return false;
    }

    std::cout << "Successfully loaded shader '" << name << "'" << std::endl;
    m_Shaders[name] = std::move(shader);
    return true;
}

GLShader* ResourceManager::GetShader(const std::string& name) {
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ResourceManager::Clear() {
    for (auto& pair : m_Shaders) {
        if (pair.second) {
            pair.second->Destroy();
        }
    }
    m_Shaders.clear();
}

ResourceManager::~ResourceManager() {
    Clear();
}
