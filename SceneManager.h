#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include "GLShader.h"
#include "Mesh.h"
#include "Planet.h"
#include "Mat4.h"

// Classe de base abstraite pour toutes les scènes
class Scene {
public:
    Scene(const std::string& name) : m_name(name) {}
    virtual ~Scene() = default;

    // Méthodes virtuelles pures que chaque scène doit implémenter
    virtual bool Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render(const Mat4& projection, const Mat4& view, 
                       GLShader* basicShader, GLShader* colorShader, GLShader* envMapShader) = 0;
    virtual void Cleanup() = 0;

    // Accesseurs
    const std::string& GetName() const { return m_name; }
    const std::vector<Mesh*>& GetObjects() const { return m_objects; }
    Mesh* GetSun() const { return m_sun; }
    const std::vector<Planet>& GetPlanets() const { return m_planets; }

protected:
    std::string m_name;
    std::vector<Mesh*> m_objects;
    std::vector<Planet> m_planets;
    Mesh* m_sun = nullptr;
};

// Scène du système solaire
class SolarSystemScene : public Scene {
public:
    SolarSystemScene() : Scene("Solar System") {}
    virtual ~SolarSystemScene() override;

    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render(const Mat4& projection, const Mat4& view, 
               GLShader* basicShader, GLShader* colorShader, GLShader* envMapShader) override;
    void Cleanup() override;

private:
    void createSun();
    void createPlanets();
    void loadPlanetTextures();
    void updatePlanets(float deltaTime);
};

// Scène de démonstration
class DemoScene : public Scene {
public:
    DemoScene() : Scene("Demo Scene") {}
    virtual ~DemoScene() override;

    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render(const Mat4& projection, const Mat4& view, 
               GLShader* basicShader, GLShader* colorShader, GLShader* envMapShader) override;
    void Cleanup() override;

private:
    void createDemoObjects();
    float m_rotationTime = 0.0f;
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
    void Render(const Mat4& projection, const Mat4& view,
               GLShader* basicShader, GLShader* colorShader, GLShader* envMapShader);
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