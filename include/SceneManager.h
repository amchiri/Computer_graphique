#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include <algorithm>  // Ajout pour std::find
#include "GLShader.h"
#include "Mesh.h"
#include "Planet.h"
#include "Mat4.h"
#include "UI.h" // Ajouter cet include au début du fichier

// Déclarer g_UI comme externe en haut du fichier, après les includes
extern std::unique_ptr<UI> g_UI;

// Classe de base abstraite pour toutes les scènes
class Scene {
public:
    Scene(const std::string& name) : m_name(name) {
        m_lightColor[0] = m_lightColor[1] = m_lightColor[2] = 1.0f;
        m_lightIntensity = 1.0f;
    }
    virtual ~Scene() = default;

    // Méthodes virtuelles pures que chaque scène doit implémenter
    virtual bool Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render(const Mat4& projection, const Mat4& view) = 0;  // Supprimé les paramètres de shader
    virtual void Cleanup() = 0;

    // Ajout des méthodes de gestion des shaders
    virtual bool InitializeShaders();
    virtual void CleanupShaders();

    // Accesseurs
    const std::string& GetName() const { return m_name; }
    const std::vector<Mesh*>& GetObjects() const { return m_objects; }
    Mesh* GetSun() const { return m_sun; }
    const std::vector<Planet>& GetPlanets() const { return m_planets; }

    // Accesseurs pour les shaders
    GLShader& GetBasicShader() { return m_basicShader; }
    GLShader& GetColorShader() { return m_colorShader; }
    GLShader& GetEnvMapShader() { return m_envMapShader; }

    // Ajouter cette méthode
    virtual void AddObject(Mesh* object) { 
        if (object) {
            m_objects.push_back(object);
        }
    }

    // Ajouter cette méthode
    virtual void RemoveObject(Mesh* object) {
        auto it = std::find(m_objects.begin(), m_objects.end(), object);
        if (it != m_objects.end()) {
            delete *it;
            m_objects.erase(it);
        }
    }

    // Ajouter la déclaration de la fonction GetShaderPath
    std::string GetShaderPath(const std::string& filename);

protected:
    std::string m_name;
    std::vector<Mesh*> m_objects;
    std::vector<Planet> m_planets;
    Mesh* m_sun = nullptr;
    float m_lightColor[3];
    float m_lightIntensity;

    // Déplacer les shaders en protected pour que les classes dérivées y accèdent
    GLShader m_basicShader;
    GLShader m_colorShader;
    GLShader m_envMapShader;
};

// Scène du système solaire
class SolarSystemScene : public Scene {
public:
    SolarSystemScene() : Scene("Solar System") {}
    virtual ~SolarSystemScene() override;

    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render(const Mat4& projection, const Mat4& view) override;
    void Cleanup() override;

private:
    void createSun();
    void createPlanets();
    void loadPlanetTextures();
    void updatePlanets(float deltaTime);
    void setupBasicShader(GLuint program, Mesh* obj, float* light_color, float light_intensity, const float* cameraPos);
    void setupColorShader(GLuint program, Mesh* obj);
    void setupEnvMapShader(GLuint program, Mesh* obj, const float* cameraPos);
};

// Scène de démonstration
class DemoScene : public Scene {
public:
    DemoScene(const std::string& name = "Demo Scene") : Scene(name) {}
    virtual ~DemoScene() override;

    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render(const Mat4& projection, const Mat4& view) override;
    void Cleanup() override;

private:
    void createDemoObjects();
    void setupBasicShaderDemo(GLuint program, Mesh* obj, float* light_color, float light_intensity, float* lightPos, const float* cameraPos);
    void setupColorShaderDemo(GLuint program, Mesh* obj);
    void setupEnvMapShaderDemo(GLuint program, Mesh* obj, const float* cameraPos);
    float m_rotationTime = 0.0f;
};

// Ajouter après la classe DemoScene
class EmptyScene : public Scene {
public:
    EmptyScene(const std::string& name);
    virtual ~EmptyScene() override = default;

    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render(const Mat4& projection, const Mat4& view) override;
    void Cleanup() override;
};

// Gestionnaire de scènes
class SceneManager {
public:
    static SceneManager& GetInstance();
    
    // Gestion des scènes
    void AddScene(std::unique_ptr<Scene> scene);
    bool SetActiveScene(const std::string& sceneName);
    Scene* GetActiveScene() const { return m_activeScene; }
    void RemoveScene(const std::string& name);
    
    // Méthodes de cycle de vie
    bool Initialize();
    void Update(float deltaTime);
    void Render(const Mat4& projection, const Mat4& view);
    void Cleanup();
    
    // Utilitaires
    std::vector<std::string> GetSceneNames() const;
    void NextScene();
    void PreviousScene();

private:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::map<std::string, std::unique_ptr<Scene>> m_scenes;
    Scene* m_activeScene = nullptr;
    std::string m_activeSceneName;
    std::vector<std::string> m_sceneOrder;
};