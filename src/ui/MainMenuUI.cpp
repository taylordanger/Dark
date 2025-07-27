#include "MainMenuUI.h"

namespace RPGEngine {
namespace UI {

MainMenuUI::MainMenuUI(std::shared_ptr<UIRenderer> uiRenderer)
    : m_uiRenderer(uiRenderer)
    , m_state(MainMenuState::MainMenu)
    , m_visible(false)
    , m_gameTitle("RPG Game")
    , m_gameVersion("v1.0.0")
    , m_screenWidth(800.0f)
    , m_screenHeight(600.0f)
    , m_titleFontSize(48.0f)
    , m_buttonFontSize(18.0f)
    , m_versionFontSize(12.0f)
    , m_buttonSpacing(60.0f)
{
}

MainMenuUI::~MainMenuUI() {
}

bool MainMenuUI::onInitialize() {
    return true;
}

void MainMenuUI::onUpdate(float deltaTime) {
    if (!m_visible) return;
    
    m_uiRenderer->beginFrame();
    
    switch (m_state) {
        case MainMenuState::MainMenu:
            renderMainMenu();
            handleMainMenuInput();
            break;
        case MainMenuState::NewGame:
            renderNewGameMenu();
            handleNewGameInput();
            break;
        case MainMenuState::LoadGame:
            renderLoadGameMenu();
            handleLoadGameInput();
            break;
        case MainMenuState::Settings:
            renderSettingsMenu();
            handleSettingsInput();
            break;
        case MainMenuState::Credits:
            renderCreditsMenu();
            handleCreditsInput();
            break;
        case MainMenuState::Quit:
            // Handle quit confirmation
            break;
    }
    
    m_uiRenderer->endFrame();
}

void MainMenuUI::onShutdown() {
}

void MainMenuUI::show() {
    m_visible = true;
    m_state = MainMenuState::MainMenu;
}

void MainMenuUI::hide() {
    m_visible = false;
}

void MainMenuUI::renderMainMenu() {
    auto center = getScreenCenter();
    
    // Draw background panel
    UIRect backgroundPanel(0, 0, m_screenWidth, m_screenHeight);
    m_uiRenderer->drawPanel(backgroundPanel, "main_menu_background");
    
    // Draw game title
    UIRect titleBounds(0, center.second - 200, m_screenWidth, 60);
    m_uiRenderer->drawText(titleBounds, m_gameTitle, UIAlignment::Center, m_titleFontSize, "game_title");
    
    // Draw menu buttons
    float startY = center.second - 100;
    
    if (m_uiRenderer->drawButton(createCenteredButton("New Game", startY), "New Game", "new_game_btn")) {
        m_state = MainMenuState::NewGame;
        if (m_callbacks.onNewGame) m_callbacks.onNewGame();
    }
    
    if (m_uiRenderer->drawButton(createCenteredButton("Load Game", startY + m_buttonSpacing), "Load Game", "load_game_btn")) {
        m_state = MainMenuState::LoadGame;
        if (m_callbacks.onLoadGame) m_callbacks.onLoadGame();
    }
    
    if (m_uiRenderer->drawButton(createCenteredButton("Settings", startY + m_buttonSpacing * 2), "Settings", "settings_btn")) {
        m_state = MainMenuState::Settings;
        if (m_callbacks.onSettings) m_callbacks.onSettings();
    }
    
    if (m_uiRenderer->drawButton(createCenteredButton("Credits", startY + m_buttonSpacing * 3), "Credits", "credits_btn")) {
        m_state = MainMenuState::Credits;
        if (m_callbacks.onCredits) m_callbacks.onCredits();
    }
    
    if (m_uiRenderer->drawButton(createCenteredButton("Quit", startY + m_buttonSpacing * 4), "Quit", "quit_btn")) {
        if (m_callbacks.onQuit) m_callbacks.onQuit();
    }
    
    // Draw version info
    UIRect versionBounds(m_screenWidth - 150, m_screenHeight - 30, 140, 20);
    m_uiRenderer->drawText(versionBounds, m_gameVersion, UIAlignment::CenterRight, m_versionFontSize, "version_text");
}

void MainMenuUI::renderNewGameMenu() {
    auto center = getScreenCenter();
    
    // Draw background panel
    UIRect backgroundPanel(100, 100, m_screenWidth - 200, m_screenHeight - 200);
    m_uiRenderer->drawPanel(backgroundPanel, "new_game_panel");
    
    // Draw title
    UIRect titleBounds(100, 120, m_screenWidth - 200, 40);
    m_uiRenderer->drawText(titleBounds, "New Game", UIAlignment::Center, 24.0f, "new_game_title");
    
    // Draw difficulty selection (placeholder)
    float startY = center.second - 50;
    
    if (m_uiRenderer->drawButton(createCenteredButton("Easy", startY, 150, 35), "Easy", "easy_btn")) {
        // Start easy game
        if (m_callbacks.onBack) m_callbacks.onBack();
    }
    
    if (m_uiRenderer->drawButton(createCenteredButton("Normal", startY + 50), "Normal", "normal_btn")) {
        // Start normal game
        if (m_callbacks.onBack) m_callbacks.onBack();
    }
    
    if (m_uiRenderer->drawButton(createCenteredButton("Hard", startY + 100), "Hard", "hard_btn")) {
        // Start hard game
        if (m_callbacks.onBack) m_callbacks.onBack();
    }
    
    // Back button
    if (m_uiRenderer->drawButton(createCenteredButton("Back", startY + 180), "Back", "back_btn")) {
        m_state = MainMenuState::MainMenu;
        if (m_callbacks.onBack) m_callbacks.onBack();
    }
}

void MainMenuUI::renderLoadGameMenu() {
    auto center = getScreenCenter();
    
    // Draw background panel
    UIRect backgroundPanel(100, 100, m_screenWidth - 200, m_screenHeight - 200);
    m_uiRenderer->drawPanel(backgroundPanel, "load_game_panel");
    
    // Draw title
    UIRect titleBounds(100, 120, m_screenWidth - 200, 40);
    m_uiRenderer->drawText(titleBounds, "Load Game", UIAlignment::Center, 24.0f, "load_game_title");
    
    // Draw save slots (placeholder)
    float startY = center.second - 100;
    
    for (int i = 0; i < 5; ++i) {
        std::string slotText = "Save Slot " + std::to_string(i + 1);
        std::string slotId = "slot_" + std::to_string(i);
        
        if (m_uiRenderer->drawButton(createCenteredButton(slotText, startY + i * 45, 300, 35), slotText, slotId)) {
            // Load this save slot
            if (m_callbacks.onBack) m_callbacks.onBack();
        }
    }
    
    // Back button
    if (m_uiRenderer->drawButton(createCenteredButton("Back", startY + 250), "Back", "back_btn")) {
        m_state = MainMenuState::MainMenu;
        if (m_callbacks.onBack) m_callbacks.onBack();
    }
}

void MainMenuUI::renderSettingsMenu() {
    auto center = getScreenCenter();
    
    // Draw background panel
    UIRect backgroundPanel(100, 100, m_screenWidth - 200, m_screenHeight - 200);
    m_uiRenderer->drawPanel(backgroundPanel, "settings_panel");
    
    // Draw title
    UIRect titleBounds(100, 120, m_screenWidth - 200, 40);
    m_uiRenderer->drawText(titleBounds, "Settings", UIAlignment::Center, 24.0f, "settings_title");
    
    // Draw settings options
    float startY = center.second - 100;
    float leftColumn = center.first - 150;
    float rightColumn = center.first + 50;
    
    // Master Volume
    UIRect volumeLabelBounds(leftColumn, startY, 100, 30);
    m_uiRenderer->drawText(volumeLabelBounds, "Master Volume:", UIAlignment::CenterLeft, 16.0f, "volume_label");
    
    UIRect volumeSliderBounds(rightColumn, startY + 5, 200, 20);
    static float masterVolume = 0.8f;
    masterVolume = m_uiRenderer->drawSlider(volumeSliderBounds, masterVolume, 0.0f, 1.0f, "master_volume_slider");
    
    // Music Volume
    UIRect musicLabelBounds(leftColumn, startY + 50, 100, 30);
    m_uiRenderer->drawText(musicLabelBounds, "Music Volume:", UIAlignment::CenterLeft, 16.0f, "music_label");
    
    UIRect musicSliderBounds(rightColumn, startY + 55, 200, 20);
    static float musicVolume = 0.7f;
    musicVolume = m_uiRenderer->drawSlider(musicSliderBounds, musicVolume, 0.0f, 1.0f, "music_volume_slider");
    
    // SFX Volume
    UIRect sfxLabelBounds(leftColumn, startY + 100, 100, 30);
    m_uiRenderer->drawText(sfxLabelBounds, "SFX Volume:", UIAlignment::CenterLeft, 16.0f, "sfx_label");
    
    UIRect sfxSliderBounds(rightColumn, startY + 105, 200, 20);
    static float sfxVolume = 0.9f;
    sfxVolume = m_uiRenderer->drawSlider(sfxSliderBounds, sfxVolume, 0.0f, 1.0f, "sfx_volume_slider");
    
    // Fullscreen toggle
    UIRect fullscreenLabelBounds(leftColumn, startY + 150, 100, 30);
    m_uiRenderer->drawText(fullscreenLabelBounds, "Fullscreen:", UIAlignment::CenterLeft, 16.0f, "fullscreen_label");
    
    UIRect fullscreenCheckboxBounds(rightColumn, startY + 150, 150, 30);
    static bool fullscreen = false;
    fullscreen = m_uiRenderer->drawCheckbox(fullscreenCheckboxBounds, "", fullscreen, "fullscreen_checkbox");
    
    // Back button
    if (m_uiRenderer->drawButton(createCenteredButton("Back", startY + 220), "Back", "back_btn")) {
        m_state = MainMenuState::MainMenu;
        if (m_callbacks.onBack) m_callbacks.onBack();
    }
}

void MainMenuUI::renderCreditsMenu() {
    auto center = getScreenCenter();
    
    // Draw background panel
    UIRect backgroundPanel(100, 100, m_screenWidth - 200, m_screenHeight - 200);
    m_uiRenderer->drawPanel(backgroundPanel, "credits_panel");
    
    // Draw title
    UIRect titleBounds(100, 120, m_screenWidth - 200, 40);
    m_uiRenderer->drawText(titleBounds, "Credits", UIAlignment::Center, 24.0f, "credits_title");
    
    // Draw credits text
    float startY = center.second - 80;
    
    UIRect developerBounds(120, startY, m_screenWidth - 240, 25);
    m_uiRenderer->drawText(developerBounds, "Developed by: RPG Engine Team", UIAlignment::Center, 16.0f, "developer_text");
    
    UIRect engineBounds(120, startY + 30, m_screenWidth - 240, 25);
    m_uiRenderer->drawText(engineBounds, "Engine: RPG Engine Framework", UIAlignment::Center, 16.0f, "engine_text");
    
    UIRect versionBounds(120, startY + 60, m_screenWidth - 240, 25);
    m_uiRenderer->drawText(versionBounds, "Version: " + m_gameVersion, UIAlignment::Center, 16.0f, "version_text");
    
    UIRect thanksBounds(120, startY + 100, m_screenWidth - 240, 25);
    m_uiRenderer->drawText(thanksBounds, "Thanks for playing!", UIAlignment::Center, 16.0f, "thanks_text");
    
    // Back button
    if (m_uiRenderer->drawButton(createCenteredButton("Back", startY + 160), "Back", "back_btn")) {
        m_state = MainMenuState::MainMenu;
        if (m_callbacks.onBack) m_callbacks.onBack();
    }
}

void MainMenuUI::handleMainMenuInput() {
    // Input handling is done through button clicks in the render functions
}

void MainMenuUI::handleNewGameInput() {
    // Input handling is done through button clicks in the render functions
}

void MainMenuUI::handleLoadGameInput() {
    // Input handling is done through button clicks in the render functions
}

void MainMenuUI::handleSettingsInput() {
    // Input handling is done through button clicks in the render functions
}

void MainMenuUI::handleCreditsInput() {
    // Input handling is done through button clicks in the render functions
}

std::pair<float, float> MainMenuUI::getScreenCenter() const {
    return std::make_pair(m_screenWidth * 0.5f, m_screenHeight * 0.5f);
}

UIRect MainMenuUI::createCenteredButton(const std::string& text, float y, float width, float height) {
    float x = (m_screenWidth - width) * 0.5f;
    return UIRect(x, y, width, height);
}

} // namespace UI
} // namespace RPGEngine