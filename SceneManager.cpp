#include "SceneManager.h"
#include <iostream>
#include <algorithm>

// ==================== SolarSystemScene Implementation ====================

SolarSystemScene::~SolarSystemScene() {
    Cleanup();
}

bool SolarSystemScene::Initialize() {
    try {
        createSun();
        createPlanets();
        loadPlanetTextures();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing Solar System Scene: " << e.what() << std::endl;
        return false;
    }
}

void SolarSystemScene::Update(float deltaTime) {
    updatePlanets(deltaTime);
}

void SolarSystemScene::Render(const Mat4& projection, const Mat4& view, 
                             GLShader* basicShader, GLShader* colorShader, GLShader* envMapShader) {
    // Variables d'éclairage (devraient être passées en paramètre dans une vraie implémentation)
    static float light_color[3] = { 1.0f, 1.0f, 1.0f };
    static float light_intensity = 1.0f;

    for (Mesh* obj : m_objects) {
        GLShader* currentShader = obj->getCurrentShader();
        if (!currentShader) {
            currentShader = basicShader;
            obj->setCurrentShader(currentShader);
        }

        GLuint program = currentShader->GetProgram();
        glUseProgram(program);

        // Matrices communes
        GLint loc_proj = glGetUniformLocation(program, "u_projection");
        if (loc_proj >= 0) glUniformMatrix4fv(loc_proj, 1, GL_FALSE, projection.data());
        
        GLint loc_view = glGetUniformLocation(program, "u_view");
        if (loc_view >= 0) glUniformMatrix4fv(loc_view, 1, GL_FALSE, view.data());

        // Matrice de transformation de l'objet
        float modelMatrix[16];
        obj->calculateModelMatrix(modelMatrix);
        GLint loc_transform = glGetUniformLocation(program, "u_transform");
        if (loc_transform >= 0) glUniformMatrix4fv(loc_transform, 1, GL_FALSE, modelMatrix);

        // Configuration spécifique selon le shader
        if (currentShader == basicShader) {
            // Éclairage
            const float* sunPos = m_sun->getPosition();
            GLint loc_lightDir = glGetUniformLocation(program, "u_light.direction");
            if (loc_lightDir >= 0) glUniform3f(loc_lightDir, sunPos[0], sunPos[1], sunPos[2]);
            
            float lightDiffuse[3] = {
                light_color[0] * light_intensity * 2.0f,
                light_color[1] * light_intensity * 2.0f,
                light_color[2] * light_intensity * 2.0f
            };

            GLint loc_lightDiffuse = glGetUniformLocation(program, "u_light.diffuseColor");
            GLint loc_lightSpecular = glGetUniformLocation(program, "u_light.specularColor");
            GLint loc_intensity = glGetUniformLocation(program, "u_intensity");
            
            if (loc_intensity >= 0) glUniform1f(loc_intensity, light_intensity);
            if (loc_lightDiffuse >= 0) glUniform3fv(loc_lightDiffuse, 1, lightDiffuse);
            if (loc_lightSpecular >= 0) glUniform3fv(loc_lightSpecular, 1, lightDiffuse);

            // Matériau
            float matDiffuse[3] = {0.8f, 0.8f, 0.8f};
            float matSpecular[3] = {1.0f, 1.0f, 1.0f};
            float matShininess = 20.0f;
            
            GLint loc_matDiffuse = glGetUniformLocation(program, "u_material.diffuseColor");
            GLint loc_matSpecular = glGetUniformLocation(program, "u_material.specularColor");
            GLint loc_matShininess = glGetUniformLocation(program, "u_material.shininess");
            
            if (loc_matDiffuse >= 0) glUniform3fv(loc_matDiffuse, 1, matDiffuse);
            if (loc_matSpecular >= 0) glUniform3fv(loc_matSpecular, 1, matSpecular);
            if (loc_matShininess >= 0) glUniform1f(loc_matShininess, matShininess);

            // Objets émissifs
            GLint loc_isEmissive = glGetUniformLocation(program, "u_material.isEmissive");
            if (loc_isEmissive >= 0) {
                bool isEmissive = (obj == m_sun);
                glUniform1i(loc_isEmissive, isEmissive ? 1 : 0);
            }
        }

        obj->draw(*currentShader);
    }
}

void SolarSystemScene::Cleanup() {
    // Nettoyage des objets qui ne sont pas des planètes
    std::vector<Mesh*> meshesToDelete;
    for(Mesh* obj : m_objects) {
        bool isPlanetMesh = false;
        for(const auto& planet : m_planets) {
            if(planet.GetMesh() == obj) {
                isPlanetMesh = true;
                break;
            }
        }
        
        if(!isPlanetMesh && obj != m_sun) {
            meshesToDelete.push_back(obj);
        }
    }

    for(Mesh* obj : meshesToDelete) {
        delete obj;
    }

    delete m_sun;
    m_sun = nullptr;
    m_objects.clear();
    m_planets.clear();
}

void SolarSystemScene::createSun() {
    m_sun = new Mesh();
    m_sun->createSphere(1.0f, 32, 32);

    const float sunScale = 8.0f;
    m_sun->setScale(sunScale, sunScale, sunScale);
    m_sun->setPosition(0.0f, 0.0f, 0.0f);

    Material sunMaterial;
    sunMaterial.diffuse[0] = sunMaterial.diffuse[1] = sunMaterial.diffuse[2] = 1.0f;
    sunMaterial.specular[0] = sunMaterial.specular[1] = sunMaterial.specular[2] = 0.0f;
    sunMaterial.shininess = 0.0f;
    sunMaterial.isEmissive = true;
    m_sun->setMaterial(sunMaterial);
    m_sun->loadTexture("models/sun.png");

    m_objects.push_back(m_sun);
}

void SolarSystemScene::createPlanets() {
    // Paramètres: rayon orbital, vitesse orbitale, taille
    float planetData[][3] = {
        {15.0f, 0.8f, 0.8f},    // Mercure
        {20.0f, 0.6f, 1.2f},    // Vénus
        {28.0f, 0.4f, 1.5f},    // Terre
        {35.0f, 0.3f, 1.2f},    // Mars
        {50.0f, 0.15f, 4.0f},   // Jupiter
        {65.0f, 0.12f, 3.5f},   // Saturne
        {80.0f, 0.08f, 2.5f},   // Uranus
    };

    for(int i = 0; i < 7; i++) {
        m_planets.emplace_back();
        m_planets.back().Initialize(
            planetData[i][0],    // rayon orbital
            planetData[i][1],    // vitesse de rotation
            planetData[i][2]     // taille
        );
        m_objects.push_back(m_planets.back().GetMesh());
    }
}

void SolarSystemScene::loadPlanetTextures() {
    const char* textures[] = {
        "models/mercury.png",
        "models/venus.png",
        "models/earth.png",
        "models/mars.png",
        "models/jupiter.png",
        "models/saturn.png",
        "models/uranus.png"
    };

    for(size_t i = 0; i < m_planets.size(); i++) {
        Planet& planet = m_planets[i];
        Material planetMaterial;
        planetMaterial.diffuse[0] = planetMaterial.diffuse[1] = planetMaterial.diffuse[2] = 1.0f;
        planetMaterial.specular[0] = planetMaterial.specular[1] = planetMaterial.specular[2] = 0.5f;
        planetMaterial.shininess = 32.0f;
        planetMaterial.isEmissive = false;
        planet.SetMaterial(planetMaterial);
        
        if (!planet.LoadTexture(textures[i])) {
            std::cerr << "Error: Could not load texture for planet " << i << std::endl;
        }
    }
}

void SolarSystemScene::updatePlanets(float deltaTime) {
    for(auto& planet : m_planets) {
        planet.Update(deltaTime);
    }
}

// ==================== DemoScene Implementation ====================

DemoScene::~DemoScene() {
    Cleanup();
}

bool DemoScene::Initialize() {
    try {
        createDemoObjects();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing Demo Scene: " << e.what() << std::endl;
        return false;
    }
}

void DemoScene::Update(float deltaTime) {
    m_rotationTime += deltaTime;
    
    // Faire tourner les objets de démonstration
    if (m_objects.size() >= 3) {
        // Rotation autour de l'axe Y
        float rotation = m_rotationTime * 0.5f;
        m_objects[0]->setRotation(0, rotation, 0);
        m_objects[1]->setRotation(0, rotation * 1.5f, 0);
        m_objects[2]->setRotation(0, rotation * 2.0f, 0);
    }
}

void DemoScene::Render(const Mat4& projection, const Mat4& view, 
                      GLShader* basicShader, GLShader* colorShader, GLShader* envMapShader) {
    // Variables d'éclairage
    static float light_color[3] = { 1.0f, 1.0f, 1.0f };
    static float light_intensity = 1.0f;
    static float lightPos[3] = { 0.0f, 10.0f, 0.0f };

    for (Mesh* obj : m_objects) {
        GLShader* currentShader = obj->getCurrentShader();
        if (!currentShader) {
            currentShader = basicShader;
            obj->setCurrentShader(currentShader);
        }

        GLuint program = currentShader->GetProgram();
        glUseProgram(program);

        // Matrices communes
        GLint loc_proj = glGetUniformLocation(program, "u_projection");
        if (loc_proj >= 0) glUniformMatrix4fv(loc_proj, 1, GL_FALSE, projection.data());
        
        GLint loc_view = glGetUniformLocation(program, "u_view");
        if (loc_view >= 0) glUniformMatrix4fv(loc_view, 1, GL_FALSE, view.data());

        // Matrice de transformation de l'objet
        float modelMatrix[16];
        obj->calculateModelMatrix(modelMatrix);
        GLint loc_transform = glGetUniformLocation(program, "u_transform");
        if (loc_transform >= 0) glUniformMatrix4fv(loc_transform, 1, GL_FALSE, modelMatrix);

        // Configuration spécifique selon le shader
        if (currentShader == colorShader) {
            GLint loc_color = glGetUniformLocation(program, "u_color");
            if (loc_color >= 0) {
                const Material& mat = obj->getMaterial();
                glUniform3fv(loc_color, 1, mat.diffuse);
            }
        }
        else if (currentShader == basicShader) {
            // Éclairage
            GLint loc_lightDir = glGetUniformLocation(program, "u_light.direction");
            if (loc_lightDir >= 0) glUniform3f(loc_lightDir, lightPos[0], lightPos[1], lightPos[2]);
            
            float lightDiffuse[3] = {
                light_color[0] * light_intensity,
                light_color[1] * light_intensity,
                light_color[2] * light_intensity
            };

            GLint loc_lightDiffuse = glGetUniformLocation(program, "u_light.diffuseColor");
            GLint loc_lightSpecular = glGetUniformLocation(program, "u_light.specularColor");
            GLint loc_intensity = glGetUniformLocation(program, "u_intensity");
            
            if (loc_intensity >= 0) glUniform1f(loc_intensity, light_intensity);
            if (loc_lightDiffuse >= 0) glUniform3fv(loc_lightDiffuse, 1, lightDiffuse);
            if (loc_lightSpecular >= 0) glUniform3fv(loc_lightSpecular, 1, lightDiffuse);

            // Matériau
            float matDiffuse[3] = {0.8f, 0.8f, 0.8f};
            float matSpecular[3] = {1.0f, 1.0f, 1.0f};
            float matShininess = 20.0f;
            
            GLint loc_matDiffuse = glGetUniformLocation(program, "u_material.diffuseColor");
            GLint loc_matSpecular = glGetUniformLocation(program, "u_material.specularColor");
            GLint loc_matShininess = glGetUniformLocation(program, "u_material.shininess");
            
            if (loc_matDiffuse >= 0) glUniform3fv(loc_matDiffuse, 1, matDiffuse);
            if (loc_matSpecular >= 0) glUniform3fv(loc_matSpecular, 1, matSpecular);
            if (loc_matShininess >= 0) glUniform1f(loc_matShininess, matShininess);

            GLint loc_isEmissive = glGetUniformLocation(program, "u_material.isEmissive");
            if (loc_isEmissive >= 0) glUniform1i(loc_isEmissive, 0);
        }

        obj->draw(*currentShader);
    }
}

void DemoScene::Cleanup() {
    for (Mesh* obj : m_objects) {
        delete obj;
    }
    m_objects.clear();
}

void DemoScene::createDemoObjects() {
    // Cube couleur (rouge)
    Mesh* colorCube = new Mesh();
    colorCube->createSphere(1.0f, 32, 32);
    Material matColor;
    matColor.diffuse[0] = 0.8f; matColor.diffuse[1] = 0.2f; matColor.diffuse[2] = 0.2f;
    matColor.specular[0] = matColor.specular[1] = matColor.specular[2] = 0.0f;
    matColor.shininess = 1.0f;
    matColor.isEmissive = false;
    colorCube->setMaterial(matColor);
    colorCube->setPosition(-8, 0, 0);
    colorCube->setCurrentShader(nullptr); // Sera défini lors du rendu
    m_objects.push_back(colorCube);

    // Cube texturé
    Mesh* texCube = new Mesh();
    texCube->createSphere(1.0f, 32, 32);
    texCube->loadTexture("models/earth.png");
    Material matTex;
    matTex.diffuse[0] = matTex.diffuse[1] = matTex.diffuse[2] = 1.0f;
    matTex.specular[0] = matTex.specular[1] = matTex.specular[2] = 0.5f;
    matTex.shininess = 16.0f;
    matTex.isEmissive = false;
    texCube->setMaterial(matTex);
    texCube->setPosition(0, 0, 0);
    texCube->setCurrentShader(nullptr);
    m_objects.push_back(texCube);

    // Cube environment mapping
    Mesh* envCube = new Mesh();
    envCube->createSphere(1.0f, 32, 32);
    Material matEnv;
    matEnv.diffuse[0] = matEnv.diffuse[1] = matEnv.diffuse[2] = 1.0f;
    matEnv.specular[0] = matEnv.specular[1] = matEnv.specular[2] = 1.0f;
    matEnv.shininess = 64.0f;
    matEnv.isEmissive = false;
    envCube->setMaterial(matEnv);
    envCube->setPosition(8, 0, 0);
    envCube->setCurrentShader(nullptr);
    m_objects.push_back(envCube);
}

// ==================== SceneManager Implementation ====================

SceneManager& SceneManager::GetInstance() {
    static SceneManager instance;
    return instance;
}

void SceneManager::AddScene(std::unique_ptr<Scene> scene) {
    std::string name = scene->GetName();
    m_scenes[name] = std::move(scene);
    m_sceneOrder.push_back(name);
}

bool SceneManager::SetActiveScene(const std::string& sceneName) {
    auto it = m_scenes.find(sceneName);
    if (it == m_scenes.end()) {
        std::cerr << "Scene '" << sceneName << "' not found" << std::endl;
        return false;
    }

    m_activeScene = it->second.get();
    m_activeSceneName = sceneName;
    return true;
}

bool SceneManager::Initialize() {
    // Initialiser toutes les scènes
    for (auto& pair : m_scenes) {
        if (!pair.second->Initialize()) {
            std::cerr << "Failed to initialize scene: " << pair.first << std::endl;
            return false;
        }
    }

    // Définir la première scène comme active
    if (!m_sceneOrder.empty()) {
        SetActiveScene(m_sceneOrder[0]);
    }

    return true;
}

void SceneManager::Update(float deltaTime) {
    if (m_activeScene) {
        m_activeScene->Update(deltaTime);
    }
}

void SceneManager::Render(const Mat4& projection, const Mat4& view,
                         GLShader* basicShader, GLShader* colorShader, GLShader* envMapShader) {
    if (m_activeScene) {
        m_activeScene->Render(projection, view, basicShader, colorShader, envMapShader);
    }
}

void SceneManager::Cleanup() {
    m_activeScene = nullptr;
    m_activeSceneName.clear();
    m_scenes.clear();
    m_sceneOrder.clear();
}

std::vector<std::string> SceneManager::GetSceneNames() const {
    return m_sceneOrder;
}

void SceneManager::NextScene() {
    if (m_sceneOrder.empty()) return;

    auto it = std::find(m_sceneOrder.begin(), m_sceneOrder.end(), m_activeSceneName);
    if (it != m_sceneOrder.end()) {
        ++it;
        if (it == m_sceneOrder.end()) {
            it = m_sceneOrder.begin();
        }
        SetActiveScene(*it);
    }
}

void SceneManager::PreviousScene() {
    if (m_sceneOrder.empty()) return;

    auto it = std::find(m_sceneOrder.begin(), m_sceneOrder.end(), m_activeSceneName);
    if (it != m_sceneOrder.end()) {
        if (it == m_sceneOrder.begin()) {
            it = m_sceneOrder.end();
        }
        --it;
        SetActiveScene(*it);
    }
}

void SceneManager::RemoveScene(const std::string& name) {
    if (name == m_activeSceneName) {
        NextScene();
    }
    
    m_scenes.erase(name);
    auto it = std::find(m_sceneOrder.begin(), m_sceneOrder.end(), name);
    if (it != m_sceneOrder.end()) {
        m_sceneOrder.erase(it);
    }
}