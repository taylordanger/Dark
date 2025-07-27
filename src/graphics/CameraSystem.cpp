#include "CameraSystem.h"
#include "../components/TransformComponent.h"
#include <iostream>

namespace RPGEngine {
namespace Graphics {

// Define default camera name
const std::string CameraSystem::DEFAULT_CAMERA_NAME = "main";

CameraSystem::CameraSystem(std::shared_ptr<ComponentManager> componentManager)
    : System("CameraSystem")
    , m_componentManager(componentManager)
    , m_activeCameraName("")
{
}

CameraSystem::~CameraSystem() {
    if (isInitialized()) {
        shutdown();
    }
}

bool CameraSystem::onInitialize() {
    if (!m_componentManager) {
        std::cerr << "Component manager not provided to CameraSystem" << std::endl;
        return false;
    }
    
    // Create default camera
    createCamera(DEFAULT_CAMERA_NAME);
    setActiveCamera(DEFAULT_CAMERA_NAME);
    
    std::cout << "CameraSystem initialized" << std::endl;
    return true;
}

void CameraSystem::onUpdate(float deltaTime) {
    // Update all cameras
    for (auto& pair : m_cameras) {
        auto& camera = pair.second;
        
        // Update camera
        camera->update(deltaTime);
    }
}

void CameraSystem::onShutdown() {
    // Clear all cameras
    m_cameras.clear();
    m_activeCameraName = "";
    
    std::cout << "CameraSystem shutdown" << std::endl;
}

std::shared_ptr<Camera> CameraSystem::createCamera(const std::string& name) {
    // Check if camera already exists
    if (hasCamera(name)) {
        std::cerr << "Camera '" << name << "' already exists" << std::endl;
        return getCamera(name);
    }
    
    // Create camera
    auto camera = std::make_shared<Camera>();
    m_cameras[name] = camera;
    
    std::cout << "Created camera '" << name << "'" << std::endl;
    return camera;
}

std::shared_ptr<Camera> CameraSystem::getCamera(const std::string& name) const {
    auto it = m_cameras.find(name);
    if (it != m_cameras.end()) {
        return it->second;
    }
    
    return nullptr;
}

bool CameraSystem::removeCamera(const std::string& name) {
    // Check if camera exists
    if (!hasCamera(name)) {
        return false;
    }
    
    // Check if it's the active camera
    if (name == m_activeCameraName) {
        m_activeCameraName = "";
    }
    
    // Remove camera
    m_cameras.erase(name);
    
    std::cout << "Removed camera '" << name << "'" << std::endl;
    return true;
}

bool CameraSystem::setActiveCamera(const std::string& name) {
    // Check if camera exists
    if (!hasCamera(name)) {
        std::cerr << "Camera '" << name << "' does not exist" << std::endl;
        return false;
    }
    
    // Set active camera
    m_activeCameraName = name;
    
    std::cout << "Set active camera to '" << name << "'" << std::endl;
    return true;
}

std::shared_ptr<Camera> CameraSystem::getActiveCamera() const {
    return getCamera(m_activeCameraName);
}

bool CameraSystem::hasCamera(const std::string& name) const {
    return m_cameras.find(name) != m_cameras.end();
}

Entity CameraSystem::createCameraEntity(const std::string& name) {
    // TODO: Create entity with transform component
    // For now, just return an invalid entity
    return Entity();
}

bool CameraSystem::setCameraToFollowEntity(const std::string& cameraName, Entity entity, float offsetX, float offsetY) {
    // Get camera
    auto camera = getCamera(cameraName);
    if (!camera) {
        std::cerr << "Camera '" << cameraName << "' does not exist" << std::endl;
        return false;
    }
    
    // Set camera to follow entity
    camera->followEntity(entity, offsetX, offsetY);
    
    std::cout << "Camera '" << cameraName << "' set to follow entity " << entity.getID() << std::endl;
    return true;
}

bool CameraSystem::stopCameraFromFollowing(const std::string& cameraName) {
    // Get camera
    auto camera = getCamera(cameraName);
    if (!camera) {
        std::cerr << "Camera '" << cameraName << "' does not exist" << std::endl;
        return false;
    }
    
    // Stop camera from following
    camera->stopFollowing();
    
    std::cout << "Camera '" << cameraName << "' stopped following entity" << std::endl;
    return true;
}

} // namespace Graphics
} // namespace RPGEngine