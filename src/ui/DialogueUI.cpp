#include "DialogueUI.h"
#include <algorithm>
#include <sstream>
#include <iostream>

namespace RPGEngine {
namespace UI {

// Static constants
const float DialogueUI::INPUT_COOLDOWN_TIME = 0.2f;
const std::string DialogueUI::ACTION_ADVANCE = "dialogue_advance";
const std::string DialogueUI::ACTION_CHOICE_UP = "dialogue_choice_up";
const std::string DialogueUI::ACTION_CHOICE_DOWN = "dialogue_choice_down";
const std::string DialogueUI::ACTION_SELECT = "dialogue_select";
const std::string DialogueUI::ACTION_HISTORY = "dialogue_history";
const std::string DialogueUI::ACTION_SKIP = "dialogue_skip";
const std::string DialogueUI::ACTION_CLOSE = "dialogue_close";

DialogueUI::DialogueUI(std::shared_ptr<Graphics::SpriteRenderer> spriteRenderer,
                       std::shared_ptr<Input::InputManager> inputManager)
    : System("DialogueUI")
    , m_spriteRenderer(spriteRenderer)
    , m_inputManager(inputManager)
    , m_dialogueComponent(nullptr)
    , m_state(DialogueUIState::Hidden)
    , m_fadeAlpha(0.0f)
    , m_fadeTimer(0.0f)
    , m_selectedChoiceIndex(0)
    , m_typewriterTimer(0.0f)
    , m_visibleCharacters(0)
    , m_typewriterComplete(true)
    , m_historyScrollOffset(0)
    , m_inputCooldown(0.0f)
{
}

DialogueUI::~DialogueUI() {
}

bool DialogueUI::onInitialize() {
    if (!m_inputManager) {
        std::cerr << "DialogueUI: InputManager is required" << std::endl;
        return false;
    }
    
    // Create input actions for dialogue UI
    m_inputManager->createAction(ACTION_ADVANCE);
    m_inputManager->createAction(ACTION_CHOICE_UP);
    m_inputManager->createAction(ACTION_CHOICE_DOWN);
    m_inputManager->createAction(ACTION_SELECT);
    m_inputManager->createAction(ACTION_HISTORY);
    m_inputManager->createAction(ACTION_SKIP);
    m_inputManager->createAction(ACTION_CLOSE);
    
    // Bind default keys
    m_inputManager->bindKeyToAction(ACTION_ADVANCE, Input::KeyCode::Space);
    m_inputManager->bindKeyToAction(ACTION_ADVANCE, Input::KeyCode::Enter);
    m_inputManager->bindKeyToAction(ACTION_CHOICE_UP, Input::KeyCode::Up);
    m_inputManager->bindKeyToAction(ACTION_CHOICE_DOWN, Input::KeyCode::Down);
    m_inputManager->bindKeyToAction(ACTION_SELECT, Input::KeyCode::Enter);
    m_inputManager->bindKeyToAction(ACTION_HISTORY, Input::KeyCode::H);
    m_inputManager->bindKeyToAction(ACTION_SKIP, Input::KeyCode::S);
    m_inputManager->bindKeyToAction(ACTION_CLOSE, Input::KeyCode::Escape);
    
    std::cout << "DialogueUI initialized successfully" << std::endl;
    return true;
}

void DialogueUI::onUpdate(float deltaTime) {
    if (m_state == DialogueUIState::Hidden) {
        return;
    }
    
    // Update input cooldown
    if (m_inputCooldown > 0.0f) {
        m_inputCooldown -= deltaTime;
    }
    
    updateAnimations(deltaTime);
    updateInput(deltaTime);
    render();
}

void DialogueUI::onShutdown() {
    // Remove input actions
    if (m_inputManager) {
        m_inputManager->removeAction(ACTION_ADVANCE);
        m_inputManager->removeAction(ACTION_CHOICE_UP);
        m_inputManager->removeAction(ACTION_CHOICE_DOWN);
        m_inputManager->removeAction(ACTION_SELECT);
        m_inputManager->removeAction(ACTION_HISTORY);
        m_inputManager->removeAction(ACTION_SKIP);
        m_inputManager->removeAction(ACTION_CLOSE);
    }
    
    std::cout << "DialogueUI shutdown" << std::endl;
}

void DialogueUI::setDialogueComponent(std::shared_ptr<Components::DialogueComponent> dialogueComponent) {
    m_dialogueComponent = dialogueComponent;
    
    if (m_dialogueComponent) {
        // Set up callbacks to update UI when dialogue changes
        m_dialogueComponent->setNodeChangedCallback([this](const std::string& nodeId) {
            const Components::DialogueNode* node = m_dialogueComponent->getCurrentNode();
            if (node) {
                m_currentText = node->text;
                m_currentSpeaker = node->speaker;
                m_currentChoices = m_dialogueComponent->getAvailableChoices();
                m_selectedChoiceIndex = 0;
                
                if (m_style.enableTypewriter && !m_currentText.empty()) {
                    startTypewriter();
                }
            }
        });
        
        m_dialogueComponent->setDialogueEndedCallback([this]() {
            hide();
        });
    }
}

void DialogueUI::show() {
    if (m_state == DialogueUIState::Hidden) {
        m_state = DialogueUIState::FadingIn;
        m_fadeTimer = 0.0f;
        m_fadeAlpha = 0.0f;
        
        // Initialize dialogue content
        if (m_dialogueComponent && m_dialogueComponent->isDialogueActive()) {
            const Components::DialogueNode* node = m_dialogueComponent->getCurrentNode();
            if (node) {
                m_currentText = node->text;
                m_currentSpeaker = node->speaker;
                m_currentChoices = m_dialogueComponent->getAvailableChoices();
                m_selectedChoiceIndex = 0;
                
                if (m_style.enableTypewriter && !m_currentText.empty()) {
                    startTypewriter();
                }
            }
        }
        
        std::cout << "DialogueUI showing" << std::endl;
    }
}

void DialogueUI::hide() {
    if (m_state != DialogueUIState::Hidden && m_state != DialogueUIState::FadingOut) {
        m_state = DialogueUIState::FadingOut;
        m_fadeTimer = 0.0f;
        
        std::cout << "DialogueUI hiding" << std::endl;
    }
}

void DialogueUI::toggleHistory() {
    if (m_state == DialogueUIState::ShowingHistory) {
        m_state = DialogueUIState::Showing;
    } else if (m_state == DialogueUIState::Showing) {
        m_state = DialogueUIState::ShowingHistory;
        m_historyScrollOffset = 0;
        
        // Update history lines
        if (m_dialogueComponent) {
            const auto& history = m_dialogueComponent->getDialogueHistory();
            m_historyLines.clear();
            
            for (const auto& entry : history) {
                // Wrap history text
                auto wrappedLines = wrapText(entry, m_style.historyWindowWidth - m_style.windowPadding * 2, m_style.historyTextSize);
                m_historyLines.insert(m_historyLines.end(), wrappedLines.begin(), wrappedLines.end());
            }
        }
    }
}

bool DialogueUI::isVisible() const {
    return m_state != DialogueUIState::Hidden;
}

bool DialogueUI::isShowingHistory() const {
    return m_state == DialogueUIState::ShowingHistory;
}

void DialogueUI::setStyle(const DialogueUIStyle& style) {
    m_style = style;
}

void DialogueUI::setTypewriterEnabled(bool enabled) {
    m_style.enableTypewriter = enabled;
    
    if (!enabled) {
        m_typewriterComplete = true;
        m_visibleCharacters = m_currentText.length();
    }
}

void DialogueUI::skipTypewriter() {
    if (!m_typewriterComplete) {
        m_typewriterComplete = true;
        m_visibleCharacters = m_currentText.length();
    }
}

void DialogueUI::setAdvanceCallback(std::function<void(const std::string&)> callback) {
    m_advanceCallback = callback;
}

void DialogueUI::setCloseCallback(std::function<void()> callback) {
    m_closeCallback = callback;
}

void DialogueUI::updateInput(float deltaTime) {
    if (!m_inputManager || m_inputCooldown > 0.0f) {
        return;
    }
    
    if (m_state == DialogueUIState::ShowingHistory) {
        handleHistoryInput();
    } else if (m_state == DialogueUIState::Showing) {
        if (!m_currentChoices.empty()) {
            handleChoiceInput();
        } else {
            handleAdvanceInput();
        }
        
        // Always check for history and skip
        handleHistoryInput();
        
        if (m_inputManager->isActionJustActivated(ACTION_SKIP)) {
            skipTypewriter();
            m_inputCooldown = INPUT_COOLDOWN_TIME;
        }
    }
    
    // Always check for close
    if (m_inputManager->isActionJustActivated(ACTION_CLOSE)) {
        if (m_closeCallback) {
            m_closeCallback();
        } else {
            hide();
        }
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
}

void DialogueUI::updateAnimations(float deltaTime) {
    // Update fade animation
    if (m_state == DialogueUIState::FadingIn) {
        m_fadeTimer += deltaTime;
        m_fadeAlpha = std::min(1.0f, m_fadeTimer / m_style.fadeInDuration);
        
        if (m_fadeAlpha >= 1.0f) {
            m_state = DialogueUIState::Showing;
        }
    } else if (m_state == DialogueUIState::FadingOut) {
        m_fadeTimer += deltaTime;
        m_fadeAlpha = std::max(0.0f, 1.0f - (m_fadeTimer / m_style.fadeOutDuration));
        
        if (m_fadeAlpha <= 0.0f) {
            m_state = DialogueUIState::Hidden;
        }
    } else if (m_state == DialogueUIState::Showing || m_state == DialogueUIState::ShowingHistory) {
        m_fadeAlpha = 1.0f;
    }
    
    // Update typewriter effect
    if (m_style.enableTypewriter && !m_typewriterComplete) {
        updateTypewriter(deltaTime);
    }
}

void DialogueUI::render() {
    if (!m_spriteRenderer || m_fadeAlpha <= 0.0f) {
        return;
    }
    
    if (m_state == DialogueUIState::ShowingHistory) {
        renderDialogueHistory();
    } else {
        renderDialogueWindow();
        renderDialogueText();
        
        if (!m_currentChoices.empty()) {
            renderDialogueChoices();
        }
    }
}

void DialogueUI::renderDialogueWindow() {
    // Apply fade alpha to colors
    Graphics::Color bgColor = m_style.windowBackgroundColor;
    Graphics::Color borderColor = m_style.windowBorderColor;
    bgColor.a *= m_fadeAlpha;
    borderColor.a *= m_fadeAlpha;
    
    // Render background
    renderRectangle(m_style.windowX, m_style.windowY, m_style.windowWidth, m_style.windowHeight, bgColor, true);
    
    // Render border
    if (m_style.windowBorderWidth > 0.0f) {
        renderRectangle(m_style.windowX, m_style.windowY, m_style.windowWidth, m_style.windowBorderWidth, borderColor, true);
        renderRectangle(m_style.windowX, m_style.windowY + m_style.windowHeight - m_style.windowBorderWidth, m_style.windowWidth, m_style.windowBorderWidth, borderColor, true);
        renderRectangle(m_style.windowX, m_style.windowY, m_style.windowBorderWidth, m_style.windowHeight, borderColor, true);
        renderRectangle(m_style.windowX + m_style.windowWidth - m_style.windowBorderWidth, m_style.windowY, m_style.windowBorderWidth, m_style.windowHeight, borderColor, true);
    }
}

void DialogueUI::renderDialogueText() {
    float textX = m_style.windowX + m_style.windowPadding;
    float textY = m_style.windowY + m_style.windowPadding;
    float textWidth = m_style.windowWidth - m_style.windowPadding * 2;
    
    // Apply fade alpha to text colors
    Graphics::Color speakerColor = m_style.speakerColor;
    Graphics::Color textColor = m_style.textColor;
    speakerColor.a *= m_fadeAlpha;
    textColor.a *= m_fadeAlpha;
    
    // Render speaker name
    if (!m_currentSpeaker.empty()) {
        renderText(m_currentSpeaker + ":", textX, textY, speakerColor, m_style.speakerTextSize);
        textY += m_style.speakerTextSize + m_style.lineSpacing;
    }
    
    // Render dialogue text with typewriter effect
    std::string displayText = getVisibleText();
    if (!displayText.empty()) {
        auto wrappedLines = wrapText(displayText, textWidth, m_style.textSize);
        
        for (const auto& line : wrappedLines) {
            renderText(line, textX, textY, textColor, m_style.textSize);
            textY += m_style.lineSpacing;
        }
    }
    
    // Show continue indicator if typewriter is complete and no choices
    if (m_typewriterComplete && m_currentChoices.empty()) {
        Graphics::Color indicatorColor = textColor;
        indicatorColor.a *= 0.7f;
        
        float indicatorX = m_style.windowX + m_style.windowWidth - m_style.windowPadding - 20.0f;
        float indicatorY = m_style.windowY + m_style.windowHeight - m_style.windowPadding - 10.0f;
        
        renderText("▼", indicatorX, indicatorY, indicatorColor, m_style.textSize);
    }
}

void DialogueUI::renderDialogueChoices() {
    if (m_currentChoices.empty()) {
        return;
    }
    
    float choiceX = m_style.windowX + m_style.windowPadding;
    float choiceY = m_style.windowY + m_style.windowHeight - m_style.choiceStartY;
    float choiceWidth = m_style.windowWidth - m_style.windowPadding * 2;
    
    for (size_t i = 0; i < m_currentChoices.size(); ++i) {
        const auto& choice = m_currentChoices[i];
        bool isSelected = (static_cast<int>(i) == m_selectedChoiceIndex);
        
        // Apply fade alpha to choice colors
        Graphics::Color bgColor = isSelected ? m_style.choiceSelectedBackgroundColor : m_style.choiceBackgroundColor;
        Graphics::Color textColor = isSelected ? m_style.choiceSelectedColor : m_style.choiceNormalColor;
        bgColor.a *= m_fadeAlpha;
        textColor.a *= m_fadeAlpha;
        
        float choiceHeight = m_style.textSize + m_style.choicePadding * 2;
        
        // Render choice background
        renderRectangle(choiceX, choiceY - choiceHeight, choiceWidth, choiceHeight, bgColor, true);
        
        // Render choice text
        std::string choiceText = "• " + choice.text;
        renderText(choiceText, choiceX + m_style.choicePadding, choiceY - choiceHeight + m_style.choicePadding, textColor, m_style.textSize);
        
        choiceY -= m_style.choiceSpacing;
    }
}

void DialogueUI::renderDialogueHistory() {
    // Apply fade alpha to colors
    Graphics::Color bgColor = m_style.historyBackgroundColor;
    Graphics::Color textColor = m_style.historyTextColor;
    bgColor.a *= m_fadeAlpha;
    textColor.a *= m_fadeAlpha;
    
    // Render background
    renderRectangle(m_style.historyWindowX, m_style.historyWindowY, m_style.historyWindowWidth, m_style.historyWindowHeight, bgColor, true);
    
    // Render border
    Graphics::Color borderColor = m_style.windowBorderColor;
    borderColor.a *= m_fadeAlpha;
    
    if (m_style.windowBorderWidth > 0.0f) {
        renderRectangle(m_style.historyWindowX, m_style.historyWindowY, m_style.historyWindowWidth, m_style.windowBorderWidth, borderColor, true);
        renderRectangle(m_style.historyWindowX, m_style.historyWindowY + m_style.historyWindowHeight - m_style.windowBorderWidth, m_style.historyWindowWidth, m_style.windowBorderWidth, borderColor, true);
        renderRectangle(m_style.historyWindowX, m_style.historyWindowY, m_style.windowBorderWidth, m_style.historyWindowHeight, borderColor, true);
        renderRectangle(m_style.historyWindowX + m_style.historyWindowWidth - m_style.windowBorderWidth, m_style.historyWindowY, m_style.windowBorderWidth, m_style.historyWindowHeight, borderColor, true);
    }
    
    // Render history text
    float textX = m_style.historyWindowX + m_style.windowPadding;
    float textY = m_style.historyWindowY + m_style.windowPadding;
    float maxHeight = m_style.historyWindowHeight - m_style.windowPadding * 2;
    
    // Calculate visible lines
    int maxVisibleLines = static_cast<int>(maxHeight / m_style.historyLineSpacing);
    int startLine = std::max(0, static_cast<int>(m_historyLines.size()) - maxVisibleLines - m_historyScrollOffset);
    int endLine = std::min(static_cast<int>(m_historyLines.size()), startLine + maxVisibleLines);
    
    for (int i = startLine; i < endLine; ++i) {
        renderText(m_historyLines[i], textX, textY, textColor, m_style.historyTextSize);
        textY += m_style.historyLineSpacing;
    }
    
    // Show scroll indicators
    if (m_historyLines.size() > maxVisibleLines) {
        Graphics::Color indicatorColor = textColor;
        indicatorColor.a *= 0.7f;
        
        if (startLine > 0) {
            renderText("▲", m_style.historyWindowX + m_style.historyWindowWidth - 30.0f, m_style.historyWindowY + 20.0f, indicatorColor, m_style.historyTextSize);
        }
        
        if (endLine < static_cast<int>(m_historyLines.size())) {
            renderText("▼", m_style.historyWindowX + m_style.historyWindowWidth - 30.0f, m_style.historyWindowY + m_style.historyWindowHeight - 30.0f, indicatorColor, m_style.historyTextSize);
        }
    }
    
    // Show instructions
    Graphics::Color instructionColor = textColor;
    instructionColor.a *= 0.5f;
    renderText("Press H to close history", textX, m_style.historyWindowY + m_style.historyWindowHeight - 20.0f, instructionColor, m_style.historyTextSize * 0.8f);
}

float DialogueUI::renderText(const std::string& text, float x, float y, const Graphics::Color& color, float size) {
    if (!m_spriteRenderer || text.empty()) {
        return 0.0f;
    }
    
    // Simple text rendering using rectangles (placeholder implementation)
    // In a real implementation, you would use a proper font rendering system
    float charWidth = size * 0.6f;
    float charHeight = size;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        if (c == ' ') {
            x += charWidth;
            continue;
        }
        
        // Render character as a simple rectangle (placeholder)
        Graphics::Color charColor = color;
        charColor.a *= 0.8f; // Make it slightly transparent to simulate text
        
        renderRectangle(x, y, charWidth * 0.8f, charHeight * 0.8f, charColor, true);
        x += charWidth;
    }
    
    return charHeight;
}

void DialogueUI::renderRectangle(float x, float y, float width, float height, const Graphics::Color& color, bool filled) {
    if (!m_spriteRenderer) {
        return;
    }
    
    m_spriteRenderer->drawRectangle(x, y, width, height, color, filled);
}

float DialogueUI::getTextWidth(const std::string& text, float size) {
    return text.length() * size * 0.6f; // Approximate character width
}

float DialogueUI::getTextHeight(float size) {
    return size;
}

std::vector<std::string> DialogueUI::wrapText(const std::string& text, float maxWidth, float textSize) {
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word;
    std::string currentLine;
    
    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        
        if (getTextWidth(testLine, textSize) <= maxWidth) {
            currentLine = testLine;
        } else {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine = word;
            } else {
                // Word is too long, break it
                lines.push_back(word);
            }
        }
    }
    
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    
    return lines;
}

void DialogueUI::updateChoiceSelection() {
    if (m_currentChoices.empty()) {
        return;
    }
    
    if (m_inputManager->isActionJustActivated(ACTION_CHOICE_UP)) {
        m_selectedChoiceIndex = (m_selectedChoiceIndex - 1 + static_cast<int>(m_currentChoices.size())) % static_cast<int>(m_currentChoices.size());
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_CHOICE_DOWN)) {
        m_selectedChoiceIndex = (m_selectedChoiceIndex + 1) % static_cast<int>(m_currentChoices.size());
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
}

void DialogueUI::handleAdvanceInput() {
    if (m_inputManager->isActionJustActivated(ACTION_ADVANCE)) {
        if (!m_typewriterComplete) {
            skipTypewriter();
        } else {
            if (m_advanceCallback) {
                m_advanceCallback("");
            }
        }
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
}

void DialogueUI::handleChoiceInput() {
    updateChoiceSelection();
    
    if (m_inputManager->isActionJustActivated(ACTION_SELECT)) {
        if (m_selectedChoiceIndex >= 0 && m_selectedChoiceIndex < static_cast<int>(m_currentChoices.size())) {
            const auto& selectedChoice = m_currentChoices[m_selectedChoiceIndex];
            
            if (m_advanceCallback) {
                m_advanceCallback(selectedChoice.id);
            }
        }
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
}

void DialogueUI::handleHistoryInput() {
    if (m_inputManager->isActionJustActivated(ACTION_HISTORY)) {
        toggleHistory();
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
    
    // Handle history scrolling
    if (m_state == DialogueUIState::ShowingHistory) {
        if (m_inputManager->isActionJustActivated(ACTION_CHOICE_UP)) {
            m_historyScrollOffset = std::max(0, m_historyScrollOffset - 1);
            m_inputCooldown = INPUT_COOLDOWN_TIME;
        } else if (m_inputManager->isActionJustActivated(ACTION_CHOICE_DOWN)) {
            int maxScroll = std::max(0, static_cast<int>(m_historyLines.size()) - 10); // Approximate max visible lines
            m_historyScrollOffset = std::min(maxScroll, m_historyScrollOffset + 1);
            m_inputCooldown = INPUT_COOLDOWN_TIME;
        }
    }
}

void DialogueUI::updateTypewriter(float deltaTime) {
    if (m_typewriterComplete || m_currentText.empty()) {
        return;
    }
    
    m_typewriterTimer += deltaTime;
    
    float charactersToShow = m_typewriterTimer * m_style.typewriterSpeed;
    m_visibleCharacters = std::min(static_cast<int>(charactersToShow), static_cast<int>(m_currentText.length()));
    
    if (m_visibleCharacters >= static_cast<int>(m_currentText.length())) {
        m_typewriterComplete = true;
    }
}

void DialogueUI::startTypewriter() {
    m_typewriterTimer = 0.0f;
    m_visibleCharacters = 0;
    m_typewriterComplete = false;
}

bool DialogueUI::isTypewriterComplete() const {
    return m_typewriterComplete;
}

std::string DialogueUI::getVisibleText() const {
    if (!m_style.enableTypewriter || m_typewriterComplete) {
        return m_currentText;
    }
    
    return m_currentText.substr(0, m_visibleCharacters);
}

} // namespace UI
} // namespace RPGEngine