#pragma once

#include "UIRenderer.h"
#include "../systems/System.h"
#include "../components/StatsComponent.h"
#include "../components/InventoryComponent.h"
#include "../components/ComponentManager.h"
#include "../core/Types.h"
#include <memory>
#include <string>
#include <vector>

namespace RPGEngine {
namespace UI {

/**
 * HUD element visibility flags
 */
struct HUDVisibility {
    bool showHealthBar = true;
    bool showManaBar = true;
    bool showExperienceBar = true;
    bool showMinimap = false;
    bool showInventoryQuickSlots = true;
    bool showQuestTracker = true;
    bool showStatusEffects = true;
    bool showClock = false;
};

/**
 * Quick slot item information
 */
struct QuickSlotItem {
    std::string itemId;
    std::string itemName;
    int quantity;
    bool usable;
    
    QuickSlotItem() : quantity(0), usable(false) {}
    QuickSlotItem(const std::string& id, const std::string& name, int qty, bool canUse)
        : itemId(id), itemName(name), quantity(qty), usable(canUse) {}
};

/**
 * Active quest information for HUD display
 */
struct HUDQuestInfo {
    std::string questId;
    std::string questName;
    std::string currentObjective;
    int completedObjectives;
    int totalObjectives;
    
    HUDQuestInfo() : completedObjectives(0), totalObjectives(0) {}
};

/**
 * Status effect information for HUD display
 */
struct HUDStatusEffect {
    std::string effectId;
    std::string effectName;
    float remainingTime;
    bool isBeneficial;
    
    HUDStatusEffect() : remainingTime(0.0f), isBeneficial(true) {}
    HUDStatusEffect(const std::string& id, const std::string& name, float time, bool beneficial)
        : effectId(id), effectName(name), remainingTime(time), isBeneficial(beneficial) {}
};

/**
 * Game HUD System
 * Handles the heads-up display during gameplay
 */
class GameHUD : public System {
public:
    /**
     * Constructor
     * @param uiRenderer UI renderer for drawing
     */
    GameHUD(std::shared_ptr<UIRenderer> uiRenderer);
    
    /**
     * Destructor
     */
    ~GameHUD();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Show HUD
     */
    void show();
    
    /**
     * Hide HUD
     */
    void hide();
    
    /**
     * Check if HUD is visible
     * @return true if visible
     */
    bool isVisible() const { return m_visible; }
    
    /**
     * Set player entity for HUD display
     * @param playerEntity Player entity ID
     */
    void setPlayerEntity(EntityId playerEntity) { m_playerEntity = playerEntity; }
    
    /**
     * Set HUD visibility options
     * @param visibility Visibility settings
     */
    void setVisibility(const HUDVisibility& visibility) { m_visibility = visibility; }
    
    /**
     * Get HUD visibility options
     * @return Current visibility settings
     */
    const HUDVisibility& getVisibility() const { return m_visibility; }
    
    /**
     * Set quick slot items
     * @param quickSlots Quick slot items
     */
    void setQuickSlots(const std::vector<QuickSlotItem>& quickSlots) { m_quickSlots = quickSlots; }
    
    /**
     * Set active quest information
     * @param questInfo Active quest information
     */
    void setActiveQuest(const HUDQuestInfo& questInfo) { m_activeQuest = questInfo; }
    
    /**
     * Set status effects
     * @param statusEffects Current status effects
     */
    void setStatusEffects(const std::vector<HUDStatusEffect>& statusEffects) { m_statusEffects = statusEffects; }
    
    /**
     * Set component manager for accessing entity components
     * @param componentManager Component manager
     */
    void setComponentManager(std::shared_ptr<ComponentManager> componentManager) { m_componentManager = componentManager; }
    
private:
    /**
     * Render health bar
     */
    void renderHealthBar();
    
    /**
     * Render mana bar
     */
    void renderManaBar();
    
    /**
     * Render experience bar
     */
    void renderExperienceBar();
    
    /**
     * Render minimap
     */
    void renderMinimap();
    
    /**
     * Render inventory quick slots
     */
    void renderQuickSlots();
    
    /**
     * Render quest tracker
     */
    void renderQuestTracker();
    
    /**
     * Render status effects
     */
    void renderStatusEffects();
    
    /**
     * Render clock
     */
    void renderClock();
    
    /**
     * Get player stats component
     * @return Player stats component or nullptr
     */
    std::shared_ptr<Components::StatsComponent> getPlayerStats();
    
    /**
     * Get player inventory component
     * @return Player inventory component or nullptr
     */
    std::shared_ptr<Components::InventoryComponent> getPlayerInventory();
    
    /**
     * Format time string
     * @param seconds Time in seconds
     * @return Formatted time string (MM:SS)
     */
    std::string formatTime(float seconds);
    
    /**
     * Get health bar color based on percentage
     * @param healthPercentage Health percentage (0.0 to 1.0)
     * @return Health bar color
     */
    Graphics::Color getHealthBarColor(float healthPercentage);
    
    /**
     * Get mana bar color
     * @return Mana bar color
     */
    Graphics::Color getManaBarColor();
    
    /**
     * Get experience bar color
     * @return Experience bar color
     */
    Graphics::Color getExperienceBarColor();
    
    // Dependencies
    std::shared_ptr<UIRenderer> m_uiRenderer;
    std::shared_ptr<ComponentManager> m_componentManager;
    
    // HUD state
    bool m_visible;
    EntityId m_playerEntity;
    HUDVisibility m_visibility;
    
    // HUD data
    std::vector<QuickSlotItem> m_quickSlots;
    HUDQuestInfo m_activeQuest;
    std::vector<HUDStatusEffect> m_statusEffects;
    
    // Screen dimensions (would normally come from graphics system)
    float m_screenWidth;
    float m_screenHeight;
    
    // HUD layout constants
    static const float HEALTH_BAR_X;
    static const float HEALTH_BAR_Y;
    static const float HEALTH_BAR_WIDTH;
    static const float HEALTH_BAR_HEIGHT;
    
    static const float MANA_BAR_X;
    static const float MANA_BAR_Y;
    static const float MANA_BAR_WIDTH;
    static const float MANA_BAR_HEIGHT;
    
    static const float EXP_BAR_X;
    static const float EXP_BAR_Y;
    static const float EXP_BAR_WIDTH;
    static const float EXP_BAR_HEIGHT;
    
    static const float QUICK_SLOT_SIZE;
    static const float QUICK_SLOT_SPACING;
    static const float QUICK_SLOT_START_X;
    static const float QUICK_SLOT_Y;
    
    static const float QUEST_TRACKER_X;
    static const float QUEST_TRACKER_Y;
    static const float QUEST_TRACKER_WIDTH;
    static const float QUEST_TRACKER_HEIGHT;
    
    static const float STATUS_EFFECT_SIZE;
    static const float STATUS_EFFECT_SPACING;
    static const float STATUS_EFFECT_START_X;
    static const float STATUS_EFFECT_Y;
    
    static const int MAX_QUICK_SLOTS;
    static const int MAX_STATUS_EFFECTS;
};

} // namespace UI
} // namespace RPGEngine