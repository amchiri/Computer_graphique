#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>

class CubeMap {
private:
    GLuint m_TextureID;
    bool m_IsLoaded;

public:
    CubeMap();
    ~CubeMap();
    
    // Charge un cubemap à partir de 6 images individuelles
    bool LoadFromImages(const std::vector<std::string>& faces);
    
    // Crée un cubemap procédural simple
    bool CreateProcedural();
    
    // Charge un cubemap depuis des fichiers spécifiés par l'utilisateur
    bool LoadFromFiles(const std::string& posX, const std::string& negX,
                       const std::string& posY, const std::string& negY,
                       const std::string& posZ, const std::string& negZ);
    
    // Retourne l'ID de texture OpenGL
    GLuint GetTextureID() const { return m_TextureID; }
    
    // Est-ce que le cubemap est chargé ?
    bool IsLoaded() const { return m_IsLoaded; }
    
    // Lie le cubemap à l'unité de texture spécifiée
    void Bind(GLuint unit = 0);
    
    // Recharge le cubemap
    void Reload();
};
