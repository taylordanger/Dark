#include "QuestUI.h"
#include <algorithm>
#include <sstream>
#include <iostream>

namespace RPGEngine {
namespace UI {

// Static constants
const float QuestUI::INPUT_COOLDOWN_TIME = 0.2f;
const std::string QuestUI::ACTION_TOGGLE_QUEST_LOG = "toggle_quest_log";
const std::string QuestUI::ACTION_QUEST_LOG_UP = "quest_log_up";
const std::string QuestUI::ACTION_QUEST_LOG_DOWN = "quest_log_down";
const std::string QuestUI::ACTION_CLOSE_QUEST_LOG = "close_quest_log";

QuestUI::QuestUI(std::shared_ptr<Graphics::SpriteRenderer> spriteRenderer,
                 std::shared_ptr<Input::InputManager> inputManager)
    : System("QuestUI")
    , m_spriteRenderer(spriteRenderer)
    , m_inputManager(inputManager)
    , m_questComponent(nullptr)
    , m_state(QuestUIState::TrackerOnly)
    , m_maxTrackedQuests(5)
    , m_selectedQuestIndex(0)
    , m_questLogScrollOffset(0)
    , m_inputCooldown(0.0f)
{
}

QuestUI::~QuestUI() {
}

bool QuestUI::onInitialize() {
    if (!m_inputManager) {
        std::cerr << "QuestUI: InputManager is required" << std::endl;
        return false;
    }
    
    // Create input actions for quest UI
    m_inputManager->createAction(ACTION_TOGGLE_QUEST_LOG);
    m_inputManager->createAction(ACTION_QUEST_LOG_UP);
    m_inputManager->createAction(ACTION_QUEST_LOG_DOWN);
    m_inputManager->createAction(ACTION_CLOSE_QUEST_LOG);
    
    // Bind default keys
    m_inputManager->bindKeyToAction(ACTION_TOGGLE_QUEST_LOG, Input::KeyCode::Q);
    m_inputManager->bindKeyToAction(ACTION_QUEST_LOG_UP, Input::KeyCode::Up);
    m_inputManager->bindKeyToAction(ACTION_QUEST_LOG_DOWN, Input::KeyCode::Down);
    m_inputManager->bindKeyToAction(ACTION_CLOSE_QUEST_LOG, Input::KeyCode::Escape);
    
    std::cout << "QuestUI initialized successfully" << std::endl;
    return true;
}

void QuestUI::onUpdate(float deltaTime) {
    if (m_state == QuestUIState::Hidden) {
        return;
    }
    
    // Update input cooldown
    if (m_inputCooldown > 0.0f) {
        m_inputCooldown -= deltaTime;
    }
    
    updateInput(deltaTime);
    updateNotifications(deltaTime);
    render();
}

void QuestUI::onShutdown() {
    // Remove input actions
    if (m_inputManager) {
        m_inputManager->removeAction(ACTION_TOGGLE_QUEST_LOG);
        m_inputManager->removeAction(ACTION_QUEST_LOG_UP);
        m_inputManager->removeAction(ACTION_QUEST_LOG_DOWN);
        m_inputManager->removeAction(ACTION_CLOSE_QUEST_LOG);
    }
    
    std::cout << "QuestUI shutdown" << std::endl;
}

void QuestUI::setQuestComponent(std::shared_ptr<Components::QuestComponent> questComponent) {
    m_questComponent = questComponent;
    
    if (m_questComponent) {
        // Set up callbacks to show notifications when quest events occur
        m_questComponent->setQuestStartedCallback([this](const std::string& questId) {
            showQuestStartedNotification(questId);
        });
        
        m_questComponent->setQuestCompletedCallback([this](const std::string& questId) {
            showQuestCompletedNotification(questId);
        });
        
        m_questComponent->setObjectiveCompletedCallback([this](const std::string& questId, const std::string& objectiveId) {
            showObjectiveCompletedNotification(questId, objectiveId);
        });
        
        m_questComponent->setRewardGivenCallback([this](const Components::QuestReward& reward) {
            showRewardReceivedNotification(reward);
        });
    }
}

void QuestUI::showTracker() {
    if (m_state == QuestUIState::Hidden) {
        m_state = QuestUIState::TrackerOnly;
    }
}

void QuestUI::hideTracker() {
    if (m_state == QuestUIState::TrackerOnly) {
        m_state = QuestUIState::Hidden;
    }
}

void QuestUI::toggleQuestLog() {
    if (m_state == QuestUIState::ShowingLog) {
        hideQuestLog();
    } else {
        showQuestLog();
    }
}

void QuestUI::showQuestLog() {
    m_state = QuestUIState::ShowingLog;
    m_selectedQuestIndex = 0;
    m_questLogScrollOffset = 0;
}

void QuestUI::hideQuestLog() {
    if (m_state == QuestUIState::ShowingLog) {
        m_state = QuestUIState::TrackerOnly;
    }
}

bool QuestUI::isTrackerVisible() const {
    return m_state == QuestUIState::TrackerOnly || m_state == QuestUIState::ShowingLog;
}

bool QuestUI::isQuestLogVisible() const {
    return m_state == QuestUIState::ShowingLog;
}

void QuestUI::setStyle(const QuestUIStyle& style) {
    m_style = style;
}

void QuestUI::addNotification(const std::string& type, const std::string& title, 
                             const std::string& message, const Graphics::Color& color) {
    QuestNotification notification(type, title, message, color);
    notification.timeRemaining = m_style.notificationDuration;
    notification.alpha = 1.0f;
    
    m_notifications.push_back(notification);
    
    // Limit number of notifications
    if (m_notifications.size() > 5) {
        m_notifications.erase(m_notifications.begin());
    }
}

void QuestUI::showQuestStartedNotification(const std::string& questId) {
    if (!m_questComponent) return;
    
    const Components::QuestDefinition* questDef = Components::QuestComponent::getQuestDefinition(questId);
    if (questDef) {
        Graphics::Color color = getQuestCategoryColor(questDef->category);
        addNotification("quest_started", "Quest Started", questDef->name, color);
    }
}

void QuestUI::showQuestCompletedNotification(const std::string& questId) {
    if (!m_questComponent) return;
    
    const Components::QuestDefinition* questDef = Components::QuestComponent::getQuestDefinition(questId);
    if (questDef) {
        Graphics::Color color = Graphics::Color(0.5f, 1.0f, 0.5f, 1.0f); // Green for completion
        addNotification("quest_completed", "Quest Completed", questDef->name, color);
    }
}

void QuestUI::showObjectiveCompletedNotification(const std::string& questId, const std::string& objectiveId) {
    if (!m_questComponent) return;
    
    const Components::ActiveQuest* quest = m_questComponent->getActiveQuest(questId);
    if (quest) {
        const Components::QuestObjective* objective = quest->getObjective(objectiveId);
        if (objective) {
            Graphics::Color color = Graphics::Color(0.8f, 0.8f, 1.0f, 1.0f); // Light blue for objectives
            addNotification("objective_completed", "Objective Complete", objective->description, color);
        }
    }
}

void QuestUI::showRewardReceivedNotification(const Components::QuestReward& reward) {
    std::string message = reward.target;
    if (reward.amount > 0) {
        message += " x" + std::to_string(reward.amount);
    }
    
    Graphics::Color color = Graphics::Color(1.0f, 1.0f, 0.5f, 1.0f); // Yellow for rewards
    addNotification("reward_received", "Reward Received", message, color);
}

void QuestUI::showMultipleRewardsNotification(const std::vector<Components::QuestReward>& rewards, 
                                             const std::string& questName) {
    if (rewards.empty()) return;
    
    std::string message;
    if (rewards.size() == 1) {
        const auto& reward = rewards[0];
        message = reward.target;
        if (reward.amount > 0) {
            message += " x" + std::to_string(reward.amount);
        }
    } else {
        message = std::to_string(rewards.size()) + " rewards received";
    }
    
    Graphics::Color color = Graphics::Color(1.0f, 1.0f, 0.5f, 1.0f); // Yellow for rewards
    addNotification("multiple_rewards", "Quest Rewards", message, color);
    
    // Show individual reward notifications for each reward
    for (const auto& reward : rewards) {
        showRewardReceivedNotification(reward);
    }
}

void QuestUI::showQuestCompletionWithRewards(const std::string& questId, 
                                            const std::vector<Components::QuestReward>& rewards) {
    // Show quest completion notification first
    showQuestCompletedNotification(questId);
    
    // Then show rewards if any
    if (!rewards.empty()) {
        const Components::QuestDefinition* questDef = Components::QuestComponent::getQuestDefinition(questId);
        std::string questName = questDef ? questDef->name : questId;
        showMultipleRewardsNotification(rewards, questName);
    }
}

void QuestUI::setQuestLogCloseCallback(std::function<void()> callback) {
    m_questLogCloseCallback = callback;
}

void QuestUI::trackQuest(const std::string& questId) {
    // Check if already tracked
    auto it = std::find(m_trackedQuestIds.begin(), m_trackedQuestIds.end(), questId);
    if (it == m_trackedQuestIds.end()) {
        m_trackedQuestIds.push_back(questId);
        
        // Limit tracked quests
        if (static_cast<int>(m_trackedQuestIds.size()) > m_maxTrackedQuests) {
            m_trackedQuestIds.erase(m_trackedQuestIds.begin());
        }
        
        std::cout << "Now tracking quest: " << questId << std::endl;
    }
}

void QuestUI::untrackQuest(const std::string& questId) {
    auto it = std::find(m_trackedQuestIds.begin(), m_trackedQuestIds.end(), questId);
    if (it != m_trackedQuestIds.end()) {
        m_trackedQuestIds.erase(it);
        std::cout << "Stopped tracking quest: " << questId << std::endl;
    }
}

bool QuestUI::isQuestTracked(const std::string& questId) const {
    return std::find(m_trackedQuestIds.begin(), m_trackedQuestIds.end(), questId) != m_trackedQuestIds.end();
}

void QuestUI::showQuestProgressNotification(const std::string& questId, const std::string& objectiveId, 
                                           int progress, int maxProgress) {
    if (!m_questComponent) return;
    
    const Components::ActiveQuest* quest = m_questComponent->getActiveQuest(questId);
    if (quest) {
        const Components::QuestObjective* objective = quest->getObjective(objectiveId);
        if (objective) {
            std::string message = objective->description + " (" + std::to_string(progress) + "/" + std::to_string(maxProgress) + ")";
            Graphics::Color color = Graphics::Color(0.8f, 0.8f, 1.0f, 1.0f); // Light blue for progress
            addNotification("quest_progress", "Quest Progress", message, color);
        }
    }
}

void QuestUI::showQuestFailedNotification(const std::string& questId, const std::string& reason) {
    if (!m_questComponent) return;
    
    const Components::QuestDefinition* questDef = Components::QuestComponent::getQuestDefinition(questId);
    if (questDef) {
        std::string message = questDef->name;
        if (!reason.empty()) {
            message += " - " + reason;
        }
        Graphics::Color color = Graphics::Color(1.0f, 0.5f, 0.5f, 1.0f); // Red for failure
        addNotification("quest_failed", "Quest Failed", message, color);
    }
}

void QuestUI::updateInput(float deltaTime) {
    if (!m_inputManager || m_inputCooldown > 0.0f) {
        return;
    }
    
    // Toggle quest log
    if (m_inputManager->isActionJustActivated(ACTION_TOGGLE_QUEST_LOG)) {
        toggleQuestLog();
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
    
    // Handle quest log input
    if (m_state == QuestUIState::ShowingLog) {
        handleQuestLogInput();
    }
}

void QuestUI::updateNotifications(float deltaTime) {
    // Update notification timers and remove expired ones
    for (auto it = m_notifications.begin(); it != m_notifications.end();) {
        it->timeRemaining -= deltaTime;
        
        // Handle fade out
        if (it->timeRemaining <= m_style.notificationFadeTime) {
            it->alpha = std::max(0.0f, it->timeRemaining / m_style.notificationFadeTime);
        }
        
        if (it->timeRemaining <= 0.0f) {
            it = m_notifications.erase(it);
        } else {
            ++it;
        }
    }
}

void QuestUI::render() {
    if (!m_spriteRenderer) {
        return;
    }
    
    // Render quest tracker
    if (isTrackerVisible()) {
        renderQuestTracker();
    }
    
    // Render quest log
    if (isQuestLogVisible()) {
        renderQuestLog();
    }
    
    // Render notifications
    renderNotifications();
}

void QuestUI::renderQuestTracker() {
    if (!m_questComponent) {
        return;
    }
    
    auto trackedQuests = getTrackedQuests();
    if (trackedQuests.empty()) {
        return;
    }
    
    // Calculate tracker height
    float totalHeight = m_style.trackerPadding * 2;
    for (const auto& quest : trackedQuests) {
        totalHeight += m_style.lineSpacing * 2; // Title + spacing
        for (const auto& objective : quest.objectives) {
            if (!objective.isHidden) {
                totalHeight += m_style.lineSpacing;
            }
        }
        totalHeight += m_style.lineSpacing; // Extra spacing between quests
    }
    
    float trackerHeight = std::min(totalHeight, m_style.trackerMaxHeight);
    
    // Render background
    renderRectangle(m_style.trackerX, m_style.trackerY, m_style.trackerWidth, trackerHeight, 
                   m_style.trackerBackgroundColor, true);
    
    // Render border
    if (m_style.trackerBorderWidth > 0.0f) {
        renderRectangle(m_style.trackerX, m_style.trackerY, m_style.trackerWidth, m_style.trackerBorderWidth, 
                       m_style.trackerBorderColor, true);
        renderRectangle(m_style.trackerX, m_style.trackerY + trackerHeight - m_style.trackerBorderWidth, 
                       m_style.trackerWidth, m_style.trackerBorderWidth, m_style.trackerBorderColor, true);
        renderRectangle(m_style.trackerX, m_style.trackerY, m_style.trackerBorderWidth, trackerHeight, 
                       m_style.trackerBorderColor, true);
        renderRectangle(m_style.trackerX + m_style.trackerWidth - m_style.trackerBorderWidth, m_style.trackerY, 
                       m_style.trackerBorderWidth, trackerHeight, m_style.trackerBorderColor, true);
    }
    
    // Render quests
    float currentY = m_style.trackerY + m_style.trackerPadding;
    float availableHeight = trackerHeight - m_style.trackerPadding * 2;
    
    for (const auto& quest : trackedQuests) {
        if (currentY - m_style.trackerY > availableHeight) {
            break; // Don't render beyond available space
        }
        
        float questHeight = renderTrackedQuest(quest, m_style.trackerX + m_style.trackerPadding, currentY);
        currentY += questHeight + m_style.lineSpacing;
    }
}

void QuestUI::renderQuestLog() {
    if (!m_questComponent) {
        return;
    }
    
    // Render background
    renderRectangle(m_style.logWindowX, m_style.logWindowY, m_style.logWindowWidth, m_style.logWindowHeight, 
                   m_style.logBackgroundColor, true);
    
    // Render border
    if (m_style.logBorderWidth > 0.0f) {
        renderRectangle(m_style.logWindowX, m_style.logWindowY, m_style.logWindowWidth, m_style.logBorderWidth, 
                       m_style.logBorderColor, true);
        renderRectangle(m_style.logWindowX, m_style.logWindowY + m_style.logWindowHeight - m_style.logBorderWidth, 
                       m_style.logWindowWidth, m_style.logBorderWidth, m_style.logBorderColor, true);
        renderRectangle(m_style.logWindowX, m_style.logWindowY, m_style.logBorderWidth, m_style.logWindowHeight, 
                       m_style.logBorderColor, true);
        renderRectangle(m_style.logWindowX + m_style.logWindowWidth - m_style.logBorderWidth, m_style.logWindowY, 
                       m_style.logBorderWidth, m_style.logWindowHeight, m_style.logBorderColor, true);
    }
    
    // Render title
    float titleY = m_style.logWindowY + m_style.logPadding;
    renderText("Quest Log", m_style.logWindowX + m_style.logPadding, titleY, 
               m_style.questTitleColor, m_style.questTitleSize + 4.0f);
    
    // Render quests
    float currentY = titleY + m_style.lineSpacing * 2;
    float availableWidth = m_style.logWindowWidth - m_style.logPadding * 2;
    float availableHeight = m_style.logWindowHeight - m_style.logPadding * 2 - (m_style.lineSpacing * 3);
    
    const auto& activeQuests = m_questComponent->getActiveQuests();
    
    // Calculate which quests to show based on scroll offset
    int startIndex = m_questLogScrollOffset;
    int endIndex = std::min(static_cast<int>(activeQuests.size()), startIndex + 10); // Show up to 10 quests
    
    for (int i = startIndex; i < endIndex; ++i) {
        if (currentY - m_style.logWindowY > availableHeight) {
            break;
        }
        
        const auto& quest = activeQuests[i];
        float questHeight = renderLogQuest(quest, m_style.logWindowX + m_style.logPadding, currentY, availableWidth);
        currentY += questHeight + m_style.lineSpacing * 2;
    }
    
    // Show scroll indicators
    if (activeQuests.size() > 10) {
        Graphics::Color indicatorColor = m_style.objectiveColor;
        indicatorColor.a *= 0.7f;
        
        if (m_questLogScrollOffset > 0) {
            renderText("▲", m_style.logWindowX + m_style.logWindowWidth - 30.0f, 
                      m_style.logWindowY + 30.0f, indicatorColor, m_style.objectiveSize);
        }
        
        if (m_questLogScrollOffset + 10 < static_cast<int>(activeQuests.size())) {
            renderText("▼", m_style.logWindowX + m_style.logWindowWidth - 30.0f, 
                      m_style.logWindowY + m_style.logWindowHeight - 50.0f, indicatorColor, m_style.objectiveSize);
        }
    }
    
    // Show instructions
    Graphics::Color instructionColor = m_style.objectiveColor;
    instructionColor.a *= 0.5f;
    renderText("Press Q to close quest log", m_style.logWindowX + m_style.logPadding, 
               m_style.logWindowY + m_style.logWindowHeight - 25.0f, instructionColor, m_style.objectiveSize * 0.8f);
}

void QuestUI::renderNotifications() {
    float notificationY = m_style.notificationY;
    
    for (const auto& notification : m_notifications) {
        // Apply alpha for fade effect
        Graphics::Color bgColor = m_style.notificationBackgroundColor;
        Graphics::Color borderColor = m_style.notificationBorderColor;
        Graphics::Color textColor = notification.color;
        
        bgColor.a *= notification.alpha;
        borderColor.a *= notification.alpha;
        textColor.a *= notification.alpha;
        
        // Render background
        renderRectangle(m_style.notificationX, notificationY, m_style.notificationWidth, m_style.notificationHeight, 
                       bgColor, true);
        
        // Render border
        renderRectangle(m_style.notificationX, notificationY, m_style.notificationWidth, 2.0f, borderColor, true);
        renderRectangle(m_style.notificationX, notificationY + m_style.notificationHeight - 2.0f, 
                       m_style.notificationWidth, 2.0f, borderColor, true);
        renderRectangle(m_style.notificationX, notificationY, 2.0f, m_style.notificationHeight, borderColor, true);
        renderRectangle(m_style.notificationX + m_style.notificationWidth - 2.0f, notificationY, 
                       2.0f, m_style.notificationHeight, borderColor, true);
        
        // Render text
        float textX = m_style.notificationX + 10.0f;
        float textY = notificationY + 10.0f;
        
        renderText(notification.title, textX, textY, textColor, m_style.notificationTextSize);
        textY += m_style.lineSpacing;
        
        Graphics::Color messageColor = m_style.notificationTextColor;
        messageColor.a *= notification.alpha;
        renderText(notification.message, textX, textY, messageColor, m_style.notificationTextSize * 0.9f);
        
        notificationY += m_style.notificationHeight + 10.0f;
    }
}

float QuestUI::renderTrackedQuest(const Components::ActiveQuest& quest, float x, float y) {
    float startY = y;
    
    // Get quest definition for additional info
    const Components::QuestDefinition* questDef = Components::QuestComponent::getQuestDefinition(quest.questId);
    std::string questName = questDef ? questDef->name : quest.questId;
    
    // Render quest title with category color
    Graphics::Color titleColor = questDef ? getQuestCategoryColor(questDef->category) : m_style.questTitleColor;
    renderText(questName, x, y, titleColor, m_style.questTitleSize);
    y += m_style.lineSpacing;
    
    // Render objectives
    for (const auto& objective : quest.objectives) {
        if (objective.isHidden) continue;
        
        Graphics::Color objColor = objective.isCompleted ? m_style.completedObjectiveColor :
                                  (objective.isOptional ? m_style.optionalObjectiveColor : m_style.objectiveColor);
        
        std::string objText = objective.isCompleted ? "✓ " : "• ";
        objText += objective.description;
        
        if (objective.requiredCount > 1) {
            objText += " (" + std::to_string(objective.currentCount) + "/" + std::to_string(objective.requiredCount) + ")";
        }
        
        renderText(objText, x + 10.0f, y, objColor, m_style.objectiveSize);
        y += m_style.lineSpacing;
    }
    
    return y - startY;
}

float QuestUI::renderLogQuest(const Components::ActiveQuest& quest, float x, float y, float width) {
    float startY = y;
    
    // Get quest definition for additional info
    const Components::QuestDefinition* questDef = Components::QuestComponent::getQuestDefinition(quest.questId);
    std::string questName = questDef ? questDef->name : quest.questId;
    
    // Render quest title
    Graphics::Color titleColor = questDef ? getQuestCategoryColor(questDef->category) : m_style.questTitleColor;
    renderText(questName, x, y, titleColor, m_style.questTitleSize);
    y += m_style.lineSpacing;
    
    // Render quest description if available
    if (questDef && !questDef->description.empty()) {
        auto descLines = wrapText(questDef->description, width - 20.0f, m_style.objectiveSize * 0.9f);
        Graphics::Color descColor = m_style.objectiveColor;
        descColor.a *= 0.8f;
        
        for (const auto& line : descLines) {
            renderText(line, x + 10.0f, y, descColor, m_style.objectiveSize * 0.9f);
            y += m_style.lineSpacing * 0.8f;
        }
        y += m_style.lineSpacing * 0.5f;
    }
    
    // Render objectives
    for (const auto& objective : quest.objectives) {
        if (objective.isHidden) continue;
        
        y += renderObjective(objective, x + 10.0f, y, width - 20.0f);
    }
    
    // Show completion percentage
    float completion = quest.getCompletionPercentage();
    std::string completionText = "Progress: " + std::to_string(static_cast<int>(completion * 100)) + "%";
    Graphics::Color completionColor = m_style.objectiveColor;
    completionColor.a *= 0.7f;
    renderText(completionText, x, y, completionColor, m_style.objectiveSize * 0.8f);
    y += m_style.lineSpacing;
    
    return y - startY;
}

float QuestUI::renderObjective(const Components::QuestObjective& objective, float x, float y, float width) {
    Graphics::Color objColor = objective.isCompleted ? m_style.completedObjectiveColor :
                              (objective.isOptional ? m_style.optionalObjectiveColor : m_style.objectiveColor);
    
    std::string objText = objective.isCompleted ? "✓ " : "• ";
    objText += objective.description;
    
    if (objective.requiredCount > 1) {
        objText += " (" + std::to_string(objective.currentCount) + "/" + std::to_string(objective.requiredCount) + ")";
    }
    
    if (objective.isOptional) {
        objText += " (Optional)";
    }
    
    auto objLines = wrapText(objText, width, m_style.objectiveSize);
    float totalHeight = 0.0f;
    
    for (const auto& line : objLines) {
        renderText(line, x, y, objColor, m_style.objectiveSize);
        y += m_style.lineSpacing;
        totalHeight += m_style.lineSpacing;
    }
    
    return totalHeight;
}

float QuestUI::renderText(const std::string& text, float x, float y, const Graphics::Color& color, float size) {
    if (!m_spriteRenderer || text.empty()) {
        return 0.0f;
    }
    
    // Simple text rendering using rectangles (placeholder implementation)
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
        charColor.a *= 0.8f;
        
        renderRectangle(x, y, charWidth * 0.8f, charHeight * 0.8f, charColor, true);
        x += charWidth;
    }
    
    return charHeight;
}

void QuestUI::renderRectangle(float x, float y, float width, float height, const Graphics::Color& color, bool filled) {
    if (!m_spriteRenderer) {
        return;
    }
    
    m_spriteRenderer->drawRectangle(x, y, width, height, color, filled);
}

float QuestUI::getTextWidth(const std::string& text, float size) {
    return text.length() * size * 0.6f;
}

float QuestUI::getTextHeight(float size) {
    return size;
}

std::vector<std::string> QuestUI::wrapText(const std::string& text, float maxWidth, float textSize) {
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
                lines.push_back(word);
            }
        }
    }
    
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    
    return lines;
}

Graphics::Color QuestUI::getQuestCategoryColor(const std::string& category) const {
    if (category == "main") {
        return m_style.mainQuestColor;
    } else if (category == "side") {
        return m_style.sideQuestColor;
    } else if (category == "daily") {
        return m_style.dailyQuestColor;
    }
    return m_style.questTitleColor;
}

void QuestUI::handleQuestLogInput() {
    if (m_inputManager->isActionJustActivated(ACTION_CLOSE_QUEST_LOG)) {
        if (m_questLogCloseCallback) {
            m_questLogCloseCallback();
        } else {
            hideQuestLog();
        }
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
    
    // Handle scrolling
    if (m_questComponent) {
        const auto& activeQuests = m_questComponent->getActiveQuests();
        int maxScroll = std::max(0, static_cast<int>(activeQuests.size()) - 10);
        
        if (m_inputManager->isActionJustActivated(ACTION_QUEST_LOG_UP)) {
            m_questLogScrollOffset = std::max(0, m_questLogScrollOffset - 1);
            m_inputCooldown = INPUT_COOLDOWN_TIME;
        } else if (m_inputManager->isActionJustActivated(ACTION_QUEST_LOG_DOWN)) {
            m_questLogScrollOffset = std::min(maxScroll, m_questLogScrollOffset + 1);
            m_inputCooldown = INPUT_COOLDOWN_TIME;
        }
    }
}

std::vector<Components::ActiveQuest> QuestUI::getTrackedQuests() const {
    if (!m_questComponent) {
        return {};
    }
    
    const auto& activeQuests = m_questComponent->getActiveQuests();
    std::vector<Components::ActiveQuest> trackedQuests;
    
    // If we have specific tracked quest IDs, use those
    if (!m_trackedQuestIds.empty()) {
        for (const auto& questId : m_trackedQuestIds) {
            const Components::ActiveQuest* quest = m_questComponent->getActiveQuest(questId);
            if (quest) {
                trackedQuests.push_back(*quest);
            }
        }
    } else {
        // Otherwise, take up to maxTrackedQuests active quests
        int count = 0;
        for (const auto& quest : activeQuests) {
            if (count >= m_maxTrackedQuests) {
                break;
            }
            trackedQuests.push_back(quest);
            count++;
        }
    }
    
    return trackedQuests;
}

} // namespace UI
} // namespace RPGEngine