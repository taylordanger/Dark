#pragma once

#include "../graphics/SpriteRenderer.h"
#include "../input/InputManager.h"
#include "../components/DialogueComponent.h"
#include "../systems/System.h"
#include "../core/Types.h"
#include "../graphics/Sprite.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace RPGEngine {
namespace UI {

/**
 * Dialogue UI style configuration
 */
struct DialogueUIStyle {
    // Window styling
    float windowX = 50.0f;
    float windowY = 400.0f;
    float windowWidth = 700.0f;
    float windowHeight = 200.0f;
    Graphics::Color windowBackgroundColor = Graphics::Color(0.0f, 0.0f, 0.0f, 0.8f);
    Graphics::Color windowBorderColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    float windowBorderWidth = 2.0f;
    float windowPadding = 20.0f;
    
    // Text styling
    Graphics::Color textColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    Graphics::Color speakerColor = Graphics::Color(1.0f, 1.0f, 0.0f, 1.0f);
    float textSize = 16.0f;
    float lineSpacing = 20.0f;
    float speakerTextSize = 18.0f;
    
    // Choice styling
    float choiceStartY = 50.0f;
    float choiceSpacing = 30.0f;
    Graphics::Color choiceNormalColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    Graphics::Color choiceSelectedColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    Graphics::Color choiceBackgroundColor = Graphics::Color(0.2f, 0.2f, 0.2f, 0.6f);
    Graphics::Color choiceSelectedBackgroundColor = Graphics::Color(0.4f, 0.4f, 0.4f, 0.8f);
    float choicePadding = 10.0f;
    
    // History styling
    float historyWindowX = 50.0f;
    float historyWindowY = 50.0f;
    float historyWindowWidth = 400.0f;
    float historyWindowHeight = 300.0f;
    Graphics::Color historyBackgroundColor = Graphics::Color(0.0f, 0.0f, 0.0f, 0.9f);
    Graphics::Color historyTextColor = Graphics::Color(0.9f, 0.9f, 0.9f, 1.0f);
    float historyTextSize = 14.0f;
    float historyLineSpacing = 18.0f;
    
    // Animation settings
    float typewriterSpeed = 50.0f; // Characters per second
    bool enableTypewriter = true;
    float fadeInDuration = 0.3f;
    float fadeOutDuration = 0.2f;
};

/**
 * Dialogue UI state enumeration
 */
enum class DialogueUIState {
    Hidden,
    FadingIn,
    Showing,
    FadingOut,
    ShowingHistory
};

/**
 * Dialogue UI system
 * Handles rendering and interaction for dialogue
 */
class DialogueUI : public System {
public:
    /**
     * Constructor
     * @param spriteRenderer Sprite renderer for drawing
     * @param inputManager Input manager for handling input
     */
    DialogueUI(std::shared_ptr<Graphics::SpriteRenderer> spriteRenderer,
               std::shared_ptr<Input::InputManager> inputManager);
    
    /**
     * Destructor
     */
    ~DialogueUI();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Set the dialogue component to display
     * @param dialogueComponent Dialogue component
     */
    void setDialogueComponent(std::shared_ptr<Components::DialogueComponent> dialogueComponent);
    
    /**
     * Show the dialogue UI
     */
    void show();
    
    /**
     * Hide the dialogue UI
     */
    void hide();
    
    /**
     * Toggle dialogue history display
     */
    void toggleHistory();
    
    /**
     * Check if dialogue UI is visible
     * @return true if visible
     */
    bool isVisible() const;
    
    /**
     * Check if dialogue UI is showing history
     * @return true if showing history
     */
    bool isShowingHistory() const;
    
    /**
     * Set UI style
     * @param style New style configuration
     */
    void setStyle(const DialogueUIStyle& style);
    
    /**
     * Get UI style
     * @return Current style configuration
     */
    const DialogueUIStyle& getStyle() const { return m_style; }
    
    /**
     * Set typewriter effect enabled
     * @param enabled Whether to enable typewriter effect
     */
    void setTypewriterEnabled(bool enabled);
    
    /**
     * Skip current typewriter animation
     */
    void skipTypewriter();
    
    /**
     * Set dialogue advance callback
     * @param callback Function called when dialogue should advance
     */
    void setAdvanceCallback(std::function<void(const std::string&)> callback);
    
    /**
     * Set dialogue close callback
     * @param callback Function called when dialogue should close
     */
    void setCloseCallback(std::function<void()> callback);
    
private:
    /**
     * Update input handling
     * @param deltaTime Time elapsed since last update
     */
    void updateInput(float deltaTime);
    
    /**
     * Update animations
     * @param deltaTime Time elapsed since last update
     */
    void updateAnimations(float deltaTime);
    
    /**
     * Render the dialogue UI
     */
    void render();
    
    /**
     * Render dialogue window
     */
    void renderDialogueWindow();
    
    /**
     * Render dialogue text
     */
    void renderDialogueText();
    
    /**
     * Render dialogue choices
     */
    void renderDialogueChoices();
    
    /**
     * Render dialogue history
     */
    void renderDialogueHistory();
    
    /**
     * Render a text string
     * @param text Text to render
     * @param x X position
     * @param y Y position
     * @param color Text color
     * @param size Text size
     * @return Height of rendered text
     */
    float renderText(const std::string& text, float x, float y, const Graphics::Color& color, float size);
    
    /**
     * Render a rectangle
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @param color Color
     * @param filled Whether to fill the rectangle
     */
    void renderRectangle(float x, float y, float width, float height, const Graphics::Color& color, bool filled = true);
    
    /**
     * Get text width (approximation)
     * @param text Text to measure
     * @param size Text size
     * @return Approximate text width
     */
    float getTextWidth(const std::string& text, float size);
    
    /**
     * Get text height
     * @param size Text size
     * @return Text height
     */
    float getTextHeight(float size);
    
    /**
     * Wrap text to fit within specified width
     * @param text Text to wrap
     * @param maxWidth Maximum width
     * @param textSize Text size
     * @return Vector of wrapped lines
     */
    std::vector<std::string> wrapText(const std::string& text, float maxWidth, float textSize);
    
    /**
     * Update choice selection based on input
     */
    void updateChoiceSelection();
    
    /**
     * Handle dialogue advance input
     */
    void handleAdvanceInput();
    
    /**
     * Handle choice selection input
     */
    void handleChoiceInput();
    
    /**
     * Handle history toggle input
     */
    void handleHistoryInput();
    
    /**
     * Update typewriter effect
     * @param deltaTime Time elapsed since last update
     */
    void updateTypewriter(float deltaTime);
    
    /**
     * Start typewriter animation for current text
     */
    void startTypewriter();
    
    /**
     * Check if typewriter animation is complete
     * @return true if complete
     */
    bool isTypewriterComplete() const;
    
    /**
     * Get current visible text (for typewriter effect)
     * @return Visible portion of text
     */
    std::string getVisibleText() const;
    
    // Dependencies
    std::shared_ptr<Graphics::SpriteRenderer> m_spriteRenderer;
    std::shared_ptr<Input::InputManager> m_inputManager;
    std::shared_ptr<Components::DialogueComponent> m_dialogueComponent;
    
    // UI state
    DialogueUIState m_state;
    DialogueUIStyle m_style;
    float m_fadeAlpha;
    float m_fadeTimer;
    
    // Choice selection
    int m_selectedChoiceIndex;
    std::vector<Components::DialogueChoice> m_currentChoices;
    
    // Typewriter effect
    std::string m_currentText;
    std::string m_currentSpeaker;
    float m_typewriterTimer;
    int m_visibleCharacters;
    bool m_typewriterComplete;
    
    // History display
    std::vector<std::string> m_historyLines;
    int m_historyScrollOffset;
    
    // Input timing
    float m_inputCooldown;
    static const float INPUT_COOLDOWN_TIME;
    
    // Callbacks
    std::function<void(const std::string&)> m_advanceCallback;
    std::function<void()> m_closeCallback;
    
    // Input action names
    static const std::string ACTION_ADVANCE;
    static const std::string ACTION_CHOICE_UP;
    static const std::string ACTION_CHOICE_DOWN;
    static const std::string ACTION_SELECT;
    static const std::string ACTION_HISTORY;
    static const std::string ACTION_SKIP;
    static const std::string ACTION_CLOSE;
};

} // namespace UI
} // namespace RPGEngine