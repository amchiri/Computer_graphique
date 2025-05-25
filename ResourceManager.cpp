#include "ResourceManager.h"
#include <iostream>

ResourceManager& ResourceManager::Get() {
    static ResourceManager instance;
    return instance;
}

bool ResourceManager::LoadShader(const std::string& name, const char* vertexPath, const char* fragmentPath) {
    auto shader = std::make_unique<GLShader>();
    
    if (!shader->LoadVertexShader(vertexPath)) {
        std::cerr << "Failed to load vertex shader: " << vertexPath << std::endl;
        return false;
    }

    if (!shader->LoadFragmentShader(fragmentPath)) {
        std::cerr << "Failed to load fragment shader: " << fragmentPath << std::endl;
        return false;
    }

    if (!shader->Create()) {
        std::cerr << "Failed to create shader program: " << name << std::endl;
        return false;
    }

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
