#pragma once

#include "Scene.h"
#include "../audio/MusicManager.h"
#include "../audio/SoundEffectManager.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace RPGEngine {
namespace Scene {

/**
 * Menu item structure
 */
struct MenuItem {
    std::string id;
    std::string text;
    std::string description;
    bool enabled;
    bool visible;
    std::function<void()> action;
    
    MenuItem(const std::string& itemId, const std::string& itemText, 
             std::function<void()> itemAction = nullptr)
        : id(itemId), text(itemText), enabled(true), visible(true), action(itemAction) {}
};

/**
 * Menu scene
 * Base class for menu-based scenes with navigation and UI
 */
class MenuScene : public Scene {
public:
    /**
     * Constructor
     * @param sceneId Scene identifier
     * @param entityManager Entity manager
     * @param componentManager Component manager
     * @param systemManager System manager
     * @param resourceManager Resource manager
     */
    MenuScene(const std::string& sceneId,
              std::shared_ptr<EntityManager> entityManager,
              std::shared_ptr<ComponentManager> componentManager,
              std::shared_ptr<SystemManager> systemManager,
              std::shared_ptr<Resources::ResourceManager> resourceManager);
    
    /**
     * Destructor
     */
    virtual ~MenuScene();
    
    /**
     * Set music manager
     * @param musicManager Music manager
     */
    void setMusicManager(std::shared_ptr<Audio::MusicManager> musicManager) {
        m_musicManager = musicManager;
    }
    
    /**
     * Set sound effect manager
     * @param soundManager Sound effect manager
     */
    void setSoundEffectManager(std::shared_ptr<Audio::SoundEffectManager> soundManager) {
        m_soundManager = soundManager;
    }
    
    /**
     * Add menu item
     * @param item Menu item to add
     */
    void addMenuItem(const MenuItem& item);
    
    /**
     * Remove menu item
     * @param itemId Item ID to remove
     * @return true if item was removed
     */
    bool removeMenuItem(const std::string& itemId);
    
    /**
     * Get menu item
     * @param itemId Item ID
     * @return Menu item, or nullptr if not found
     */
    MenuItem* getMenuItem(const std::string& itemId);
    
    /**
     * Set menu item enabled state
     * @param itemId Item ID
     * @param enabled Whether the item is enabled
     */
    void setMenuItemEnabled(const std::string& itemId, bool enabled);
    
    /**
     * Set menu item visible state
     * @param itemId Item ID
     * @param visible Whether the item is visible
     */
    void setMenuItemVisible(const std::string& itemId, bool visible);
    
    /**
     * Get current selected item index
     * @return Selected item index
     */
    int getSelectedItemIndex() const { return m_selectedItemIndex; }
    
    /**
     * Set selected item index
     * @param index Item index to select
     */
    void setSelectedItemIndex(int index);
    
    /**
     * Move selection up
     */
    void moveSelectionUp();
    
    /**
     * Move selection down
     */
    void moveSelectionDown();
    
    /**
     * Activate current selected item
     */
    void activateSelectedItem();
    
    /**
     * Get menu items
     * @return Vector of menu items
     */
    const std::vector<MenuItem>& getMenuItems() const { return m_menuItems; }
    
    /**
     * Clear all menu items
     */
    void clearMenuItems();
    
    /**
     * Set background music
     * @param musicId Music resource ID
     * @param fadeIn Whether to fade in the music
     */
    void setBackgroundMusic(const std::string& musicId, bool fadeIn = true);
    
    /**
     * Play UI sound effect
     * @param soundId Sound resource ID
     * @param volume Volume (0.0 to 1.0)
     */
    void playUISound(const std::string& soundId, float volume = 1.0f);
    
    /**
     * Set menu title
     * @param title Menu title
     */
    void setMenuTitle(const std::string& title) { m_menuTitle = title; }
    
    /**
     * Get menu title
     * @return Menu title
     */
    const std::string& getMenuTitle() const { return m_menuTitle; }
    
    /**
     * Set menu subtitle
     * @param subtitle Menu subtitle
     */
    void setMenuSubtitle(const std::string& subtitle) { m_menuSubtitle = subtitle; }
    
    /**
     * Get menu subtitle
     * @return Menu subtitle
     */
    const std::string& getMenuSubtitle() const { return m_menuSubtitle; }
    
    /**
     * Set whether menu wraps around
     * @param wrap Whether selection wraps around
     */
    void setWrapAround(bool wrap) { m_wrapAround = wrap; }
    
    /**
     * Check if menu wraps around
     * @return true if menu wraps around
     */
    bool getWrapAround() const { return m_wrapAround; }
    
protected:
    // Scene lifecycle overrides
    bool onLoad() override;
    void onUnload() override;
    void onActivate() override;
    void onDeactivate() override;
    void onPause() override;
    void onResume() override;
    void onUpdate(float deltaTime) override;
    void onRender(float deltaTime) override;
    void onHandleInput(const std::string& event) override;
    void onSaveState(SceneData& data) override;
    bool onLoadState(const SceneData& data) override;
    
    /**
     * Called when menu selection changes
     * @param oldIndex Previous selected index
     * @param newIndex New selected index
     */
    virtual void onSelectionChanged(int oldIndex, int newIndex);
    
    /**
     * Called when menu item is activated
     * @param item Activated menu item
     */
    virtual void onItemActivated(const MenuItem& item);
    
    /**
     * Initialize menu items
     * Override this method to set up menu-specific items
     */
    virtual void initializeMenuItems() {}
    
    /**
     * Update menu logic
     * @param deltaTime Time since last update
     */
    virtual void updateMenu(float deltaTime) {}
    
    /**
     * Render menu
     * @param deltaTime Time since last render
     */
    virtual void renderMenu(float deltaTime) {}
    
private:
    /**
     * Find next selectable item index
     * @param startIndex Starting index
     * @param direction Direction to search (1 for down, -1 for up)
     * @return Next selectable index, or -1 if none found
     */
    int findNextSelectableItem(int startIndex, int direction) const;
    
    // Managers
    std::shared_ptr<Audio::MusicManager> m_musicManager;
    std::shared_ptr<Audio::SoundEffectManager> m_soundManager;
    
    // Menu state
    std::vector<MenuItem> m_menuItems;
    int m_selectedItemIndex;
    std::string m_menuTitle;
    std::string m_menuSubtitle;
    bool m_wrapAround;
    
    // Menu settings
    std::string m_selectionSound;
    std::string m_activationSound;
    std::string m_cancelSound;
};

} // namespace Scene
} // namespace RPGEngine