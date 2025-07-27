#pragma once

#include "UIRenderer.h"
#include "../systems/System.h"
#include "../core/Types.h"
#include <memory>
#include <string>
#include <functional>

namespace RPGEngine {
namespace UI {

/**
 * Main menu UI state
 */
enum class MainMenuState {
    MainMenu,
    NewGame,
    LoadGame,
    Settings,
    Credits,
    Quit
};

/**
 * Main menu callbacks
 */
struct MainMenuCallbacks {
    std::function<void()> onNewGame;
    std::function<void()> onLoadGame;
    std::function<void()> onSettings;
    std::function<void()> onCredits;
    std::function<void()> onQuit;
    std::function<void()> onBack;
};

/**
 * Main Menu UI System
 * Handles the main menu interface
 */
class MainMenuUI : public System {
public:
    /**
     * Constructor
     * @param uiRenderer UI renderer for drawing
     */
    MainMenuUI(std::shared_ptr<UIRenderer> uiRenderer);
    
    /**
     * Destructor
     */
    ~MainMenuUI();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Show main menu
     */
    void show();
    
    /**
     * Hide main menu
     */
    void hide();
    
    /**
     * Check if main menu is visible
     * @return true if visible
     */
    bool isVisible() const { return m_visible; }
    
    /**
     * Get current menu state
     * @return Current menu state
     */
    MainMenuState getState() const { return m_state; }
    
    /**
     * Set menu callbacks
     * @param callbacks Menu callbacks
     */
    void setCallbacks(const MainMenuCallbacks& callbacks) { m_callbacks = callbacks; }
    
    /**
     * Set game title
     * @param title Game title
     */
    void setGameTitle(const std::string& title) { m_gameTitle = title; }
    
    /**
     * Set game version
     * @param version Game version
     */
    void setGameVersion(const std::string& version) { m_gameVersion = version; }
    
private:
    /**
     * Render main menu
     */
    void renderMainMenu();
    
    /**
     * Render new game menu
     */
    void renderNewGameMenu();
    
    /**
     * Render load game menu
     */
    void renderLoadGameMenu();
    
    /**
     * Render settings menu
     */
    void renderSettingsMenu();
    
    /**
     * Render credits menu
     */
    void renderCreditsMenu();
    
    /**
     * Handle main menu input
     */
    void handleMainMenuInput();
    
    /**
     * Handle new game input
     */
    void handleNewGameInput();
    
    /**
     * Handle load game input
     */
    void handleLoadGameInput();
    
    /**
     * Handle settings input
     */
    void handleSettingsInput();
    
    /**
     * Handle credits input
     */
    void handleCreditsInput();
    
    /**
     * Get screen center position
     * @return Screen center position
     */
    std::pair<float, float> getScreenCenter() const;
    
    /**
     * Create centered button
     * @param text Button text
     * @param y Y position
     * @param width Button width
     * @param height Button height
     * @return Button bounds
     */
    UIRect createCenteredButton(const std::string& text, float y, float width = 200.0f, float height = 40.0f);
    
    // Dependencies
    std::shared_ptr<UIRenderer> m_uiRenderer;
    
    // UI state
    MainMenuState m_state;
    bool m_visible;
    MainMenuCallbacks m_callbacks;
    
    // Menu content
    std::string m_gameTitle;
    std::string m_gameVersion;
    
    // Screen dimensions (would normally come from graphics system)
    float m_screenWidth;
    float m_screenHeight;
    
    // Menu styling
    float m_titleFontSize;
    float m_buttonFontSize;
    float m_versionFontSize;
    float m_buttonSpacing;
};

} // namespace UI
} // namespace RPGEngine