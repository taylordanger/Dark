#pragma once

#include "../graphics/SpriteRenderer.h"
#include "../input/InputManager.h"
#include "../systems/CombatSystem.h"
#include "../components/CombatComponent.h"
#include "../components/StatsComponent.h"
#include "../components/InventoryComponent.h"
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
 * Combat UI style configuration
 */
struct CombatUIStyle {
    // Main combat window
    float windowX = 50.0f;
    float windowY = 450.0f;
    float windowWidth = 700.0f;
    float windowHeight = 150.0f;
    Graphics::Color windowBackgroundColor = Graphics::Color(0.0f, 0.0f, 0.0f, 0.8f);
    Graphics::Color windowBorderColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    float windowBorderWidth = 2.0f;
    float windowPadding = 15.0f;
    
    // Action menu styling
    float actionMenuX = 100.0f;
    float actionMenuY = 300.0f;
    float actionMenuWidth = 200.0f;
    float actionMenuItemHeight = 30.0f;
    Graphics::Color actionMenuBackgroundColor = Graphics::Color(0.1f, 0.1f, 0.1f, 0.9f);
    Graphics::Color actionMenuBorderColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    Graphics::Color actionMenuSelectedColor = Graphics::Color(0.3f, 0.3f, 0.8f, 0.8f);
    Graphics::Color actionMenuTextColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    Graphics::Color actionMenuSelectedTextColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    float actionMenuTextSize = 14.0f;
    
    // Target selection styling
    float targetMenuX = 350.0f;
    float targetMenuY = 300.0f;
    float targetMenuWidth = 250.0f;
    float targetMenuItemHeight = 25.0f;
    Graphics::Color targetMenuBackgroundColor = Graphics::Color(0.1f, 0.1f, 0.1f, 0.9f);
    Graphics::Color targetMenuBorderColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    Graphics::Color targetMenuSelectedColor = Graphics::Color(0.8f, 0.3f, 0.3f, 0.8f);
    Graphics::Color targetMenuTextColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    Graphics::Color targetMenuSelectedTextColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    float targetMenuTextSize = 12.0f;
    
    // HP/MP bars styling
    float hpBarX = 20.0f;
    float hpBarY = 20.0f;
    float hpBarWidth = 200.0f;
    float hpBarHeight = 20.0f;
    float mpBarY = 45.0f;
    Graphics::Color hpBarBackgroundColor = Graphics::Color(0.3f, 0.0f, 0.0f, 0.8f);
    Graphics::Color hpBarForegroundColor = Graphics::Color(0.0f, 0.8f, 0.0f, 1.0f);
    Graphics::Color hpBarLowColor = Graphics::Color(0.8f, 0.8f, 0.0f, 1.0f);
    Graphics::Color hpBarCriticalColor = Graphics::Color(0.8f, 0.0f, 0.0f, 1.0f);
    Graphics::Color mpBarBackgroundColor = Graphics::Color(0.0f, 0.0f, 0.3f, 0.8f);
    Graphics::Color mpBarForegroundColor = Graphics::Color(0.0f, 0.0f, 0.8f, 1.0f);
    Graphics::Color barBorderColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    float barBorderWidth = 1.0f;
    
    // Turn order display
    float turnOrderX = 600.0f;
    float turnOrderY = 20.0f;
    float turnOrderWidth = 180.0f;
    float turnOrderItemHeight = 25.0f;
    Graphics::Color turnOrderBackgroundColor = Graphics::Color(0.0f, 0.0f, 0.0f, 0.7f);
    Graphics::Color turnOrderBorderColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    Graphics::Color turnOrderCurrentColor = Graphics::Color(1.0f, 1.0f, 0.0f, 1.0f);
    Graphics::Color turnOrderPlayerColor = Graphics::Color(0.0f, 0.8f, 0.0f, 1.0f);
    Graphics::Color turnOrderEnemyColor = Graphics::Color(0.8f, 0.0f, 0.0f, 1.0f);
    float turnOrderTextSize = 12.0f;
    
    // Status effects display
    float statusEffectsX = 20.0f;
    float statusEffectsY = 80.0f;
    float statusEffectIconSize = 24.0f;
    float statusEffectSpacing = 28.0f;
    Graphics::Color statusEffectBackgroundColor = Graphics::Color(0.0f, 0.0f, 0.0f, 0.6f);
    Graphics::Color statusEffectBorderColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    Graphics::Color statusEffectBeneficialColor = Graphics::Color(0.0f, 0.8f, 0.0f, 1.0f);
    Graphics::Color statusEffectHarmfulColor = Graphics::Color(0.8f, 0.0f, 0.0f, 1.0f);
    float statusEffectTextSize = 10.0f;
    
    // Damage numbers
    Graphics::Color damageTextColor = Graphics::Color(1.0f, 0.0f, 0.0f, 1.0f);
    Graphics::Color healingTextColor = Graphics::Color(0.0f, 1.0f, 0.0f, 1.0f);
    Graphics::Color criticalTextColor = Graphics::Color(1.0f, 1.0f, 0.0f, 1.0f);
    Graphics::Color missTextColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    float damageTextSize = 16.0f;
    float damageTextDuration = 2.0f;
    float damageTextSpeed = 50.0f;
    
    // Text styling
    Graphics::Color textColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    Graphics::Color highlightTextColor = Graphics::Color(1.0f, 1.0f, 0.0f, 1.0f);
    float textSize = 14.0f;
    float lineSpacing = 18.0f;
};

/**
 * Combat UI state enumeration
 */
enum class CombatUIState {
    Hidden,
    ActionSelection,
    SkillSelection,
    ItemSelection,
    TargetSelection,
    WaitingForAction,
    ShowingResults
};

/**
 * Combat action menu item
 */
struct CombatActionMenuItem {
    std::string id;
    std::string name;
    std::string description;
    bool enabled;
    Components::CombatActionType actionType;
    
    CombatActionMenuItem(const std::string& itemId, const std::string& itemName, 
                        Components::CombatActionType type, bool isEnabled = true)
        : id(itemId), name(itemName), enabled(isEnabled), actionType(type) {}
};

/**
 * Floating damage text
 */
struct FloatingDamageText {
    std::string text;
    float x, y;
    Graphics::Color color;
    float timeRemaining;
    float startY;
    
    FloatingDamageText(const std::string& txt, float posX, float posY, 
                      const Graphics::Color& textColor, float duration)
        : text(txt), x(posX), y(posY), color(textColor), timeRemaining(duration), startY(posY) {}
};

/**
 * Combat UI system
 * Handles combat interface, action selection, and visual feedback
 */
class CombatUI : public System {
public:
    /**
     * Constructor
     * @param spriteRenderer Sprite renderer for drawing
     * @param inputManager Input manager for handling input
     */
    CombatUI(std::shared_ptr<Graphics::SpriteRenderer> spriteRenderer,
             std::shared_ptr<Input::InputManager> inputManager);
    
    /**
     * Destructor
     */
    ~CombatUI();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Set combat system reference
     * @param combatSystem Combat system
     */
    void setCombatSystem(std::shared_ptr<Systems::CombatSystem> combatSystem);
    
    /**
     * Set player entity for UI display
     * @param playerEntity Player entity ID
     */
    void setPlayerEntity(EntityId playerEntity);
    
    /**
     * Show combat UI
     */
    void show();
    
    /**
     * Hide combat UI
     */
    void hide();
    
    /**
     * Check if combat UI is visible
     * @return true if visible
     */
    bool isVisible() const;
    
    /**
     * Set UI style
     * @param style New style configuration
     */
    void setStyle(const CombatUIStyle& style);
    
    /**
     * Get UI style
     * @return Current style configuration
     */
    const CombatUIStyle& getStyle() const { return m_style; }
    
    /**
     * Show damage text at position
     * @param damage Damage amount
     * @param x X position
     * @param y Y position
     * @param isCritical Whether damage was critical
     * @param isHealing Whether this is healing
     */
    void showDamageText(float damage, float x, float y, bool isCritical = false, bool isHealing = false);
    
    /**
     * Show miss text at position
     * @param x X position
     * @param y Y position
     */
    void showMissText(float x, float y);
    
    /**
     * Show status effect notification
     * @param effectName Effect name
     * @param applied Whether effect was applied or removed
     * @param x X position
     * @param y Y position
     */
    void showStatusEffectNotification(const std::string& effectName, bool applied, float x, float y);
    
    /**
     * Set action selected callback
     * @param callback Function called when action is selected
     */
    void setActionSelectedCallback(std::function<void(const Components::CombatAction&)> callback);
    
    /**
     * Set component manager for accessing entity components
     * @param componentManager Component manager
     */
    void setComponentManager(std::shared_ptr<ComponentManager> componentManager);
    
private:
    /**
     * Update input handling
     * @param deltaTime Time elapsed since last update
     */
    void updateInput(float deltaTime);
    
    /**
     * Update floating damage text
     * @param deltaTime Time elapsed since last update
     */
    void updateFloatingText(float deltaTime);
    
    /**
     * Render the combat UI
     */
    void render();
    
    /**
     * Render HP/MP bars for entity
     * @param entity Entity to render bars for
     * @param x X position
     * @param y Y position
     * @param width Bar width
     */
    void renderHealthBars(EntityId entity, float x, float y, float width);
    
    /**
     * Render status effects for entity
     * @param entity Entity to render effects for
     * @param x X position
     * @param y Y position
     */
    void renderStatusEffects(EntityId entity, float x, float y);
    
    /**
     * Render turn order display
     */
    void renderTurnOrder();
    
    /**
     * Render action selection menu
     */
    void renderActionMenu();
    
    /**
     * Render skill selection menu
     */
    void renderSkillMenu();
    
    /**
     * Render item selection menu
     */
    void renderItemMenu();
    
    /**
     * Render target selection menu
     */
    void renderTargetMenu();
    
    /**
     * Render floating damage text
     */
    void renderFloatingText();
    
    /**
     * Render a menu
     * @param items Menu items
     * @param selectedIndex Selected item index
     * @param x X position
     * @param y Y position
     * @param width Menu width
     * @param backgroundColor Background color
     * @param borderColor Border color
     * @param selectedColor Selected item color
     * @param textColor Text color
     * @param selectedTextColor Selected text color
     * @param textSize Text size
     * @param itemHeight Item height
     */
    void renderMenu(const std::vector<std::string>& items, int selectedIndex,
                   float x, float y, float width, 
                   const Graphics::Color& backgroundColor,
                   const Graphics::Color& borderColor,
                   const Graphics::Color& selectedColor,
                   const Graphics::Color& textColor,
                   const Graphics::Color& selectedTextColor,
                   float textSize, float itemHeight);
    
    /**
     * Render a rectangle
     * @param x X position
     * @param y Y position
     * @param width Width
     * @param height Height
     * @param color Color
     * @param filled Whether to fill the rectangle
     */
    void renderRectangle(float x, float y, float width, float height, 
                        const Graphics::Color& color, bool filled = true);
    
    /**
     * Render text
     * @param text Text to render
     * @param x X position
     * @param y Y position
     * @param color Text color
     * @param size Text size
     * @return Height of rendered text
     */
    float renderText(const std::string& text, float x, float y, 
                    const Graphics::Color& color, float size);
    
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
     * Handle action menu input
     */
    void handleActionMenuInput();
    
    /**
     * Handle skill menu input
     */
    void handleSkillMenuInput();
    
    /**
     * Handle item menu input
     */
    void handleItemMenuInput();
    
    /**
     * Handle target menu input
     */
    void handleTargetMenuInput();
    
    /**
     * Build action menu items
     */
    void buildActionMenu();
    
    /**
     * Build skill menu items
     */
    void buildSkillMenu();
    
    /**
     * Build item menu items
     */
    void buildItemMenu();
    
    /**
     * Build target menu items
     */
    void buildTargetMenu();
    
    /**
     * Execute selected action
     */
    void executeSelectedAction();
    
    /**
     * Get HP bar color based on percentage
     * @param hpPercentage HP percentage (0.0 to 1.0)
     * @return HP bar color
     */
    Graphics::Color getHPBarColor(float hpPercentage) const;
    
    /**
     * Get status effect color
     * @param effect Status effect
     * @return Effect color
     */
    Graphics::Color getStatusEffectColor(const Components::StatusEffect& effect) const;
    
    /**
     * Get entity position for UI display
     * @param entity Entity ID
     * @return Screen position
     */
    std::pair<float, float> getEntityScreenPosition(EntityId entity) const;
    
    // Dependencies
    std::shared_ptr<Graphics::SpriteRenderer> m_spriteRenderer;
    std::shared_ptr<Input::InputManager> m_inputManager;
    std::shared_ptr<Systems::CombatSystem> m_combatSystem;
    std::shared_ptr<ComponentManager> m_componentManager;
    
    // UI state
    CombatUIState m_state;
    CombatUIStyle m_style;
    bool m_visible;
    EntityId m_playerEntity;
    
    // Menu state
    int m_selectedActionIndex;
    int m_selectedSkillIndex;
    int m_selectedItemIndex;
    int m_selectedTargetIndex;
    
    // Menu items
    std::vector<CombatActionMenuItem> m_actionMenuItems;
    std::vector<const Components::CombatSkill*> m_skillMenuItems;
    std::vector<std::pair<std::string, int>> m_itemMenuItems; // itemId, quantity
    std::vector<EntityId> m_targetMenuItems;
    
    // Current action being built
    Components::CombatAction m_currentAction;
    
    // Visual effects
    std::vector<FloatingDamageText> m_floatingTexts;
    
    // Input timing
    float m_inputCooldown;
    static const float INPUT_COOLDOWN_TIME;
    
    // Callbacks
    std::function<void(const Components::CombatAction&)> m_actionSelectedCallback;
    
    // Input action names
    static const std::string ACTION_UP;
    static const std::string ACTION_DOWN;
    static const std::string ACTION_SELECT;
    static const std::string ACTION_CANCEL;
    static const std::string ACTION_MENU;
};

} // namespace UI
} // namespace RPGEngine