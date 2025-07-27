#pragma once

#include "Component.h"
#include "../entities/Entity.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace RPGEngine {
namespace Components {

/**
 * Quest status enumeration
 */
enum class QuestStatus {
    NotStarted,     // Quest hasn't been started yet
    Available,      // Quest is available to start
    Active,         // Quest is currently active
    Completed,      // Quest has been completed
    Failed,         // Quest has failed
    Abandoned       // Quest was abandoned by player
};

/**
 * Objective type enumeration
 */
enum class ObjectiveType {
    Kill,           // Kill specific enemies
    Collect,        // Collect specific items
    Deliver,        // Deliver items to NPCs
    Talk,           // Talk to specific NPCs
    Reach,          // Reach specific locations
    Escort,         // Escort NPCs
    Survive,        // Survive for a duration
    Custom          // Custom objective type
};

/**
 * Quest objective structure
 */
struct QuestObjective {
    std::string id;                     // Objective ID
    std::string description;            // Objective description
    ObjectiveType type;                 // Objective type
    std::string target;                 // Target (enemy type, item ID, NPC ID, etc.)
    int requiredCount;                  // Required count to complete
    int currentCount;                   // Current progress
    bool isCompleted;                   // Whether objective is completed
    bool isOptional;                    // Whether objective is optional
    bool isHidden;                      // Whether objective is hidden from UI
    std::unordered_map<std::string, std::string> parameters; // Additional parameters
    
    QuestObjective(const std::string& objId = "", const std::string& desc = "",
                   ObjectiveType objType = ObjectiveType::Custom, const std::string& objTarget = "",
                   int required = 1)
        : id(objId), description(desc), type(objType), target(objTarget),
          requiredCount(required), currentCount(0), isCompleted(false),
          isOptional(false), isHidden(false) {}
    
    /**
     * Get completion percentage
     * @return Completion percentage (0.0 to 1.0)
     */
    float getCompletionPercentage() const {
        if (requiredCount <= 0) return 1.0f;
        return std::min(1.0f, static_cast<float>(currentCount) / static_cast<float>(requiredCount));
    }
    
    /**
     * Check if objective can be completed
     * @return true if current count meets required count
     */
    bool canComplete() const {
        return currentCount >= requiredCount;
    }
    
    /**
     * Add progress to objective
     * @param amount Amount to add
     * @return true if objective was completed by this progress
     */
    bool addProgress(int amount = 1) {
        if (isCompleted) return false;
        
        currentCount += amount;
        if (currentCount >= requiredCount) {
            currentCount = requiredCount;
            isCompleted = true;
            return true;
        }
        return false;
    }
    
    /**
     * Set progress directly
     * @param count New progress count
     * @return true if objective was completed
     */
    bool setProgress(int count) {
        if (isCompleted) return false;
        
        currentCount = std::max(0, count);
        if (currentCount >= requiredCount) {
            currentCount = requiredCount;
            isCompleted = true;
            return true;
        }
        return false;
    }
};

/**
 * Quest reward structure
 */
struct QuestReward {
    std::string type;                   // Reward type (experience, item, gold, etc.)
    std::string target;                 // Reward target (item ID, stat name, etc.)
    int amount;                         // Reward amount
    std::unordered_map<std::string, std::string> parameters; // Additional parameters
    
    QuestReward(const std::string& rewardType = "", const std::string& rewardTarget = "",
                int rewardAmount = 0)
        : type(rewardType), target(rewardTarget), amount(rewardAmount) {}
};

/**
 * Quest definition structure
 */
struct QuestDefinition {
    std::string id;                     // Quest ID
    std::string name;                   // Quest name
    std::string description;            // Quest description
    std::string category;               // Quest category (main, side, daily, etc.)
    int level;                          // Recommended level
    std::vector<std::string> prerequisites; // Required quests to complete first
    std::vector<QuestObjective> objectives; // Quest objectives
    std::vector<QuestReward> rewards;   // Quest rewards
    std::unordered_map<std::string, std::string> metadata; // Additional metadata
    bool isRepeatable;                  // Whether quest can be repeated
    bool isAutoComplete;                // Whether quest completes automatically
    int timeLimit;                      // Time limit in seconds (-1 for no limit)
    
    QuestDefinition(const std::string& questId = "", const std::string& questName = "")
        : id(questId), name(questName), level(1), isRepeatable(false),
          isAutoComplete(false), timeLimit(-1) {}
    
    /**
     * Add objective to quest
     * @param objective Objective to add
     */
    void addObjective(const QuestObjective& objective) {
        objectives.push_back(objective);
    }
    
    /**
     * Add reward to quest
     * @param reward Reward to add
     */
    void addReward(const QuestReward& reward) {
        rewards.push_back(reward);
    }
    
    /**
     * Check if quest has prerequisite
     * @param questId Quest ID to check
     * @return true if quest is a prerequisite
     */
    bool hasPrerequisite(const std::string& questId) const {
        return std::find(prerequisites.begin(), prerequisites.end(), questId) != prerequisites.end();
    }
};

/**
 * Active quest structure
 */
struct ActiveQuest {
    std::string questId;                // Quest ID
    QuestStatus status;                 // Current status
    std::vector<QuestObjective> objectives; // Current objectives
    float timeRemaining;                // Time remaining (if time limited)
    std::string startedBy;              // Who/what started the quest
    std::unordered_map<std::string, std::string> variables; // Quest-specific variables
    
    ActiveQuest(const std::string& id = "", QuestStatus questStatus = QuestStatus::NotStarted)
        : questId(id), status(questStatus), timeRemaining(-1.0f) {}
    
    /**
     * Get objective by ID
     * @param objectiveId Objective ID
     * @return Objective pointer, or nullptr if not found
     */
    QuestObjective* getObjective(const std::string& objectiveId) {
        auto it = std::find_if(objectives.begin(), objectives.end(),
            [&objectiveId](const QuestObjective& obj) {
                return obj.id == objectiveId;
            });
        return (it != objectives.end()) ? &(*it) : nullptr;
    }
    
    /**
     * Get objective by ID (const version)
     * @param objectiveId Objective ID
     * @return Objective pointer, or nullptr if not found
     */
    const QuestObjective* getObjective(const std::string& objectiveId) const {
        auto it = std::find_if(objectives.begin(), objectives.end(),
            [&objectiveId](const QuestObjective& obj) {
                return obj.id == objectiveId;
            });
        return (it != objectives.end()) ? &(*it) : nullptr;
    }
    
    /**
     * Check if all required objectives are completed
     * @return true if quest can be completed
     */
    bool canComplete() const {
        for (const auto& objective : objectives) {
            if (!objective.isOptional && !objective.isCompleted) {
                return false;
            }
        }
        return true;
    }
    
    /**
     * Get completion percentage
     * @return Overall quest completion percentage (0.0 to 1.0)
     */
    float getCompletionPercentage() const {
        if (objectives.empty()) return 1.0f;
        
        int requiredObjectives = 0;
        float totalProgress = 0.0f;
        
        for (const auto& objective : objectives) {
            if (!objective.isOptional) {
                requiredObjectives++;
                totalProgress += objective.getCompletionPercentage();
            }
        }
        
        return (requiredObjectives > 0) ? (totalProgress / requiredObjectives) : 1.0f;
    }
    
    /**
     * Set quest variable
     * @param key Variable key
     * @param value Variable value
     */
    void setVariable(const std::string& key, const std::string& value) {
        variables[key] = value;
    }
    
    /**
     * Get quest variable
     * @param key Variable key
     * @param defaultValue Default value if not found
     * @return Variable value
     */
    std::string getVariable(const std::string& key, const std::string& defaultValue = "") const {
        auto it = variables.find(key);
        return (it != variables.end()) ? it->second : defaultValue;
    }
};

/**
 * Quest component
 * Manages quest tracking, objectives, and completion
 */
class QuestComponent : public Component<QuestComponent> {
public:
    /**
     * Constructor
     * @param entityId Entity ID
     */
    explicit QuestComponent(EntityId entityId);
    
    /**
     * Destructor
     */
    ~QuestComponent();
    
    // Quest definition management
    
    /**
     * Register quest definition
     * @param definition Quest definition
     */
    static void registerQuestDefinition(const QuestDefinition& definition);
    
    /**
     * Get quest definition
     * @param questId Quest ID
     * @return Quest definition, or nullptr if not found
     */
    static const QuestDefinition* getQuestDefinition(const std::string& questId);
    
    /**
     * Check if quest definition exists
     * @param questId Quest ID
     * @return true if definition exists
     */
    static bool hasQuestDefinition(const std::string& questId);
    
    /**
     * Get all quest definitions
     * @return Map of quest IDs to definitions
     */
    static const std::unordered_map<std::string, QuestDefinition>& getQuestDefinitions() {
        return s_questDefinitions;
    }
    
    // Quest management
    
    /**
     * Start quest
     * @param questId Quest ID
     * @param startedBy Who/what started the quest
     * @return true if quest was started successfully
     */
    bool startQuest(const std::string& questId, const std::string& startedBy = "");
    
    /**
     * Complete quest
     * @param questId Quest ID
     * @param giveRewards Whether to give rewards
     * @return true if quest was completed successfully
     */
    bool completeQuest(const std::string& questId, bool giveRewards = true);
    
    /**
     * Fail quest
     * @param questId Quest ID
     * @param reason Failure reason
     * @return true if quest was failed successfully
     */
    bool failQuest(const std::string& questId, const std::string& reason = "");
    
    /**
     * Abandon quest
     * @param questId Quest ID
     * @return true if quest was abandoned successfully
     */
    bool abandonQuest(const std::string& questId);
    
    /**
     * Check if quest is active
     * @param questId Quest ID
     * @return true if quest is active
     */
    bool isQuestActive(const std::string& questId) const;
    
    /**
     * Check if quest is completed
     * @param questId Quest ID
     * @return true if quest is completed
     */
    bool isQuestCompleted(const std::string& questId) const;
    
    /**
     * Get quest status
     * @param questId Quest ID
     * @return Quest status
     */
    QuestStatus getQuestStatus(const std::string& questId) const;
    
    /**
     * Get active quest
     * @param questId Quest ID
     * @return Active quest, or nullptr if not found
     */
    const ActiveQuest* getActiveQuest(const std::string& questId) const;
    
    /**
     * Get all active quests
     * @return Vector of active quests
     */
    const std::vector<ActiveQuest>& getActiveQuests() const { return m_activeQuests; }
    
    /**
     * Get completed quests
     * @return Vector of completed quest IDs
     */
    const std::vector<std::string>& getCompletedQuests() const { return m_completedQuests; }
    
    // Objective management
    
    /**
     * Update objective progress
     * @param questId Quest ID
     * @param objectiveId Objective ID
     * @param amount Amount to add to progress
     * @return true if objective was completed
     */
    bool updateObjectiveProgress(const std::string& questId, const std::string& objectiveId, int amount = 1);
    
    /**
     * Set objective progress
     * @param questId Quest ID
     * @param objectiveId Objective ID
     * @param progress New progress value
     * @return true if objective was completed
     */
    bool setObjectiveProgress(const std::string& questId, const std::string& objectiveId, int progress);
    
    /**
     * Complete objective
     * @param questId Quest ID
     * @param objectiveId Objective ID
     * @return true if objective was completed
     */
    bool completeObjective(const std::string& questId, const std::string& objectiveId);
    
    /**
     * Check if objective is completed
     * @param questId Quest ID
     * @param objectiveId Objective ID
     * @return true if objective is completed
     */
    bool isObjectiveCompleted(const std::string& questId, const std::string& objectiveId) const;
    
    /**
     * Get objective progress
     * @param questId Quest ID
     * @param objectiveId Objective ID
     * @return Current progress, or -1 if not found
     */
    int getObjectiveProgress(const std::string& questId, const std::string& objectiveId) const;
    
    // Quest tracking helpers
    
    /**
     * Track kill
     * @param enemyType Enemy type that was killed
     * @param count Number killed
     */
    void trackKill(const std::string& enemyType, int count = 1);
    
    /**
     * Track item collection
     * @param itemId Item ID that was collected
     * @param count Number collected
     */
    void trackItemCollection(const std::string& itemId, int count = 1);
    
    /**
     * Track NPC interaction
     * @param npcId NPC ID that was talked to
     */
    void trackNPCInteraction(const std::string& npcId);
    
    /**
     * Track location visit
     * @param locationId Location ID that was visited
     */
    void trackLocationVisit(const std::string& locationId);
    
    /**
     * Track custom objective
     * @param objectiveType Custom objective type
     * @param target Target identifier
     * @param count Progress amount
     */
    void trackCustomObjective(const std::string& objectiveType, const std::string& target, int count = 1);
    
    // Quest variables
    
    /**
     * Set quest variable
     * @param questId Quest ID
     * @param key Variable key
     * @param value Variable value
     */
    void setQuestVariable(const std::string& questId, const std::string& key, const std::string& value);
    
    /**
     * Get quest variable
     * @param questId Quest ID
     * @param key Variable key
     * @param defaultValue Default value if not found
     * @return Variable value
     */
    std::string getQuestVariable(const std::string& questId, const std::string& key, const std::string& defaultValue = "") const;
    
    // Time management
    
    /**
     * Update quest timers
     * @param deltaTime Time elapsed since last update
     */
    void updateQuestTimers(float deltaTime);
    
    // Callbacks
    
    /**
     * Set quest started callback
     * @param callback Function called when quest is started
     */
    void setQuestStartedCallback(std::function<void(const std::string&)> callback) {
        m_questStartedCallback = callback;
    }
    
    /**
     * Set quest completed callback
     * @param callback Function called when quest is completed
     */
    void setQuestCompletedCallback(std::function<void(const std::string&)> callback) {
        m_questCompletedCallback = callback;
    }
    
    /**
     * Set quest failed callback
     * @param callback Function called when quest fails
     */
    void setQuestFailedCallback(std::function<void(const std::string&, const std::string&)> callback) {
        m_questFailedCallback = callback;
    }
    
    /**
     * Set objective completed callback
     * @param callback Function called when objective is completed
     */
    void setObjectiveCompletedCallback(std::function<void(const std::string&, const std::string&)> callback) {
        m_objectiveCompletedCallback = callback;
    }
    
    /**
     * Set reward given callback
     * @param callback Function called when reward is given
     */
    void setRewardGivenCallback(std::function<void(const QuestReward&)> callback) {
        m_rewardGivenCallback = callback;
    }
    
    // Serialization
    
    /**
     * Serialize component data
     * @return Serialized data
     */
    std::string serialize() const;
    
    /**
     * Deserialize component data
     * @param data Serialized data
     * @return true if successful
     */
    bool deserialize(const std::string& data);
    
private:
    /**
     * Check quest prerequisites
     * @param questId Quest ID
     * @return true if all prerequisites are met
     */
    bool checkPrerequisites(const std::string& questId) const;
    
    /**
     * Give quest rewards
     * @param questId Quest ID
     */
    void giveQuestRewards(const std::string& questId);
    
    /**
     * Get active quest (non-const)
     * @param questId Quest ID
     * @return Active quest, or nullptr if not found
     */
    ActiveQuest* getActiveQuestMutable(const std::string& questId);
    
    /**
     * Remove active quest
     * @param questId Quest ID
     * @return true if quest was removed
     */
    bool removeActiveQuest(const std::string& questId);
    
    /**
     * Trigger quest started event
     * @param questId Quest ID
     */
    void triggerQuestStarted(const std::string& questId);
    
    /**
     * Trigger quest completed event
     * @param questId Quest ID
     */
    void triggerQuestCompleted(const std::string& questId);
    
    /**
     * Trigger quest failed event
     * @param questId Quest ID
     * @param reason Failure reason
     */
    void triggerQuestFailed(const std::string& questId, const std::string& reason);
    
    /**
     * Trigger objective completed event
     * @param questId Quest ID
     * @param objectiveId Objective ID
     */
    void triggerObjectiveCompleted(const std::string& questId, const std::string& objectiveId);
    
    /**
     * Trigger reward given event
     * @param reward Quest reward
     */
    void triggerRewardGiven(const QuestReward& reward);
    
    // Quest data
    std::vector<ActiveQuest> m_activeQuests;
    std::vector<std::string> m_completedQuests;
    std::vector<std::string> m_failedQuests;
    
    // Quest definitions (static registry)
    static std::unordered_map<std::string, QuestDefinition> s_questDefinitions;
    
    // Callbacks
    std::function<void(const std::string&)> m_questStartedCallback;
    std::function<void(const std::string&)> m_questCompletedCallback;
    std::function<void(const std::string&, const std::string&)> m_questFailedCallback;
    std::function<void(const std::string&, const std::string&)> m_objectiveCompletedCallback;
    std::function<void(const QuestReward&)> m_rewardGivenCallback;
};

} // namespace Components
} // namespace RPGEngine