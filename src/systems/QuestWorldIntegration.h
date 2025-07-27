#pragma once

#include "../core/Event.h"
#include "../world/WorldManager.h"
#include "../systems/QuestSystem.h"
#include "../systems/QuestDialogueIntegration.h"
#include "../ui/QuestUI.h"
#include <memory>
#include <functional>
#include <vector>

namespace RPGEngine {
namespace Systems {

/**
 * Quest World Integration System
 * Handles integration between quest system and world events
 */
class QuestWorldIntegration {
public:
    /**
     * Constructor
     * @param questSystem Quest system reference
     * @param worldManager World manager reference
     * @param questDialogueIntegration Quest-dialogue integration reference
     */
    QuestWorldIntegration(std::shared_ptr<QuestSystem> questSystem,
                         std::shared_ptr<World::WorldManager> worldManager,
                         std::shared_ptr<QuestDialogueIntegration> questDialogueIntegration);
    
    /**
     * Destructor
     */
    ~QuestWorldIntegration();
    
    /**
     * Initialize integration
     */
    bool initialize();
    
    /**
     * Shutdown integration
     */
    void shutdown();
    
    /**
     * Set quest UI for notifications
     * @param questUI Quest UI reference
     */
    void setQuestUI(std::shared_ptr<UI::QuestUI> questUI) {
        m_questUI = questUI;
    }
    
    /**
     * Set event dispatcher for world events
     * @param eventDispatcher Event dispatcher reference
     */
    void setEventDispatcher(std::shared_ptr<EventDispatcher> eventDispatcher);
    
    /**
     * Handle map transition event
     * @param event Map transition event
     */
    void handleMapTransition(const World::MapTransitionEvent& event);
    
    /**
     * Handle map loaded event
     * @param event Map loaded event
     */
    void handleMapLoaded(const World::MapLoadedEvent& event);
    
    /**
     * Handle map unloaded event
     * @param event Map unloaded event
     */
    void handleMapUnloaded(const World::MapUnloadedEvent& event);
    
    /**
     * Trigger world event for quest tracking
     * @param eventType Event type
     * @param eventData Event data
     * @param entityId Entity ID (optional)
     */
    void triggerWorldEvent(const std::string& eventType, const std::string& eventData, 
                          Components::EntityId entityId = 0);
    
    /**
     * Register custom world event handler
     * @param eventType Event type to handle
     * @param handler Event handler function
     */
    void registerCustomEventHandler(const std::string& eventType, 
                                   std::function<void(const std::string&, Components::EntityId)> handler);
    
    /**
     * Unregister custom world event handler
     * @param eventType Event type
     */
    void unregisterCustomEventHandler(const std::string& eventType);
    
    /**
     * Set global quest event callback
     * @param callback Function called for quest events
     */
    void setGlobalQuestEventCallback(std::function<void(Components::EntityId, const std::string&, const std::string&)> callback) {
        m_globalQuestEventCallback = callback;
    }
    
private:
    /**
     * Setup world event subscriptions
     */
    void setupWorldEventSubscriptions();
    
    /**
     * Cleanup world event subscriptions
     */
    void cleanupWorldEventSubscriptions();
    
    /**
     * Handle custom world event
     * @param eventType Event type
     * @param eventData Event data
     * @param entityId Entity ID
     */
    void handleCustomWorldEvent(const std::string& eventType, const std::string& eventData, 
                               Components::EntityId entityId);
    
    // System references
    std::shared_ptr<QuestSystem> m_questSystem;
    std::shared_ptr<World::WorldManager> m_worldManager;
    std::shared_ptr<QuestDialogueIntegration> m_questDialogueIntegration;
    std::shared_ptr<UI::QuestUI> m_questUI;
    std::shared_ptr<EventDispatcher> m_eventDispatcher;
    
    // Event subscriptions
    std::vector<size_t> m_eventSubscriptions;
    
    // Custom event handlers
    std::unordered_map<std::string, std::function<void(const std::string&, Components::EntityId)>> m_customEventHandlers;
    
    // Global callback
    std::function<void(Components::EntityId, const std::string&, const std::string&)> m_globalQuestEventCallback;
    
    bool m_initialized;
};

} // namespace Systems
} // namespace RPGEngine