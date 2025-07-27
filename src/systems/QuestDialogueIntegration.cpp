#include "QuestDialogueIntegration.h"
#include <iostream>
#include <algorithm>

namespace RPGEngine {
namespace Systems {

QuestDialogueIntegration::QuestDialogueIntegration(std::shared_ptr<RPGEngine::EntityManager> entityManager)
    : m_entityManager(entityManager), m_initialized(false) {
}

QuestDialogueIntegration::~QuestDialogueIntegration() {
    shutdown();
}

bool QuestDialogueIntegration::initialize() {
    if (m_initialized) {
        return true;
    }
    
    std::cout << "Initializing Quest-Dialogue Integration..." << std::endl;
    m_initialized = true;
    return true;
}

void QuestDialogueIntegration::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    m_questComponents.clear();
    m_dialogueComponents.clear();
    
    std::cout << "Quest-Dialogue Integration shutdown" << std::endl;
    m_initialized = false;
}

void QuestDialogueIntegration::registerQuestComponent(Components::EntityId entityId, 
                                                     std::shared_ptr<Components::QuestComponent> questComponent) {
    if (!questComponent) {
        return;
    }
    
    m_questComponents[entityId] = questComponent;
    setupQuestDialogueCallbacks(questComponent);
    
    std::cout << "Registered quest component for entity " << entityId << std::endl;
}

void QuestDialogueIntegration::registerDialogueComponent(Components::EntityId entityId, 
                                                        std::shared_ptr<Components::DialogueComponent> dialogueComponent) {
    if (!dialogueComponent) {
        return;
    }
    
    m_dialogueComponents[entityId] = dialogueComponent;
    setupDialogueQuestActions(dialogueComponent);
    
    std::cout << "Registered dialogue component for entity " << entityId << std::endl;
}

void QuestDialogueIntegration::unregisterQuestComponent(Components::EntityId entityId) {
    auto it = m_questComponents.find(entityId);
    if (it != m_questComponents.end()) {
        m_questComponents.erase(it);
        std::cout << "Unregistered quest component for entity " << entityId << std::endl;
    }
}

void QuestDialogueIntegration::unregisterDialogueComponent(Components::EntityId entityId) {
    auto it = m_dialogueComponents.find(entityId);
    if (it != m_dialogueComponents.end()) {
        m_dialogueComponents.erase(it);
        std::cout << "Unregistered dialogue component for entity " << entityId << std::endl;
    }
}

void QuestDialogueIntegration::setupDialogueQuestActions(std::shared_ptr<Components::DialogueComponent> dialogueComponent) {
    if (!dialogueComponent) {
        return;
    }
    
    // Set up external condition evaluator
    dialogueComponent->setExternalConditionEvaluator([this, dialogueComponent](const Components::DialogueCondition& condition) {
        // Find the entity ID for this dialogue component
        Components::EntityId entityId = 0;
        for (const auto& pair : m_dialogueComponents) {
            if (pair.second == dialogueComponent) {
                entityId = pair.first;
                break;
            }
        }
        
        if (entityId != 0) {
            return handleDialogueCondition(entityId, condition);
        }
        return false;
    });
    
    // Set up external action executor
    dialogueComponent->setExternalActionExecutor([this, dialogueComponent](const Components::DialogueAction& action) {
        // Find the entity ID for this dialogue component
        Components::EntityId entityId = 0;
        for (const auto& pair : m_dialogueComponents) {
            if (pair.second == dialogueComponent) {
                entityId = pair.first;
                break;
            }
        }
        
        if (entityId != 0) {
            return handleDialogueAction(entityId, action);
        }
        return false;
    });
    
    // Set up action executed callback to handle quest-related actions
    dialogueComponent->setActionExecutedCallback([this, dialogueComponent](const Components::DialogueAction& action) {
        // Find the entity ID for this dialogue component
        Components::EntityId entityId = 0;
        for (const auto& pair : m_dialogueComponents) {
            if (pair.second == dialogueComponent) {
                entityId = pair.first;
                break;
            }
        }
        
        if (entityId != 0) {
            handleDialogueAction(entityId, action);
        }
    });
}

void QuestDialogueIntegration::setupQuestDialogueCallbacks(std::shared_ptr<Components::QuestComponent> questComponent) {
    if (!questComponent) {
        return;
    }
    
    // Set up quest callbacks to trigger dialogue events
    questComponent->setQuestStartedCallback([this, questComponent](const std::string& questId) {
        // Find the entity ID for this quest component
        Components::EntityId entityId = 0;
        for (const auto& pair : m_questComponents) {
            if (pair.second == questComponent) {
                entityId = pair.first;
                break;
            }
        }
        
        if (entityId != 0) {
            triggerQuestEvent(entityId, "quest_started", questId);
        }
    });
    
    questComponent->setQuestCompletedCallback([this, questComponent](const std::string& questId) {
        Components::EntityId entityId = 0;
        for (const auto& pair : m_questComponents) {
            if (pair.second == questComponent) {
                entityId = pair.first;
                break;
            }
        }
        
        if (entityId != 0) {
            triggerQuestEvent(entityId, "quest_completed", questId);
        }
    });
    
    questComponent->setObjectiveCompletedCallback([this, questComponent](const std::string& questId, const std::string& objectiveId) {
        Components::EntityId entityId = 0;
        for (const auto& pair : m_questComponents) {
            if (pair.second == questComponent) {
                entityId = pair.first;
                break;
            }
        }
        
        if (entityId != 0) {
            triggerQuestEvent(entityId, "objective_completed", questId + ":" + objectiveId);
        }
    });
}

bool QuestDialogueIntegration::handleDialogueAction(Components::EntityId entityId, const Components::DialogueAction& action) {
    auto questComponent = getQuestComponent(entityId);
    if (!questComponent) {
        return false;
    }
    
    // Handle quest-related dialogue actions
    if (action.type == "start_quest") {
        return startQuestFromDialogue(entityId, action.target, action.value);
    }
    else if (action.type == "complete_quest") {
        return completeQuestFromDialogue(entityId, action.target);
    }
    else if (action.type == "update_objective") {
        // Parse objective ID and progress from parameters
        std::string objectiveId = action.value;
        int progress = 1;
        
        auto progressIt = action.parameters.find("progress");
        if (progressIt != action.parameters.end()) {
            progress = std::stoi(progressIt->second);
        }
        
        return updateQuestObjectiveFromDialogue(entityId, action.target, objectiveId, progress);
    }
    else if (action.type == "set_quest_variable") {
        // Parse variable key and value from parameters
        auto keyIt = action.parameters.find("key");
        auto valueIt = action.parameters.find("value");
        
        if (keyIt != action.parameters.end() && valueIt != action.parameters.end()) {
            return setQuestVariableFromDialogue(entityId, action.target, keyIt->second, valueIt->second);
        }
    }
    else if (action.type == "track_npc_interaction") {
        questComponent->trackNPCInteraction(action.target);
        return true;
    }
    else if (action.type == "track_location_visit") {
        questComponent->trackLocationVisit(action.target);
        return true;
    }
    else if (action.type == "track_custom_objective") {
        auto targetIt = action.parameters.find("target");
        auto countIt = action.parameters.find("count");
        
        std::string target = targetIt != action.parameters.end() ? targetIt->second : action.target;
        int count = countIt != action.parameters.end() ? std::stoi(countIt->second) : 1;
        
        questComponent->trackCustomObjective(action.type, target, count);
        return true;
    }
    
    return false;
}

bool QuestDialogueIntegration::handleDialogueCondition(Components::EntityId entityId, const Components::DialogueCondition& condition) {
    auto questComponent = getQuestComponent(entityId);
    if (!questComponent) {
        return false;
    }
    
    bool result = false;
    
    // Handle quest-related dialogue conditions
    if (condition.type == "quest_active") {
        result = isQuestActiveForDialogue(entityId, condition.target);
    }
    else if (condition.type == "quest_completed") {
        result = isQuestCompletedForDialogue(entityId, condition.target);
    }
    else if (condition.type == "objective_completed") {
        // Parse quest ID and objective ID from target (format: "questId:objectiveId")
        size_t colonPos = condition.target.find(':');
        if (colonPos != std::string::npos) {
            std::string questId = condition.target.substr(0, colonPos);
            std::string objectiveId = condition.target.substr(colonPos + 1);
            result = isObjectiveCompletedForDialogue(entityId, questId, objectiveId);
        }
    }
    else if (condition.type == "quest_variable") {
        // Parse quest ID and variable key from target (format: "questId:variableKey")
        size_t colonPos = condition.target.find(':');
        if (colonPos != std::string::npos) {
            std::string questId = condition.target.substr(0, colonPos);
            std::string variableKey = condition.target.substr(colonPos + 1);
            std::string variableValue = getQuestVariableForDialogue(entityId, questId, variableKey);
            
            // Compare with condition value
            if (condition.operation == "==") {
                result = (variableValue == condition.value);
            } else if (condition.operation == "!=") {
                result = (variableValue != condition.value);
            } else if (condition.operation == ">") {
                result = (std::stoi(variableValue) > std::stoi(condition.value));
            } else if (condition.operation == "<") {
                result = (std::stoi(variableValue) < std::stoi(condition.value));
            } else if (condition.operation == ">=") {
                result = (std::stoi(variableValue) >= std::stoi(condition.value));
            } else if (condition.operation == "<=") {
                result = (std::stoi(variableValue) <= std::stoi(condition.value));
            }
        }
    }
    
    // Apply negation if specified
    if (condition.negate) {
        result = !result;
    }
    
    return result;
}

bool QuestDialogueIntegration::startQuestFromDialogue(Components::EntityId entityId, const std::string& questId, const std::string& startedBy) {
    auto questComponent = getQuestComponent(entityId);
    if (!questComponent) {
        return false;
    }
    
    bool success = questComponent->startQuest(questId, startedBy);
    if (success) {
        std::cout << "Started quest " << questId << " from dialogue for entity " << entityId << std::endl;
    }
    
    return success;
}

bool QuestDialogueIntegration::completeQuestFromDialogue(Components::EntityId entityId, const std::string& questId) {
    auto questComponent = getQuestComponent(entityId);
    if (!questComponent) {
        return false;
    }
    
    bool success = questComponent->completeQuest(questId, true);
    if (success) {
        std::cout << "Completed quest " << questId << " from dialogue for entity " << entityId << std::endl;
    }
    
    return success;
}

bool QuestDialogueIntegration::updateQuestObjectiveFromDialogue(Components::EntityId entityId, const std::string& questId, 
                                                               const std::string& objectiveId, int progress) {
    auto questComponent = getQuestComponent(entityId);
    if (!questComponent) {
        return false;
    }
    
    bool success = questComponent->updateObjectiveProgress(questId, objectiveId, progress);
    if (success) {
        std::cout << "Updated objective " << objectiveId << " for quest " << questId 
                  << " with progress " << progress << " for entity " << entityId << std::endl;
    }
    
    return success;
}

bool QuestDialogueIntegration::setQuestVariableFromDialogue(Components::EntityId entityId, const std::string& questId, 
                                                           const std::string& key, const std::string& value) {
    auto questComponent = getQuestComponent(entityId);
    if (!questComponent) {
        return false;
    }
    
    questComponent->setQuestVariable(questId, key, value);
    std::cout << "Set quest variable " << key << " = " << value << " for quest " << questId 
              << " for entity " << entityId << std::endl;
    
    return true;
}

bool QuestDialogueIntegration::isQuestActiveForDialogue(Components::EntityId entityId, const std::string& questId) const {
    auto questComponent = getQuestComponent(entityId);
    if (!questComponent) {
        return false;
    }
    
    return questComponent->isQuestActive(questId);
}

bool QuestDialogueIntegration::isQuestCompletedForDialogue(Components::EntityId entityId, const std::string& questId) const {
    auto questComponent = getQuestComponent(entityId);
    if (!questComponent) {
        return false;
    }
    
    return questComponent->isQuestCompleted(questId);
}

bool QuestDialogueIntegration::isObjectiveCompletedForDialogue(Components::EntityId entityId, const std::string& questId, 
                                                              const std::string& objectiveId) const {
    auto questComponent = getQuestComponent(entityId);
    if (!questComponent) {
        return false;
    }
    
    return questComponent->isObjectiveCompleted(questId, objectiveId);
}

std::string QuestDialogueIntegration::getQuestVariableForDialogue(Components::EntityId entityId, const std::string& questId, 
                                                                 const std::string& key, const std::string& defaultValue) const {
    auto questComponent = getQuestComponent(entityId);
    if (!questComponent) {
        return defaultValue;
    }
    
    return questComponent->getQuestVariable(questId, key, defaultValue);
}

std::shared_ptr<Components::QuestComponent> QuestDialogueIntegration::getQuestComponent(Components::EntityId entityId) const {
    auto it = m_questComponents.find(entityId);
    return (it != m_questComponents.end()) ? it->second : nullptr;
}

std::shared_ptr<Components::DialogueComponent> QuestDialogueIntegration::getDialogueComponent(Components::EntityId entityId) const {
    auto it = m_dialogueComponents.find(entityId);
    return (it != m_dialogueComponents.end()) ? it->second : nullptr;
}

void QuestDialogueIntegration::triggerQuestEvent(Components::EntityId entityId, const std::string& eventType, const std::string& data) {
    if (m_questEventCallback) {
        m_questEventCallback(entityId, eventType, data);
    }
    
    std::cout << "Quest event: " << eventType << " for entity " << entityId << " with data: " << data << std::endl;
}

void QuestDialogueIntegration::triggerDialogueEvent(Components::EntityId entityId, const std::string& eventType, const std::string& data) {
    if (m_dialogueEventCallback) {
        m_dialogueEventCallback(entityId, eventType, data);
    }
    
    std::cout << "Dialogue event: " << eventType << " for entity " << entityId << " with data: " << data << std::endl;
}

void QuestDialogueIntegration::handleWorldEvent(const std::string& eventType, const std::string& eventData, Components::EntityId entityId) {
    // Handle world events that affect quests
    if (eventType == "map_transition") {
        // Track location visits for all quest components
        for (const auto& pair : m_questComponents) {
            auto questComponent = pair.second;
            if (questComponent) {
                questComponent->trackLocationVisit(eventData);
            }
        }
    }
    else if (eventType == "map_loaded") {
        // Track location visits when maps are loaded
        for (const auto& pair : m_questComponents) {
            auto questComponent = pair.second;
            if (questComponent) {
                questComponent->trackLocationVisit(eventData);
            }
        }
    }
    else if (eventType == "item_collected") {
        // Track item collection for all quest components
        for (const auto& pair : m_questComponents) {
            auto questComponent = pair.second;
            if (questComponent) {
                questComponent->trackItemCollection(eventData);
            }
        }
    }
    else if (eventType == "enemy_killed") {
        // Track kills for all quest components
        for (const auto& pair : m_questComponents) {
            auto questComponent = pair.second;
            if (questComponent) {
                questComponent->trackKill(eventData);
            }
        }
    }
    else if (eventType == "custom_objective") {
        // Parse custom objective data (format: "type:target:count")
        size_t firstColon = eventData.find(':');
        size_t secondColon = eventData.find(':', firstColon + 1);
        
        if (firstColon != std::string::npos && secondColon != std::string::npos) {
            std::string objectiveType = eventData.substr(0, firstColon);
            std::string target = eventData.substr(firstColon + 1, secondColon - firstColon - 1);
            int count = std::stoi(eventData.substr(secondColon + 1));
            
            for (const auto& pair : m_questComponents) {
                auto questComponent = pair.second;
                if (questComponent) {
                    questComponent->trackCustomObjective(objectiveType, target, count);
                }
            }
        }
    }
    
    // Forward to world event handler if set
    if (m_worldEventHandler) {
        m_worldEventHandler(eventType, eventData, entityId);
    }
    
    std::cout << "World event handled: " << eventType << " with data: " << eventData << std::endl;
}

} // namespace Systems
} // namespace RPGEngine