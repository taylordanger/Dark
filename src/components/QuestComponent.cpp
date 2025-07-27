#include "QuestComponent.h"
#include <algorithm>
#include <sstream>
#include <iostream>

namespace RPGEngine {
namespace Components {

// Static quest definitions registry
std::unordered_map<std::string, QuestDefinition> QuestComponent::s_questDefinitions;

QuestComponent::QuestComponent(EntityId entityId) : Component<QuestComponent>(entityId) {
    // Initialize component
}

QuestComponent::~QuestComponent() {
    // Cleanup
}

// Quest definition management

void QuestComponent::registerQuestDefinition(const QuestDefinition& definition) {
    if (definition.id.empty()) {
        std::cerr << "Warning: Cannot register quest with empty ID" << std::endl;
        return;
    }
    
    s_questDefinitions[definition.id] = definition;
}

const QuestDefinition* QuestComponent::getQuestDefinition(const std::string& questId) {
    auto it = s_questDefinitions.find(questId);
    return (it != s_questDefinitions.end()) ? &it->second : nullptr;
}

bool QuestComponent::hasQuestDefinition(const std::string& questId) {
    return s_questDefinitions.find(questId) != s_questDefinitions.end();
}

// Quest management

bool QuestComponent::startQuest(const std::string& questId, const std::string& startedBy) {
    // Check if quest definition exists
    const QuestDefinition* definition = getQuestDefinition(questId);
    if (!definition) {
        std::cerr << "Error: Quest definition not found: " << questId << std::endl;
        return false;
    }
    
    // Check if quest is already active or completed
    if (isQuestActive(questId)) {
        std::cerr << "Warning: Quest already active: " << questId << std::endl;
        return false;
    }
    
    if (isQuestCompleted(questId) && !definition->isRepeatable) {
        std::cerr << "Warning: Quest already completed and not repeatable: " << questId << std::endl;
        return false;
    }
    
    // Check prerequisites
    if (!checkPrerequisites(questId)) {
        std::cerr << "Warning: Quest prerequisites not met: " << questId << std::endl;
        return false;
    }
    
    // Create active quest
    ActiveQuest activeQuest(questId, QuestStatus::Active);
    activeQuest.startedBy = startedBy;
    activeQuest.objectives = definition->objectives;
    activeQuest.timeRemaining = (definition->timeLimit > 0) ? static_cast<float>(definition->timeLimit) : -1.0f;
    
    // Add to active quests
    m_activeQuests.push_back(activeQuest);
    
    // Trigger quest started event
    triggerQuestStarted(questId);
    
    return true;
}

bool QuestComponent::completeQuest(const std::string& questId, bool giveRewards) {
    ActiveQuest* activeQuest = getActiveQuestMutable(questId);
    if (!activeQuest) {
        std::cerr << "Warning: Cannot complete inactive quest: " << questId << std::endl;
        return false;
    }
    
    if (activeQuest->status != QuestStatus::Active) {
        std::cerr << "Warning: Cannot complete quest that is not active: " << questId << std::endl;
        return false;
    }
    
    // Check if quest can be completed
    if (!activeQuest->canComplete()) {
        std::cerr << "Warning: Quest objectives not completed: " << questId << std::endl;
        return false;
    }
    
    // Update status
    activeQuest->status = QuestStatus::Completed;
    
    // Give rewards if requested
    if (giveRewards) {
        giveQuestRewards(questId);
    }
    
    // Move to completed quests
    m_completedQuests.push_back(questId);
    removeActiveQuest(questId);
    
    // Trigger quest completed event
    triggerQuestCompleted(questId);
    
    return true;
}

bool QuestComponent::failQuest(const std::string& questId, const std::string& reason) {
    ActiveQuest* activeQuest = getActiveQuestMutable(questId);
    if (!activeQuest) {
        std::cerr << "Warning: Cannot fail inactive quest: " << questId << std::endl;
        return false;
    }
    
    if (activeQuest->status != QuestStatus::Active) {
        std::cerr << "Warning: Cannot fail quest that is not active: " << questId << std::endl;
        return false;
    }
    
    // Update status
    activeQuest->status = QuestStatus::Failed;
    
    // Move to failed quests
    m_failedQuests.push_back(questId);
    removeActiveQuest(questId);
    
    // Trigger quest failed event
    triggerQuestFailed(questId, reason);
    
    return true;
}

bool QuestComponent::abandonQuest(const std::string& questId) {
    ActiveQuest* activeQuest = getActiveQuestMutable(questId);
    if (!activeQuest) {
        std::cerr << "Warning: Cannot abandon inactive quest: " << questId << std::endl;
        return false;
    }
    
    if (activeQuest->status != QuestStatus::Active) {
        std::cerr << "Warning: Cannot abandon quest that is not active: " << questId << std::endl;
        return false;
    }
    
    // Update status
    activeQuest->status = QuestStatus::Abandoned;
    
    // Remove from active quests
    removeActiveQuest(questId);
    
    return true;
}

bool QuestComponent::isQuestActive(const std::string& questId) const {
    return getActiveQuest(questId) != nullptr;
}

bool QuestComponent::isQuestCompleted(const std::string& questId) const {
    return std::find(m_completedQuests.begin(), m_completedQuests.end(), questId) != m_completedQuests.end();
}

QuestStatus QuestComponent::getQuestStatus(const std::string& questId) const {
    const ActiveQuest* activeQuest = getActiveQuest(questId);
    if (activeQuest) {
        return activeQuest->status;
    }
    
    if (isQuestCompleted(questId)) {
        return QuestStatus::Completed;
    }
    
    if (std::find(m_failedQuests.begin(), m_failedQuests.end(), questId) != m_failedQuests.end()) {
        return QuestStatus::Failed;
    }
    
    // Check if quest is available (prerequisites met)
    if (hasQuestDefinition(questId) && checkPrerequisites(questId)) {
        return QuestStatus::Available;
    }
    
    return QuestStatus::NotStarted;
}

const ActiveQuest* QuestComponent::getActiveQuest(const std::string& questId) const {
    auto it = std::find_if(m_activeQuests.begin(), m_activeQuests.end(),
        [&questId](const ActiveQuest& quest) {
            return quest.questId == questId;
        });
    return (it != m_activeQuests.end()) ? &(*it) : nullptr;
}

// Objective management

bool QuestComponent::updateObjectiveProgress(const std::string& questId, const std::string& objectiveId, int amount) {
    ActiveQuest* activeQuest = getActiveQuestMutable(questId);
    if (!activeQuest || activeQuest->status != QuestStatus::Active) {
        return false;
    }
    
    QuestObjective* objective = activeQuest->getObjective(objectiveId);
    if (!objective) {
        return false;
    }
    
    bool wasCompleted = objective->addProgress(amount);
    
    if (wasCompleted) {
        triggerObjectiveCompleted(questId, objectiveId);
        
        // Check if quest can be auto-completed
        const QuestDefinition* definition = getQuestDefinition(questId);
        if (definition && definition->isAutoComplete && activeQuest->canComplete()) {
            completeQuest(questId, true);
        }
    }
    
    return wasCompleted;
}

bool QuestComponent::setObjectiveProgress(const std::string& questId, const std::string& objectiveId, int progress) {
    ActiveQuest* activeQuest = getActiveQuestMutable(questId);
    if (!activeQuest || activeQuest->status != QuestStatus::Active) {
        return false;
    }
    
    QuestObjective* objective = activeQuest->getObjective(objectiveId);
    if (!objective) {
        return false;
    }
    
    bool wasCompleted = objective->setProgress(progress);
    
    if (wasCompleted) {
        triggerObjectiveCompleted(questId, objectiveId);
        
        // Check if quest can be auto-completed
        const QuestDefinition* definition = getQuestDefinition(questId);
        if (definition && definition->isAutoComplete && activeQuest->canComplete()) {
            completeQuest(questId, true);
        }
    }
    
    return wasCompleted;
}

bool QuestComponent::completeObjective(const std::string& questId, const std::string& objectiveId) {
    ActiveQuest* activeQuest = getActiveQuestMutable(questId);
    if (!activeQuest || activeQuest->status != QuestStatus::Active) {
        return false;
    }
    
    QuestObjective* objective = activeQuest->getObjective(objectiveId);
    if (!objective || objective->isCompleted) {
        return false;
    }
    
    objective->currentCount = objective->requiredCount;
    objective->isCompleted = true;
    
    triggerObjectiveCompleted(questId, objectiveId);
    
    // Check if quest can be auto-completed
    const QuestDefinition* definition = getQuestDefinition(questId);
    if (definition && definition->isAutoComplete && activeQuest->canComplete()) {
        completeQuest(questId, true);
    }
    
    return true;
}

bool QuestComponent::isObjectiveCompleted(const std::string& questId, const std::string& objectiveId) const {
    const ActiveQuest* activeQuest = getActiveQuest(questId);
    if (!activeQuest) {
        return false;
    }
    
    const QuestObjective* objective = activeQuest->getObjective(objectiveId);
    return objective ? objective->isCompleted : false;
}

int QuestComponent::getObjectiveProgress(const std::string& questId, const std::string& objectiveId) const {
    const ActiveQuest* activeQuest = getActiveQuest(questId);
    if (!activeQuest) {
        return -1;
    }
    
    const QuestObjective* objective = activeQuest->getObjective(objectiveId);
    return objective ? objective->currentCount : -1;
}

// Quest tracking helpers

void QuestComponent::trackKill(const std::string& enemyType, int count) {
    for (auto& activeQuest : m_activeQuests) {
        if (activeQuest.status != QuestStatus::Active) continue;
        
        for (auto& objective : activeQuest.objectives) {
            if (objective.type == ObjectiveType::Kill && 
                objective.target == enemyType && 
                !objective.isCompleted) {
                
                bool wasCompleted = objective.addProgress(count);
                if (wasCompleted) {
                    triggerObjectiveCompleted(activeQuest.questId, objective.id);
                    
                    // Check if quest can be auto-completed
                    const QuestDefinition* definition = getQuestDefinition(activeQuest.questId);
                    if (definition && definition->isAutoComplete && activeQuest.canComplete()) {
                        completeQuest(activeQuest.questId, true);
                        break; // Quest completed, no need to continue
                    }
                }
            }
        }
    }
}

void QuestComponent::trackItemCollection(const std::string& itemId, int count) {
    for (auto& activeQuest : m_activeQuests) {
        if (activeQuest.status != QuestStatus::Active) continue;
        
        for (auto& objective : activeQuest.objectives) {
            if (objective.type == ObjectiveType::Collect && 
                objective.target == itemId && 
                !objective.isCompleted) {
                
                bool wasCompleted = objective.addProgress(count);
                if (wasCompleted) {
                    triggerObjectiveCompleted(activeQuest.questId, objective.id);
                    
                    // Check if quest can be auto-completed
                    const QuestDefinition* definition = getQuestDefinition(activeQuest.questId);
                    if (definition && definition->isAutoComplete && activeQuest.canComplete()) {
                        completeQuest(activeQuest.questId, true);
                        break; // Quest completed, no need to continue
                    }
                }
            }
        }
    }
}

void QuestComponent::trackNPCInteraction(const std::string& npcId) {
    for (auto& activeQuest : m_activeQuests) {
        if (activeQuest.status != QuestStatus::Active) continue;
        
        for (auto& objective : activeQuest.objectives) {
            if (objective.type == ObjectiveType::Talk && 
                objective.target == npcId && 
                !objective.isCompleted) {
                
                bool wasCompleted = objective.addProgress(1);
                if (wasCompleted) {
                    triggerObjectiveCompleted(activeQuest.questId, objective.id);
                    
                    // Check if quest can be auto-completed
                    const QuestDefinition* definition = getQuestDefinition(activeQuest.questId);
                    if (definition && definition->isAutoComplete && activeQuest.canComplete()) {
                        completeQuest(activeQuest.questId, true);
                        break; // Quest completed, no need to continue
                    }
                }
            }
        }
    }
}

void QuestComponent::trackLocationVisit(const std::string& locationId) {
    for (auto& activeQuest : m_activeQuests) {
        if (activeQuest.status != QuestStatus::Active) continue;
        
        for (auto& objective : activeQuest.objectives) {
            if (objective.type == ObjectiveType::Reach && 
                objective.target == locationId && 
                !objective.isCompleted) {
                
                bool wasCompleted = objective.addProgress(1);
                if (wasCompleted) {
                    triggerObjectiveCompleted(activeQuest.questId, objective.id);
                    
                    // Check if quest can be auto-completed
                    const QuestDefinition* definition = getQuestDefinition(activeQuest.questId);
                    if (definition && definition->isAutoComplete && activeQuest.canComplete()) {
                        completeQuest(activeQuest.questId, true);
                        break; // Quest completed, no need to continue
                    }
                }
            }
        }
    }
}

void QuestComponent::trackCustomObjective(const std::string& objectiveType, const std::string& target, int count) {
    for (auto& activeQuest : m_activeQuests) {
        if (activeQuest.status != QuestStatus::Active) continue;
        
        for (auto& objective : activeQuest.objectives) {
            if (objective.type == ObjectiveType::Custom && 
                objective.parameters.find("custom_type") != objective.parameters.end() &&
                objective.parameters.at("custom_type") == objectiveType &&
                objective.target == target && 
                !objective.isCompleted) {
                
                bool wasCompleted = objective.addProgress(count);
                if (wasCompleted) {
                    triggerObjectiveCompleted(activeQuest.questId, objective.id);
                    
                    // Check if quest can be auto-completed
                    const QuestDefinition* definition = getQuestDefinition(activeQuest.questId);
                    if (definition && definition->isAutoComplete && activeQuest.canComplete()) {
                        completeQuest(activeQuest.questId, true);
                        break; // Quest completed, no need to continue
                    }
                }
            }
        }
    }
}

// Quest variables

void QuestComponent::setQuestVariable(const std::string& questId, const std::string& key, const std::string& value) {
    ActiveQuest* activeQuest = getActiveQuestMutable(questId);
    if (activeQuest) {
        activeQuest->setVariable(key, value);
    }
}

std::string QuestComponent::getQuestVariable(const std::string& questId, const std::string& key, const std::string& defaultValue) const {
    const ActiveQuest* activeQuest = getActiveQuest(questId);
    return activeQuest ? activeQuest->getVariable(key, defaultValue) : defaultValue;
}

// Time management

void QuestComponent::updateQuestTimers(float deltaTime) {
    std::vector<std::string> questsToFail;
    
    for (auto& activeQuest : m_activeQuests) {
        if (activeQuest.status != QuestStatus::Active) continue;
        
        if (activeQuest.timeRemaining > 0.0f) {
            activeQuest.timeRemaining -= deltaTime;
            
            if (activeQuest.timeRemaining <= 0.0f) {
                questsToFail.push_back(activeQuest.questId);
            }
        }
    }
    
    // Fail timed out quests
    for (const std::string& questId : questsToFail) {
        failQuest(questId, "Time limit exceeded");
    }
}

// Private helper methods

bool QuestComponent::checkPrerequisites(const std::string& questId) const {
    const QuestDefinition* definition = getQuestDefinition(questId);
    if (!definition) {
        return false;
    }
    
    for (const std::string& prerequisite : definition->prerequisites) {
        if (!isQuestCompleted(prerequisite)) {
            return false;
        }
    }
    
    return true;
}

void QuestComponent::giveQuestRewards(const std::string& questId) {
    const QuestDefinition* definition = getQuestDefinition(questId);
    if (!definition) {
        return;
    }
    
    for (const QuestReward& reward : definition->rewards) {
        triggerRewardGiven(reward);
    }
}

ActiveQuest* QuestComponent::getActiveQuestMutable(const std::string& questId) {
    auto it = std::find_if(m_activeQuests.begin(), m_activeQuests.end(),
        [&questId](const ActiveQuest& quest) {
            return quest.questId == questId;
        });
    return (it != m_activeQuests.end()) ? &(*it) : nullptr;
}

bool QuestComponent::removeActiveQuest(const std::string& questId) {
    auto it = std::find_if(m_activeQuests.begin(), m_activeQuests.end(),
        [&questId](const ActiveQuest& quest) {
            return quest.questId == questId;
        });
    
    if (it != m_activeQuests.end()) {
        m_activeQuests.erase(it);
        return true;
    }
    
    return false;
}

// Event triggers

void QuestComponent::triggerQuestStarted(const std::string& questId) {
    if (m_questStartedCallback) {
        m_questStartedCallback(questId);
    }
}

void QuestComponent::triggerQuestCompleted(const std::string& questId) {
    if (m_questCompletedCallback) {
        m_questCompletedCallback(questId);
    }
}

void QuestComponent::triggerQuestFailed(const std::string& questId, const std::string& reason) {
    if (m_questFailedCallback) {
        m_questFailedCallback(questId, reason);
    }
}

void QuestComponent::triggerObjectiveCompleted(const std::string& questId, const std::string& objectiveId) {
    if (m_objectiveCompletedCallback) {
        m_objectiveCompletedCallback(questId, objectiveId);
    }
}

void QuestComponent::triggerRewardGiven(const QuestReward& reward) {
    if (m_rewardGivenCallback) {
        m_rewardGivenCallback(reward);
    }
}

// Serialization

std::string QuestComponent::serialize() const {
    std::ostringstream oss;
    
    // Serialize active quests
    oss << "ACTIVE_QUESTS:" << m_activeQuests.size() << "\n";
    for (const auto& quest : m_activeQuests) {
        oss << "QUEST:" << quest.questId << ":" << static_cast<int>(quest.status) 
            << ":" << quest.timeRemaining << ":" << quest.startedBy << "\n";
        
        oss << "OBJECTIVES:" << quest.objectives.size() << "\n";
        for (const auto& objective : quest.objectives) {
            oss << "OBJ:" << objective.id << ":" << objective.description 
                << ":" << static_cast<int>(objective.type) << ":" << objective.target
                << ":" << objective.requiredCount << ":" << objective.currentCount
                << ":" << (objective.isCompleted ? 1 : 0) << ":" << (objective.isOptional ? 1 : 0)
                << ":" << (objective.isHidden ? 1 : 0) << "\n";
        }
        
        oss << "VARIABLES:" << quest.variables.size() << "\n";
        for (const auto& var : quest.variables) {
            oss << "VAR:" << var.first << ":" << var.second << "\n";
        }
    }
    
    // Serialize completed quests
    oss << "COMPLETED_QUESTS:" << m_completedQuests.size() << "\n";
    for (const auto& questId : m_completedQuests) {
        oss << "COMPLETED:" << questId << "\n";
    }
    
    // Serialize failed quests
    oss << "FAILED_QUESTS:" << m_failedQuests.size() << "\n";
    for (const auto& questId : m_failedQuests) {
        oss << "FAILED:" << questId << "\n";
    }
    
    return oss.str();
}

bool QuestComponent::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string line;
    
    m_activeQuests.clear();
    m_completedQuests.clear();
    m_failedQuests.clear();
    
    try {
        while (std::getline(iss, line)) {
            if (line.find("ACTIVE_QUESTS:") == 0) {
                int count = std::stoi(line.substr(14));
                
                for (int i = 0; i < count; ++i) {
                    if (!std::getline(iss, line) || line.find("QUEST:") != 0) {
                        return false;
                    }
                    
                    // Parse quest data
                    std::istringstream questStream(line.substr(6));
                    std::string questId, statusStr, timeStr, startedBy;
                    
                    if (!std::getline(questStream, questId, ':') ||
                        !std::getline(questStream, statusStr, ':') ||
                        !std::getline(questStream, timeStr, ':') ||
                        !std::getline(questStream, startedBy)) {
                        return false;
                    }
                    
                    ActiveQuest quest(questId, static_cast<QuestStatus>(std::stoi(statusStr)));
                    quest.timeRemaining = std::stof(timeStr);
                    quest.startedBy = startedBy;
                    
                    // Parse objectives
                    if (!std::getline(iss, line) || line.find("OBJECTIVES:") != 0) {
                        return false;
                    }
                    
                    int objCount = std::stoi(line.substr(11));
                    for (int j = 0; j < objCount; ++j) {
                        if (!std::getline(iss, line) || line.find("OBJ:") != 0) {
                            return false;
                        }
                        
                        std::istringstream objStream(line.substr(4));
                        std::string objId, desc, typeStr, target, reqStr, curStr, compStr, optStr, hidStr;
                        
                        if (!std::getline(objStream, objId, ':') ||
                            !std::getline(objStream, desc, ':') ||
                            !std::getline(objStream, typeStr, ':') ||
                            !std::getline(objStream, target, ':') ||
                            !std::getline(objStream, reqStr, ':') ||
                            !std::getline(objStream, curStr, ':') ||
                            !std::getline(objStream, compStr, ':') ||
                            !std::getline(objStream, optStr, ':') ||
                            !std::getline(objStream, hidStr)) {
                            return false;
                        }
                        
                        QuestObjective objective(objId, desc, static_cast<ObjectiveType>(std::stoi(typeStr)), target, std::stoi(reqStr));
                        objective.currentCount = std::stoi(curStr);
                        objective.isCompleted = (std::stoi(compStr) == 1);
                        objective.isOptional = (std::stoi(optStr) == 1);
                        objective.isHidden = (std::stoi(hidStr) == 1);
                        
                        quest.objectives.push_back(objective);
                    }
                    
                    // Parse variables
                    if (!std::getline(iss, line) || line.find("VARIABLES:") != 0) {
                        return false;
                    }
                    
                    int varCount = std::stoi(line.substr(10));
                    for (int j = 0; j < varCount; ++j) {
                        if (!std::getline(iss, line) || line.find("VAR:") != 0) {
                            return false;
                        }
                        
                        std::istringstream varStream(line.substr(4));
                        std::string key, value;
                        
                        if (!std::getline(varStream, key, ':') ||
                            !std::getline(varStream, value)) {
                            return false;
                        }
                        
                        quest.variables[key] = value;
                    }
                    
                    m_activeQuests.push_back(quest);
                }
            }
            else if (line.find("COMPLETED_QUESTS:") == 0) {
                int count = std::stoi(line.substr(17));
                
                for (int i = 0; i < count; ++i) {
                    if (!std::getline(iss, line) || line.find("COMPLETED:") != 0) {
                        return false;
                    }
                    
                    m_completedQuests.push_back(line.substr(10));
                }
            }
            else if (line.find("FAILED_QUESTS:") == 0) {
                int count = std::stoi(line.substr(14));
                
                for (int i = 0; i < count; ++i) {
                    if (!std::getline(iss, line) || line.find("FAILED:") != 0) {
                        return false;
                    }
                    
                    m_failedQuests.push_back(line.substr(7));
                }
            }
        }
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error deserializing QuestComponent: " << e.what() << std::endl;
        return false;
    }
}

} // namespace Components
} // namespace RPGEngine