#pragma once

#include "../graphics/SpriteRenderer.h"
#include "../input/InputManager.h"
#include "../components/QuestComponent.h"
#include "../systems/System.h"
#include "../core/Types.h"
#include "../graphics/Sprite.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>

// Forward declarations
namespace RPGEngine {
namespace Systems {
class QuestDialogueIntegration;
}
}

namespace RPGEngine {
namespace UI {

/**
 * Quest UI style configuration
 */
struct QuestUIStyle {
    // Quest tracker styling
    float trackerX = 20.0f;
    float trackerY = 20.0f;
    float trackerWidth = 300.0f;
    float trackerMaxHeight = 400.0f;
    Graphics::Color trackerBackgroundColor = Graphics::Color(0.0f, 0.0f, 0.0f, 0.7f);
    Graphics::Color trackerBorderColor = Graphics::Color(0.8f, 0.8f, 0.8f, 0.8f);
    float trackerBorderWidth = 1.0f;
    float trackerPadding = 10.0f;
    
    // Quest text styling
    Graphics::Color questTitleColor = Graphics::Color(1.0f, 1.0f, 0.0f, 1.0f);
    Graphics::Color objectiveColor = Graphics::Color(0.9f, 0.9f, 0.9f, 1.0f);
    Graphics::Color completedObjectiveColor = Graphics::Color(0.5f, 1.0f, 0.5f, 1.0f);
    Graphics::Color optionalObjectiveColor = Graphics::Color(0.7f, 0.7f, 0.9f, 1.0f);
    float questTitleSize = 14.0f;
    float objectiveSize = 12.0f;
    float lineSpacing = 16.0f;
    
    // Quest log styling
    float logWindowX = 100.0f;
    float logWindowY = 100.0f;
    float logWindowWidth = 600.0f;
    float logWindowHeight = 500.0f;
    Graphics::Color logBackgroundColor = Graphics::Color(0.1f, 0.1f, 0.1f, 0.9f);
    Graphics::Color logBorderColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    float logBorderWidth = 2.0f;
    float logPadding = 20.0f;
    
    // Quest categories
    Graphics::Color mainQuestColor = Graphics::Color(1.0f, 0.8f, 0.0f, 1.0f);
    Graphics::Color sideQuestColor = Graphics::Color(0.8f, 0.8f, 1.0f, 1.0f);
    Graphics::Color dailyQuestColor = Graphics::Color(0.8f, 1.0f, 0.8f, 1.0f);
    
    // Notification styling
    float notificationX = 400.0f;
    float notificationY = 100.0f;
    float notificationWidth = 350.0f;
    float notificationHeight = 80.0f;
    Graphics::Color notificationBackgroundColor = Graphics::Color(0.0f, 0.0f, 0.0f, 0.8f);
    Graphics::Color notificationBorderColor = Graphics::Color(1.0f, 1.0f, 0.0f, 1.0f);
    Graphics::Color notificationTextColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    float notificationTextSize = 14.0f;
    float notificationDuration = 3.0f;
    float notificationFadeTime = 0.5f;
};

/**
 * Quest UI state enumeration
 */
enum class QuestUIState {
    TrackerOnly,
    ShowingLog,
    Hidden
};

/**
 * Quest notification structure
 */
struct QuestNotification {
    std::string type;           // "quest_started", "quest_completed", "objective_completed", "reward_received"
    std::string title;          // Notification title
    std::string message;        // Notification message
    Graphics::Color color;      // Notification color
    float timeRemaining;        // Time remaining to show
    float alpha;                // Current alpha for fade effect
    
    QuestNotification(const std::string& notifType = "", const std::string& notifTitle = "",
                     const std::string& notifMessage = "", const Graphics::Color& notifColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f))
        : type(notifType), title(notifTitle), message(notifMessage), color(notifColor), timeRemaining(3.0f), alpha(1.0f) {}
};

/**
 * Quest UI system
 * Handles quest tracking, quest log, and quest notifications
 */
class QuestUI : public System {
public:
    /**
     * Constructor
     * @param spriteRenderer Sprite renderer for drawing
     * @param inputManager Input manager for handling input
     */
    QuestUI(std::shared_ptr<Graphics::SpriteRenderer> spriteRenderer,
            std::shared_ptr<Input::InputManager> inputManager);
    
    /**
     * Destructor
     */
    ~QuestUI();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Set the quest component to display
     * @param questComponent Quest component
     */
    void setQuestComponent(std::shared_ptr<Components::QuestComponent> questComponent);
    
    /**
     * Show quest tracker
     */
    void showTracker();
    
    /**
     * Hide quest tracker
     */
    void hideTracker();
    
    /**
     * Toggle quest log
     */
    void toggleQuestLog();
    
    /**
     * Show quest log
     */
    void showQuestLog();
    
    /**
     * Hide quest log
     */
    void hideQuestLog();
    
    /**
     * Check if quest tracker is visible
     * @return true if visible
     */
    bool isTrackerVisible() const;
    
    /**
     * Check if quest log is visible
     * @return true if visible
     */
    bool isQuestLogVisible() const;
    
    /**
     * Set UI style
     * @param style New style configuration
     */
    void setStyle(const QuestUIStyle& style);
    
    /**
     * Get UI style
     * @return Current style configuration
     */
    const QuestUIStyle& getStyle() const { return m_style; }
    
    /**
     * Add quest notification
     * @param type Notification type
     * @param title Notification title
     * @param message Notification message
     * @param color Notification color
     */
    void addNotification(const std::string& type, const std::string& title, 
                        const std::string& message, const Graphics::Color& color = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f));
    
    /**
     * Show quest started notification
     * @param questId Quest ID
     */
    void showQuestStartedNotification(const std::string& questId);
    
    /**
     * Show quest completed notification
     * @param questId Quest ID
     */
    void showQuestCompletedNotification(const std::string& questId);
    
    /**
     * Show objective completed notification
     * @param questId Quest ID
     * @param objectiveId Objective ID
     */
    void showObjectiveCompletedNotification(const std::string& questId, const std::string& objectiveId);
    
    /**
     * Show reward received notification
     * @param reward Quest reward
     */
    void showRewardReceivedNotification(const Components::QuestReward& reward);
    
    /**
     * Show multiple rewards notification
     * @param rewards Vector of quest rewards
     * @param questName Quest name for context
     */
    void showMultipleRewardsNotification(const std::vector<Components::QuestReward>& rewards, 
                                        const std::string& questName);
    
    /**
     * Show quest completion with rewards
     * @param questId Quest ID
     * @param rewards Vector of rewards received
     */
    void showQuestCompletionWithRewards(const std::string& questId, 
                                       const std::vector<Components::QuestReward>& rewards);
    
    /**
     * Set maximum tracked quests
     * @param maxQuests Maximum number of quests to show in tracker
     */
    void setMaxTrackedQuests(int maxQuests) { m_maxTrackedQuests = maxQuests; }
    
    /**
     * Track specific quest in UI
     * @param questId Quest ID to track
     */
    void trackQuest(const std::string& questId);
    
    /**
     * Untrack specific quest in UI
     * @param questId Quest ID to untrack
     */
    void untrackQuest(const std::string& questId);
    
    /**
     * Check if quest is being tracked
     * @param questId Quest ID
     * @return true if quest is tracked
     */
    bool isQuestTracked(const std::string& questId) const;
    
    /**
     * Get tracked quest IDs
     * @return Vector of tracked quest IDs
     */
    const std::vector<std::string>& getTrackedQuestIds() const { return m_trackedQuestIds; }
    
    /**
     * Show quest progress notification
     * @param questId Quest ID
     * @param objectiveId Objective ID
     * @param progress Current progress
     * @param maxProgress Maximum progress
     */
    void showQuestProgressNotification(const std::string& questId, const std::string& objectiveId, 
                                      int progress, int maxProgress);
    
    /**
     * Show quest failed notification
     * @param questId Quest ID
     * @param reason Failure reason
     */
    void showQuestFailedNotification(const std::string& questId, const std::string& reason);
    
    /**
     * Set quest integration system
     * @param integration Quest-dialogue integration system
     */
    void setQuestIntegration(std::shared_ptr<class QuestDialogueIntegration> integration) {
        m_questIntegration = integration;
    }
    
    /**
     * Set quest log close callback
     * @param callback Function called when quest log should close
     */
    void setQuestLogCloseCallback(std::function<void()> callback);
    
private:
    /**
     * Update input handling
     * @param deltaTime Time elapsed since last update
     */
    void updateInput(float deltaTime);
    
    /**
     * Update notifications
     * @param deltaTime Time elapsed since last update
     */
    void updateNotifications(float deltaTime);
    
    /**
     * Render the quest UI
     */
    void render();
    
    /**
     * Render quest tracker
     */
    void renderQuestTracker();
    
    /**
     * Render quest log
     */
    void renderQuestLog();
    
    /**
     * Render quest notifications
     */
    void renderNotifications();
    
    /**
     * Render a quest entry in tracker
     * @param quest Active quest
     * @param x X position
     * @param y Y position
     * @return Height of rendered quest
     */
    float renderTrackedQuest(const Components::ActiveQuest& quest, float x, float y);
    
    /**
     * Render a quest entry in log
     * @param quest Active quest
     * @param x X position
     * @param y Y position
     * @param width Available width
     * @return Height of rendered quest
     */
    float renderLogQuest(const Components::ActiveQuest& quest, float x, float y, float width);
    
    /**
     * Render quest objective
     * @param objective Quest objective
     * @param x X position
     * @param y Y position
     * @param width Available width
     * @return Height of rendered objective
     */
    float renderObjective(const Components::QuestObjective& objective, float x, float y, float width);
    
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
     * Get quest color based on category
     * @param category Quest category
     * @return Quest color
     */
    Graphics::Color getQuestCategoryColor(const std::string& category) const;
    
    /**
     * Handle quest log input
     */
    void handleQuestLogInput();
    
    /**
     * Get tracked quests (limited by max tracked quests)
     * @return Vector of quests to track
     */
    std::vector<Components::ActiveQuest> getTrackedQuests() const;
    
    // Dependencies
    std::shared_ptr<Graphics::SpriteRenderer> m_spriteRenderer;
    std::shared_ptr<Input::InputManager> m_inputManager;
    std::shared_ptr<Components::QuestComponent> m_questComponent;
    
    // UI state
    QuestUIState m_state;
    QuestUIStyle m_style;
    
    // Quest tracking
    int m_maxTrackedQuests;
    int m_selectedQuestIndex;
    int m_questLogScrollOffset;
    std::vector<std::string> m_trackedQuestIds;
    
    // Notifications
    std::vector<QuestNotification> m_notifications;
    
    // Input timing
    float m_inputCooldown;
    static const float INPUT_COOLDOWN_TIME;
    
    // Callbacks
    std::function<void()> m_questLogCloseCallback;
    
    // Integration
    std::shared_ptr<class QuestDialogueIntegration> m_questIntegration;
    
    // Input action names
    static const std::string ACTION_TOGGLE_QUEST_LOG;
    static const std::string ACTION_QUEST_LOG_UP;
    static const std::string ACTION_QUEST_LOG_DOWN;
    static const std::string ACTION_CLOSE_QUEST_LOG;
};

} // namespace UI
} // namespace RPGEngine