#include "MenuScene.h"
#include <iostream>
#include <algorithm>

namespace RPGEngine {
namespace Scene {

MenuScene::MenuScene(const std::string& sceneId,
                    std::shared_ptr<EntityManager> entityManager,
                    std::shared_ptr<ComponentManager> componentManager,
                    std::shared_ptr<SystemManager> systemManager,
                    std::shared_ptr<Resources::ResourceManager> resourceManager)
    : Scene(sceneId, entityManager, componentManager, systemManager, resourceManager)
    , m_selectedItemIndex(0)
    , m_wrapAround(true)
    , m_selectionSound("menu_select")
    , m_activationSound("menu_activate")
    , m_cancelSound("menu_cancel")
{
}

MenuScene::~MenuScene() {
    // Cleanup handled by base class
}

void MenuScene::addMenuItem(const MenuItem& item) {
    m_menuItems.push_back(item);
    
    // If this is the first item, select it
    if (m_menuItems.size() == 1) {
        m_selectedItemIndex = 0;
    }
    
    std::cout << "Added menu item: " << item.text << std::endl;
}

bool MenuScene::removeMenuItem(const std::string& itemId) {
    auto it = std::find_if(m_menuItems.begin(), m_menuItems.end(),
        [&itemId](const MenuItem& item) { return item.id == itemId; });
    
    if (it != m_menuItems.end()) {
        int removedIndex = static_cast<int>(std::distance(m_menuItems.begin(), it));
        m_menuItems.erase(it);
        
        // Adjust selected index if necessary
        if (m_selectedItemIndex >= static_cast<int>(m_menuItems.size())) {
            m_selectedItemIndex = std::max(0, static_cast<int>(m_menuItems.size()) - 1);
        } else if (m_selectedItemIndex > removedIndex) {
            m_selectedItemIndex--;
        }
        
        std::cout << "Removed menu item: " << itemId << std::endl;
        return true;
    }
    
    return false;
}

MenuItem* MenuScene::getMenuItem(const std::string& itemId) {
    auto it = std::find_if(m_menuItems.begin(), m_menuItems.end(),
        [&itemId](const MenuItem& item) { return item.id == itemId; });
    
    return (it != m_menuItems.end()) ? &(*it) : nullptr;
}

void MenuScene::setMenuItemEnabled(const std::string& itemId, bool enabled) {
    auto* item = getMenuItem(itemId);
    if (item) {
        item->enabled = enabled;
        
        // If current selected item is disabled, find next enabled item
        if (!enabled && m_selectedItemIndex < static_cast<int>(m_menuItems.size()) && 
            m_menuItems[m_selectedItemIndex].id == itemId) {
            int nextIndex = findNextSelectableItem(m_selectedItemIndex, 1);
            if (nextIndex == -1) {
                nextIndex = findNextSelectableItem(m_selectedItemIndex, -1);
            }
            if (nextIndex != -1) {
                setSelectedItemIndex(nextIndex);
            }
        }
    }
}

void MenuScene::setMenuItemVisible(const std::string& itemId, bool visible) {
    auto* item = getMenuItem(itemId);
    if (item) {
        item->visible = visible;
        
        // If current selected item is hidden, find next visible item
        if (!visible && m_selectedItemIndex < static_cast<int>(m_menuItems.size()) && 
            m_menuItems[m_selectedItemIndex].id == itemId) {
            int nextIndex = findNextSelectableItem(m_selectedItemIndex, 1);
            if (nextIndex == -1) {
                nextIndex = findNextSelectableItem(m_selectedItemIndex, -1);
            }
            if (nextIndex != -1) {
                setSelectedItemIndex(nextIndex);
            }
        }
    }
}

void MenuScene::setSelectedItemIndex(int index) {
    if (m_menuItems.empty()) {
        m_selectedItemIndex = 0;
        return;
    }
    
    int oldIndex = m_selectedItemIndex;
    m_selectedItemIndex = std::max(0, std::min(index, static_cast<int>(m_menuItems.size()) - 1));
    
    // Find next selectable item if current is not selectable
    if (m_selectedItemIndex < static_cast<int>(m_menuItems.size())) {
        const auto& item = m_menuItems[m_selectedItemIndex];
        if (!item.enabled || !item.visible) {
            int nextIndex = findNextSelectableItem(m_selectedItemIndex, 1);
            if (nextIndex == -1) {
                nextIndex = findNextSelectableItem(m_selectedItemIndex, -1);
            }
            if (nextIndex != -1) {
                m_selectedItemIndex = nextIndex;
            }
        }
    }
    
    if (oldIndex != m_selectedItemIndex) {
        onSelectionChanged(oldIndex, m_selectedItemIndex);
        playUISound(m_selectionSound, 0.7f);
    }
}

void MenuScene::moveSelectionUp() {
    if (m_menuItems.empty()) {
        return;
    }
    
    int newIndex = findNextSelectableItem(m_selectedItemIndex, -1);
    
    if (newIndex == -1 && m_wrapAround) {
        // Wrap to bottom
        newIndex = findNextSelectableItem(static_cast<int>(m_menuItems.size()), -1);
    }
    
    if (newIndex != -1) {
        setSelectedItemIndex(newIndex);
    }
}

void MenuScene::moveSelectionDown() {
    if (m_menuItems.empty()) {
        return;
    }
    
    int newIndex = findNextSelectableItem(m_selectedItemIndex, 1);
    
    if (newIndex == -1 && m_wrapAround) {
        // Wrap to top
        newIndex = findNextSelectableItem(-1, 1);
    }
    
    if (newIndex != -1) {
        setSelectedItemIndex(newIndex);
    }
}

void MenuScene::activateSelectedItem() {
    if (m_selectedItemIndex >= 0 && m_selectedItemIndex < static_cast<int>(m_menuItems.size())) {
        const auto& item = m_menuItems[m_selectedItemIndex];
        
        if (item.enabled && item.visible) {
            playUISound(m_activationSound, 0.8f);
            onItemActivated(item);
            
            if (item.action) {
                item.action();
            }
        }
    }
}

void MenuScene::clearMenuItems() {
    m_menuItems.clear();
    m_selectedItemIndex = 0;
    
    std::cout << "Cleared all menu items" << std::endl;
}

void MenuScene::setBackgroundMusic(const std::string& musicId, bool fadeIn) {
    if (!m_musicManager) {
        std::cerr << "Music manager not set for MenuScene" << std::endl;
        return;
    }
    
    if (fadeIn) {
        m_musicManager->playMusic(musicId, 0.6f, true, Audio::MusicTransitionType::FadeIn, 1.5f);
    } else {
        m_musicManager->playMusic(musicId, 0.6f, true, Audio::MusicTransitionType::Immediate);
    }
    
    std::cout << "Set menu background music: " << musicId << std::endl;
}

void MenuScene::playUISound(const std::string& soundId, float volume) {
    if (!m_soundManager) {
        return;
    }
    
    m_soundManager->play2DSound(soundId, volume, 1.0f, false, Audio::SoundCategory::UI);
}

bool MenuScene::onLoad() {
    std::cout << "MenuScene::onLoad() - " << getSceneId() << std::endl;
    
    // Set default properties
    setProperty("scene_type", "menu");
    setProperty("pausable", "false");
    setProperty("overlay", "false");
    
    // Initialize menu items
    initializeMenuItems();
    
    return true;
}

void MenuScene::onUnload() {
    std::cout << "MenuScene::onUnload() - " << getSceneId() << std::endl;
    
    // Stop background music
    if (m_musicManager) {
        m_musicManager->stopMusic(0.5f);
    }
    
    // Clear menu items
    clearMenuItems();
}

void MenuScene::onActivate() {
    std::cout << "MenuScene::onActivate() - " << getSceneId() << std::endl;
    
    // Resume background music if it was playing
    if (m_musicManager && m_musicManager->isPaused()) {
        m_musicManager->resumeMusic(0.3f);
    }
}

void MenuScene::onDeactivate() {
    std::cout << "MenuScene::onDeactivate() - " << getSceneId() << std::endl;
    
    // Pause background music
    if (m_musicManager && m_musicManager->isPlaying()) {
        m_musicManager->pauseMusic(0.3f);
    }
}

void MenuScene::onPause() {
    std::cout << "MenuScene::onPause() - " << getSceneId() << std::endl;
    
    // Pause audio
    if (m_musicManager) {
        m_musicManager->pauseMusic(0.2f);
    }
}

void MenuScene::onResume() {
    std::cout << "MenuScene::onResume() - " << getSceneId() << std::endl;
    
    // Resume audio
    if (m_musicManager) {
        m_musicManager->resumeMusic(0.2f);
    }
}

void MenuScene::onUpdate(float deltaTime) {
    // Update audio managers
    if (m_musicManager) {
        m_musicManager->update(deltaTime);
    }
    if (m_soundManager) {
        m_soundManager->update(deltaTime);
    }
    
    // Update menu logic
    updateMenu(deltaTime);
}

void MenuScene::onRender(float deltaTime) {
    // Render menu
    renderMenu(deltaTime);
    
    // TODO: Render menu items, selection highlight, etc.
    // This would require a UI rendering system
}

void MenuScene::onHandleInput(const std::string& event) {
    // Handle menu navigation
    if (event == "up" || event == "w") {
        moveSelectionUp();
    } else if (event == "down" || event == "s") {
        moveSelectionDown();
    } else if (event == "select" || event == "enter" || event == "space") {
        activateSelectedItem();
    } else if (event == "cancel" || event == "escape" || event == "backspace") {
        playUISound(m_cancelSound, 0.7f);
        
        // Default cancel behavior - go back to previous scene or main menu
        if (getSceneId() != "main_menu") {
            requestTransition("main_menu", SceneTransitionType::Fade);
        }
    }
}

void MenuScene::onSaveState(SceneData& data) {
    std::cout << "MenuScene::onSaveState() - " << getSceneId() << std::endl;
    
    // Save menu state
    data.properties["selected_item_index"] = std::to_string(m_selectedItemIndex);
    data.properties["menu_title"] = m_menuTitle;
    data.properties["menu_subtitle"] = m_menuSubtitle;
    data.properties["wrap_around"] = m_wrapAround ? "true" : "false";
    data.properties["menu_item_count"] = std::to_string(m_menuItems.size());
    
    // Save menu items
    for (size_t i = 0; i < m_menuItems.size(); ++i) {
        const auto& item = m_menuItems[i];
        std::string prefix = "item_" + std::to_string(i) + "_";
        
        data.properties[prefix + "id"] = item.id;
        data.properties[prefix + "text"] = item.text;
        data.properties[prefix + "description"] = item.description;
        data.properties[prefix + "enabled"] = item.enabled ? "true" : "false";
        data.properties[prefix + "visible"] = item.visible ? "true" : "false";
    }
}

bool MenuScene::onLoadState(const SceneData& data) {
    std::cout << "MenuScene::onLoadState() - " << getSceneId() << std::endl;
    
    // Load menu state
    auto selectedIt = data.properties.find("selected_item_index");
    if (selectedIt != data.properties.end()) {
        m_selectedItemIndex = std::stoi(selectedIt->second);
    }
    
    auto titleIt = data.properties.find("menu_title");
    if (titleIt != data.properties.end()) {
        m_menuTitle = titleIt->second;
    }
    
    auto subtitleIt = data.properties.find("menu_subtitle");
    if (subtitleIt != data.properties.end()) {
        m_menuSubtitle = subtitleIt->second;
    }
    
    auto wrapIt = data.properties.find("wrap_around");
    if (wrapIt != data.properties.end()) {
        m_wrapAround = (wrapIt->second == "true");
    }
    
    // Load menu items
    auto countIt = data.properties.find("menu_item_count");
    if (countIt != data.properties.end()) {
        int itemCount = std::stoi(countIt->second);
        
        clearMenuItems();
        
        for (int i = 0; i < itemCount; ++i) {
            std::string prefix = "item_" + std::to_string(i) + "_";
            
            MenuItem item("", "");
            
            auto idIt = data.properties.find(prefix + "id");
            if (idIt != data.properties.end()) {
                item.id = idIt->second;
            }
            
            auto textIt = data.properties.find(prefix + "text");
            if (textIt != data.properties.end()) {
                item.text = textIt->second;
            }
            
            auto descIt = data.properties.find(prefix + "description");
            if (descIt != data.properties.end()) {
                item.description = descIt->second;
            }
            
            auto enabledIt = data.properties.find(prefix + "enabled");
            if (enabledIt != data.properties.end()) {
                item.enabled = (enabledIt->second == "true");
            }
            
            auto visibleIt = data.properties.find(prefix + "visible");
            if (visibleIt != data.properties.end()) {
                item.visible = (visibleIt->second == "true");
            }
            
            addMenuItem(item);
        }
    }
    
    return true;
}

void MenuScene::onSelectionChanged(int oldIndex, int newIndex) {
    std::cout << "Menu selection changed: " << oldIndex << " -> " << newIndex << std::endl;
    
    // Show description of selected item
    if (newIndex >= 0 && newIndex < static_cast<int>(m_menuItems.size())) {
        const auto& item = m_menuItems[newIndex];
        if (!item.description.empty()) {
            std::cout << "Selected: " << item.text << " - " << item.description << std::endl;
        }
    }
}

void MenuScene::onItemActivated(const MenuItem& item) {
    std::cout << "Menu item activated: " << item.text << std::endl;
}

int MenuScene::findNextSelectableItem(int startIndex, int direction) const {
    if (m_menuItems.empty()) {
        return -1;
    }
    
    int index = startIndex + direction;
    int itemCount = static_cast<int>(m_menuItems.size());
    
    while (index >= 0 && index < itemCount) {
        const auto& item = m_menuItems[index];
        if (item.enabled && item.visible) {
            return index;
        }
        index += direction;
    }
    
    return -1;
}

} // namespace Scene
} // namespace RPGEngine