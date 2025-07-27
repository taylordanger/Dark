#include "QuestWorldIntegration.h"
#include <iostream>
#include <algorithm>

namespace RPGEngine {
namespace Systems {

QuestWorldIntegration::QuestWorldIntegration(std::shared_ptr<QuestSystem> questSystem,
                                           std::shared_ptr<World::WorldManager> worldManager,
                                           std::shared_ptr<QuestDialogueIntegration> questDialogueIntegration)
    : m_questSystem(questSystem)
    , m_worldManager(worldManager)
    , m_questDialogueIntegration(questDialogueIntegration)
    , m_questUI(nullptr)
    , m_eventDispatcher(nullptr)
    , m_initialized(false)
{
}

QuestWorldIntegration::~QuestWorldIntegration() {
    shutdown();
}

bool QuestWorldIntegration::initialize() {
    if (m_initialized) {
        return true;
    }
    
    if (!m_questSystem || !m_worldManager || !m_questDialogueIntegration) {
        std::cerr << "QuestWorldIntegration: Required systems not provided" << std::endl;
        return false;
    }
    
    // Set up world event handler in quest-dialogue integration
    m_questDialogueIntegration->setWorldEventHandler(
        [this](const std::string& eventType, const std::string& eventData, Components::EntityId entityId) {
            handleCustomWorldEvent(eventType, eventData, entityId);
        }
    );
    
    // Set up global quest callbacks
    m_questSystem->setGlobalQuestStartedCallback(
        [this](Components::EntityId entityId, const std::string& questId) {
            if (m_questUI) {
                m_questUI->showQuestStartedNotification(questId);
                m_questUI->trackQuest(questId); // Auto-track new quests
            }
            
            if (m_globalQuestEventCallback) {
                m_globalQuestEventCallback(entityId, "quest_started", questId);
            }
        }
    );
    
    m_questSystem->setGlobalQuestCompletedCallback(
        [this](Components::EntityId entityId, const std::string& questId) {
            if (m_questUI) {
                m_questUI->showQuestCompletedNotification(questId);
                m_questUI->untrackQuest(questId); // Remove completed quests from tracking
            }
            
            if (m_globalQuestEventCallback) {
                m_globalQuestEventCallback(entityId, "quest_completed", questId);
            }
        }
    );
    
    m_questSystem->setGlobalQuestFailedCallback(
        [this](Components::EntityId entityId, const std::string& questId, const std::string& reason) {
            if (m_questUI) {
                m_questUI->showQuestFailedNotification(questId, reason);
                m_questUI->untrackQuest(questId); // Remove failed quests from tracking
            }
            
            if (m_globalQuestEventCallback) {
                m_globalQuestEventCallback(entityId, "quest_failed", questId + ":" + reason);
            }
        }
    );
    
    m_questSystem->setGlobalObjectiveCompletedCallback(
        [this](Components::EntityId entityId, const std::string& questId, const std::string& objectiveId) {
            if (m_questUI) {
                m_questUI->showObjectiveCompletedNotification(questId, objectiveId);
            }
            
            if (m_globalQuestEventCallback) {
                m_globalQuestEventCallback(entityId, "objective_completed", questId + ":" + objectiveId);
            }
        }
    );
    
    m_questSystem->setGlobalRewardGivenCallback(
        [this](Components::EntityId entityId, const Components::QuestReward& reward) {
            if (m_questUI) {
                m_questUI->showRewardReceivedNotification(reward);
            }
            
            if (m_globalQuestEventCallback) {
                m_globalQuestEventCallback(entityId, "reward_given", reward.type + ":" + reward.target + ":" + std::to_string(reward.amount));
            }
        }
    );
    
    // Set up world event subscriptions if event dispatcher is available
    if (m_eventDispatcher) {
        setupWorldEventSubscriptions();
    }
    
    std::cout << "QuestWorldIntegration initialized successfully" << std::endl;
    m_initialized = true;
    return true;
}

void QuestWorldIntegration::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    cleanupWorldEventSubscriptions();
    m_customEventHandlers.clear();
    
    std::cout << "QuestWorldIntegration shutdown" << std::endl;
    m_initialized = false;
}

void QuestWorldIntegration::setEventDispatcher(std::shared_ptr<EventDispatcher> eventDispatcher) {
    if (m_eventDispatcher != eventDispatcher) {
        // Clean up old subscriptions
        cleanupWorldEventSubscriptions();
        
        m_eventDispatcher = eventDispatcher;
        
        // Set up new subscriptions if initialized
        if (m_initialized && m_eventDispatcher) {
            setupWorldEventSubscriptions();
        }
    }
}

void QuestWorldIntegration::handleMapTransition(const World::MapTransitionEvent& event) {
    std::cout << "Quest system handling map transition from " << event.fromMapId 
              << " to " << event.toMapId << " via " << event.portalName << std::endl;
    
    // Trigger world event for quest tracking
    triggerWorldEvent("map_transition", std::to_string(event.toMapId));
    
    // Track location visit for portal
    if (!event.portalName.empty()) {
        triggerWorldEvent("location_visit", event.portalName);
    }
}

void QuestWorldIntegration::handleMapLoaded(const World::MapLoadedEvent& event) {
    std::cout << "Quest system handling map loaded: " << event.mapName << " (ID: " << event.mapId << ")" << std::endl;
    
    // Trigger world event for quest tracking
    triggerWorldEvent("map_loaded", event.mapName);
    triggerWorldEvent("location_visit", event.mapName);
}

void QuestWorldIntegration::handleMapUnloaded(const World::MapUnloadedEvent& event) {
    std::cout << "Quest system handling map unloaded: " << event.mapName << " (ID: " << event.mapId << ")" << std::endl;
    
    // Trigger world event for quest tracking
    triggerWorldEvent("map_unloaded", event.mapName);
}

void QuestWorldIntegration::triggerWorldEvent(const std::string& eventType, const std::string& eventData, 
                                             Components::EntityId entityId) {
    // Forward to quest-dialogue integration
    if (m_questDialogueIntegration) {
        m_questDialogueIntegration->handleWorldEvent(eventType, eventData, entityId);
    }
    
    // Handle custom events
    handleCustomWorldEvent(eventType, eventData, entityId);
    
    std::cout << "World event triggered: " << eventType << " with data: " << eventData << std::endl;
}

void QuestWorldIntegration::registerCustomEventHandler(const std::string& eventType, 
                                                      std::function<void(const std::string&, Components::EntityId)> handler) {
    m_customEventHandlers[eventType] = handler;
    std::cout << "Registered custom event handler for: " << eventType << std::endl;
}

void QuestWorldIntegration::unregisterCustomEventHandler(const std::string& eventType) {
    auto it = m_customEventHandlers.find(eventType);
    if (it != m_customEventHandlers.end()) {
        m_customEventHandlers.erase(it);
        std::cout << "Unregistered custom event handler for: " << eventType << std::endl;
    }
}

void QuestWorldIntegration::setupWorldEventSubscriptions() {
    if (!m_eventDispatcher) {
        return;
    }
    
    // Subscribe to map transition events
    auto transitionSub = m_eventDispatcher->subscribe<World::MapTransitionEvent>(
        [this](const World::MapTransitionEvent& event) {
            handleMapTransition(event);
        }
    );
    m_eventSubscriptions.push_back(transitionSub);
    
    // Subscribe to map loaded events
    auto loadedSub = m_eventDispatcher->subscribe<World::MapLoadedEvent>(
        [this](const World::MapLoadedEvent& event) {
            handleMapLoaded(event);
        }
    );
    m_eventSubscriptions.push_back(loadedSub);
    
    // Subscribe to map unloaded events
    auto unloadedSub = m_eventDispatcher->subscribe<World::MapUnloadedEvent>(
        [this](const World::MapUnloadedEvent& event) {
            handleMapUnloaded(event);
        }
    );
    m_eventSubscriptions.push_back(unloadedSub);
    
    std::cout << "Set up world event subscriptions" << std::endl;
}

void QuestWorldIntegration::cleanupWorldEventSubscriptions() {
    if (!m_eventDispatcher) {
        return;
    }
    
    // Unsubscribe from all events
    for (size_t subscriptionId : m_eventSubscriptions) {
        m_eventDispatcher->unsubscribe<World::MapTransitionEvent>(subscriptionId);
        m_eventDispatcher->unsubscribe<World::MapLoadedEvent>(subscriptionId);
        m_eventDispatcher->unsubscribe<World::MapUnloadedEvent>(subscriptionId);
    }
    
    m_eventSubscriptions.clear();
    std::cout << "Cleaned up world event subscriptions" << std::endl;
}

void QuestWorldIntegration::handleCustomWorldEvent(const std::string& eventType, const std::string& eventData, 
                                                  Components::EntityId entityId) {
    // Handle custom events
    auto it = m_customEventHandlers.find(eventType);
    if (it != m_customEventHandlers.end()) {
        it->second(eventData, entityId);
    }
    
    // Handle specific world events for quest progress notifications
    if (eventType == "item_collected" && m_questUI) {
        // Show progress notification for item collection
        // This would need to be enhanced to show actual quest progress
        // For now, just log the event
        std::cout << "Item collected for quest tracking: " << eventData << std::endl;
    }
    else if (eventType == "enemy_killed" && m_questUI) {
        // Show progress notification for enemy kills
        std::cout << "Enemy killed for quest tracking: " << eventData << std::endl;
    }
    else if (eventType == "location_visit" && m_questUI) {
        // Show progress notification for location visits
        std::cout << "Location visited for quest tracking: " << eventData << std::endl;
    }
}

} // namespace Systems
} // namespace RPGEngine