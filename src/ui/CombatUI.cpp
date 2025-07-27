#include "CombatUI.h"
#include "../components/ComponentManager.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace RPGEngine {
namespace UI {

// Static constants
const float CombatUI::INPUT_COOLDOWN_TIME = 0.15f;
const std::string CombatUI::ACTION_UP = "ui_up";
const std::string CombatUI::ACTION_DOWN = "ui_down";
const std::string CombatUI::ACTION_SELECT = "ui_select";
const std::string CombatUI::ACTION_CANCEL = "ui_cancel";
const std::string CombatUI::ACTION_MENU = "ui_menu";

CombatUI::CombatUI(std::shared_ptr<Graphics::SpriteRenderer> spriteRenderer,
                   std::shared_ptr<Input::InputManager> inputManager)
    : System("CombatUI")
    , m_spriteRenderer(spriteRenderer)
    , m_inputManager(inputManager)
    , m_state(CombatUIState::Hidden)
    , m_visible(false)
    , m_playerEntity(0)
    , m_selectedActionIndex(0)
    , m_selectedSkillIndex(0)
    , m_selectedItemIndex(0)
    , m_selectedTargetIndex(0)
    , m_currentAction(0, 0, Components::CombatActionType::Attack)
    , m_inputCooldown(0.0f)
{
    setPriority(800); // High priority for UI rendering
}

CombatUI::~CombatUI() = default;

bool CombatUI::onInitialize() {
    if (!m_spriteRenderer || !m_inputManager) {
        return false;
    }
    
    // Register input actions
    // Note: In a real implementation, these would be registered with the input manager
    
    return true;
}

void CombatUI::onUpdate(float deltaTime) {
    if (!m_visible) {
        return;
    }
    
    updateInput(deltaTime);
    updateFloatingText(deltaTime);
    render();
}

void CombatUI::onShutdown() {
    hide();
}

void CombatUI::setCombatSystem(std::shared_ptr<Systems::CombatSystem> combatSystem) {
    m_combatSystem = combatSystem;
}

void CombatUI::setPlayerEntity(EntityId playerEntity) {
    m_playerEntity = playerEntity;
}

void CombatUI::show() {
    m_visible = true;
    m_state = CombatUIState::ActionSelection;
    m_selectedActionIndex = 0;
    buildActionMenu();
}

void CombatUI::hide() {
    m_visible = false;
    m_state = CombatUIState::Hidden;
    m_floatingTexts.clear();
}

bool CombatUI::isVisible() const {
    return m_visible;
}

void CombatUI::setStyle(const CombatUIStyle& style) {
    m_style = style;
}

void CombatUI::showDamageText(float damage, float x, float y, bool isCritical, bool isHealing) {
    std::ostringstream oss;
    
    if (damage == 0.0f) {
        showMissText(x, y);
        return;
    }
    
    oss << std::fixed << std::setprecision(0) << damage;
    
    Graphics::Color color = isHealing ? m_style.healingTextColor : m_style.damageTextColor;
    if (isCritical) {
        color = m_style.criticalTextColor;
        oss << "!";
    }
    
    m_floatingTexts.emplace_back(oss.str(), x, y, color, m_style.damageTextDuration);
}

void CombatUI::showMissText(float x, float y) {
    m_floatingTexts.emplace_back("MISS", x, y, m_style.missTextColor, m_style.damageTextDuration);
}

void CombatUI::showStatusEffectNotification(const std::string& effectName, bool applied, float x, float y) {
    std::string text = applied ? ("+" + effectName) : ("-" + effectName);
    Graphics::Color color = applied ? m_style.statusEffectBeneficialColor : m_style.statusEffectHarmfulColor;
    
    m_floatingTexts.emplace_back(text, x, y, color, m_style.damageTextDuration);
}

void CombatUI::setActionSelectedCallback(std::function<void(const Components::CombatAction&)> callback) {
    m_actionSelectedCallback = callback;
}

void CombatUI::setComponentManager(std::shared_ptr<ComponentManager> componentManager) {
    m_componentManager = componentManager;
}

void CombatUI::updateInput(float deltaTime) {
    if (m_inputCooldown > 0.0f) {
        m_inputCooldown -= deltaTime;
        return;
    }
    
    switch (m_state) {
        case CombatUIState::ActionSelection:
            handleActionMenuInput();
            break;
        case CombatUIState::SkillSelection:
            handleSkillMenuInput();
            break;
        case CombatUIState::ItemSelection:
            handleItemMenuInput();
            break;
        case CombatUIState::TargetSelection:
            handleTargetMenuInput();
            break;
        default:
            break;
    }
}

void CombatUI::updateFloatingText(float deltaTime) {
    auto it = m_floatingTexts.begin();
    while (it != m_floatingTexts.end()) {
        it->timeRemaining -= deltaTime;
        it->y = it->startY - (m_style.damageTextDuration - it->timeRemaining) * m_style.damageTextSpeed;
        
        // Fade out over time
        float alpha = it->timeRemaining / m_style.damageTextDuration;
        it->color.a = alpha;
        
        if (it->timeRemaining <= 0.0f) {
            it = m_floatingTexts.erase(it);
        } else {
            ++it;
        }
    }
}

void CombatUI::render() {
    if (!m_visible || !m_combatSystem) {
        return;
    }
    
    // Render HP/MP bars for player
    if (m_playerEntity != 0) {
        renderHealthBars(m_playerEntity, m_style.hpBarX, m_style.hpBarY, m_style.hpBarWidth);
        renderStatusEffects(m_playerEntity, m_style.statusEffectsX, m_style.statusEffectsY);
    }
    
    // Render turn order
    renderTurnOrder();
    
    // Render current menu based on state
    switch (m_state) {
        case CombatUIState::ActionSelection:
            renderActionMenu();
            break;
        case CombatUIState::SkillSelection:
            renderSkillMenu();
            break;
        case CombatUIState::ItemSelection:
            renderItemMenu();
            break;
        case CombatUIState::TargetSelection:
            renderTargetMenu();
            break;
        default:
            break;
    }
    
    // Render floating damage text
    renderFloatingText();
}

void CombatUI::renderHealthBars(EntityId entity, float x, float y, float width) {
    if (!m_componentManager) {
        return;
    }
    
    auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(entity));
    if (!statsComp) {
        return;
    }
    
    // Render HP bar
    float hpPercentage = statsComp->getHPPercentage();
    Graphics::Color hpColor = getHPBarColor(hpPercentage);
    
    // HP bar background
    renderRectangle(x, y, width, m_style.hpBarHeight, m_style.hpBarBackgroundColor);
    
    // HP bar foreground
    renderRectangle(x, y, width * hpPercentage, m_style.hpBarHeight, hpColor);
    
    // HP bar border
    renderRectangle(x, y, width, m_style.hpBarHeight, m_style.barBorderColor, false);
    
    // HP text
    std::ostringstream hpText;
    hpText << "HP: " << static_cast<int>(statsComp->getCurrentHP()) 
           << "/" << static_cast<int>(statsComp->getMaxHP());
    renderText(hpText.str(), x + 5, y + 2, m_style.textColor, 12.0f);
    
    // Render MP bar
    float mpPercentage = statsComp->getMPPercentage();
    float mpY = y + m_style.hpBarHeight + 5;
    
    // MP bar background
    renderRectangle(x, mpY, width, m_style.hpBarHeight, m_style.mpBarBackgroundColor);
    
    // MP bar foreground
    renderRectangle(x, mpY, width * mpPercentage, m_style.hpBarHeight, m_style.mpBarForegroundColor);
    
    // MP bar border
    renderRectangle(x, mpY, width, m_style.hpBarHeight, m_style.barBorderColor, false);
    
    // MP text
    std::ostringstream mpText;
    mpText << "MP: " << static_cast<int>(statsComp->getCurrentMP()) 
           << "/" << static_cast<int>(statsComp->getMaxMP());
    renderText(mpText.str(), x + 5, mpY + 2, m_style.textColor, 12.0f);
}

void CombatUI::renderStatusEffects(EntityId entity, float x, float y) {
    if (!m_componentManager) {
        return;
    }
    
    auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(entity));
    if (!combatComp) {
        return;
    }
    
    const auto& effects = combatComp->getStatusEffects();
    float currentX = x;
    
    for (const auto& effect : effects) {
        Graphics::Color effectColor = getStatusEffectColor(effect);
        
        // Render effect icon background
        renderRectangle(currentX, y, m_style.statusEffectIconSize, m_style.statusEffectIconSize, 
                       m_style.statusEffectBackgroundColor);
        
        // Render effect border
        renderRectangle(currentX, y, m_style.statusEffectIconSize, m_style.statusEffectIconSize, 
                       effectColor, false);
        
        // Render effect name (abbreviated)
        std::string shortName = effect.name.length() > 3 ? effect.name.substr(0, 3) : effect.name;
        renderText(shortName, currentX + 2, y + 2, effectColor, m_style.statusEffectTextSize);
        
        // Show duration if not permanent
        if (effect.duration > 0.0f) {
            std::ostringstream durationText;
            durationText << static_cast<int>(effect.duration);
            renderText(durationText.str(), currentX + 2, y + 14, effectColor, 8.0f);
        }
        
        currentX += m_style.statusEffectSpacing;
    }
}

void CombatUI::renderTurnOrder() {
    if (!m_combatSystem) {
        return;
    }
    
    auto turnOrder = m_combatSystem->getTurnOrder();
    if (turnOrder.empty()) {
        return;
    }
    
    float x = m_style.turnOrderX;
    float y = m_style.turnOrderY;
    float width = m_style.turnOrderWidth;
    float height = turnOrder.size() * m_style.turnOrderItemHeight;
    
    // Render background
    renderRectangle(x, y, width, height, m_style.turnOrderBackgroundColor);
    renderRectangle(x, y, width, height, m_style.turnOrderBorderColor, false);
    
    // Render title
    renderText("Turn Order", x + 5, y + 5, m_style.textColor, m_style.turnOrderTextSize);
    
    float currentY = y + 25;
    const auto* currentParticipant = m_combatSystem->getCurrentTurnParticipant();
    
    for (const auto* participant : turnOrder) {
        Graphics::Color textColor = participant->isPlayer ? 
            m_style.turnOrderPlayerColor : m_style.turnOrderEnemyColor;
        
        // Highlight current turn
        if (currentParticipant && participant->entity == currentParticipant->entity) {
            renderRectangle(x + 2, currentY - 2, width - 4, m_style.turnOrderItemHeight, 
                           m_style.turnOrderCurrentColor);
            textColor = Graphics::Color(0.0f, 0.0f, 0.0f, 1.0f); // Black text on yellow background
        }
        
        // Render participant name
        std::string displayName = participant->name;
        if (participant->hasActed) {
            displayName += " (acted)";
        }
        
        renderText(displayName, x + 5, currentY, textColor, m_style.turnOrderTextSize);
        
        currentY += m_style.turnOrderItemHeight;
    }
}

void CombatUI::renderActionMenu() {
    std::vector<std::string> menuItems;
    for (const auto& item : m_actionMenuItems) {
        std::string displayName = item.name;
        if (!item.enabled) {
            displayName = "(" + displayName + ")";
        }
        menuItems.push_back(displayName);
    }
    
    renderMenu(menuItems, m_selectedActionIndex,
               m_style.actionMenuX, m_style.actionMenuY, m_style.actionMenuWidth,
               m_style.actionMenuBackgroundColor, m_style.actionMenuBorderColor,
               m_style.actionMenuSelectedColor, m_style.actionMenuTextColor,
               m_style.actionMenuSelectedTextColor, m_style.actionMenuTextSize,
               m_style.actionMenuItemHeight);
}

void CombatUI::renderSkillMenu() {
    std::vector<std::string> menuItems;
    for (const auto* skill : m_skillMenuItems) {
        std::ostringstream oss;
        oss << skill->name;
        if (skill->mpCost > 0) {
            oss << " (MP: " << static_cast<int>(skill->mpCost) << ")";
        }
        menuItems.push_back(oss.str());
    }
    
    renderMenu(menuItems, m_selectedSkillIndex,
               m_style.actionMenuX, m_style.actionMenuY, m_style.actionMenuWidth,
               m_style.actionMenuBackgroundColor, m_style.actionMenuBorderColor,
               m_style.actionMenuSelectedColor, m_style.actionMenuTextColor,
               m_style.actionMenuSelectedTextColor, m_style.actionMenuTextSize,
               m_style.actionMenuItemHeight);
}

void CombatUI::renderItemMenu() {
    std::vector<std::string> menuItems;
    for (const auto& item : m_itemMenuItems) {
        std::ostringstream oss;
        oss << item.first << " x" << item.second;
        menuItems.push_back(oss.str());
    }
    
    renderMenu(menuItems, m_selectedItemIndex,
               m_style.actionMenuX, m_style.actionMenuY, m_style.actionMenuWidth,
               m_style.actionMenuBackgroundColor, m_style.actionMenuBorderColor,
               m_style.actionMenuSelectedColor, m_style.actionMenuTextColor,
               m_style.actionMenuSelectedTextColor, m_style.actionMenuTextSize,
               m_style.actionMenuItemHeight);
}

void CombatUI::renderTargetMenu() {
    if (!m_combatSystem) {
        return;
    }
    
    std::vector<std::string> menuItems;
    for (EntityId target : m_targetMenuItems) {
        // Get target name and status
        std::string targetName = "Target " + std::to_string(target);
        
        if (m_componentManager) {
            auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(target));
            if (statsComp) {
                std::ostringstream oss;
                oss << targetName << " (HP: " << static_cast<int>(statsComp->getCurrentHP()) 
                    << "/" << static_cast<int>(statsComp->getMaxHP()) << ")";
                targetName = oss.str();
            }
        }
        
        menuItems.push_back(targetName);
    }
    
    renderMenu(menuItems, m_selectedTargetIndex,
               m_style.targetMenuX, m_style.targetMenuY, m_style.targetMenuWidth,
               m_style.targetMenuBackgroundColor, m_style.targetMenuBorderColor,
               m_style.targetMenuSelectedColor, m_style.targetMenuTextColor,
               m_style.targetMenuSelectedTextColor, m_style.targetMenuTextSize,
               m_style.targetMenuItemHeight);
}

void CombatUI::renderFloatingText() {
    for (const auto& text : m_floatingTexts) {
        renderText(text.text, text.x, text.y, text.color, m_style.damageTextSize);
    }
}

void CombatUI::renderMenu(const std::vector<std::string>& items, int selectedIndex,
                         float x, float y, float width,
                         const Graphics::Color& backgroundColor,
                         const Graphics::Color& borderColor,
                         const Graphics::Color& selectedColor,
                         const Graphics::Color& textColor,
                         const Graphics::Color& selectedTextColor,
                         float textSize, float itemHeight) {
    if (items.empty()) {
        return;
    }
    
    float height = items.size() * itemHeight;
    
    // Render background
    renderRectangle(x, y, width, height, backgroundColor);
    renderRectangle(x, y, width, height, borderColor, false);
    
    // Render items
    for (size_t i = 0; i < items.size(); ++i) {
        float itemY = y + i * itemHeight;
        
        // Highlight selected item
        if (static_cast<int>(i) == selectedIndex) {
            renderRectangle(x + 1, itemY + 1, width - 2, itemHeight - 2, selectedColor);
        }
        
        // Render item text
        Graphics::Color itemTextColor = (static_cast<int>(i) == selectedIndex) ? 
            selectedTextColor : textColor;
        renderText(items[i], x + 5, itemY + 5, itemTextColor, textSize);
    }
}

void CombatUI::renderRectangle(float x, float y, float width, float height, 
                              const Graphics::Color& color, bool filled) {
    // This is a placeholder implementation
    // In a real implementation, this would use the sprite renderer to draw rectangles
    // For now, we'll just log the rectangle drawing
}

float CombatUI::renderText(const std::string& text, float x, float y, 
                          const Graphics::Color& color, float size) {
    // This is a placeholder implementation
    // In a real implementation, this would use the sprite renderer to draw text
    // For now, we'll just return an estimated height
    return size * 1.2f;
}

float CombatUI::getTextWidth(const std::string& text, float size) {
    // Simple approximation: assume each character is 0.6 * size wide
    return text.length() * size * 0.6f;
}

float CombatUI::getTextHeight(float size) {
    return size * 1.2f;
}

void CombatUI::handleActionMenuInput() {
    if (!m_inputManager) {
        return;
    }
    
    // Handle up/down navigation
    if (m_inputManager->isActionJustActivated(ACTION_UP)) {
        m_selectedActionIndex = (m_selectedActionIndex - 1 + m_actionMenuItems.size()) % m_actionMenuItems.size();
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_DOWN)) {
        m_selectedActionIndex = (m_selectedActionIndex + 1) % m_actionMenuItems.size();
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_SELECT)) {
        if (m_selectedActionIndex < static_cast<int>(m_actionMenuItems.size())) {
            const auto& selectedItem = m_actionMenuItems[m_selectedActionIndex];
            
            if (!selectedItem.enabled) {
                m_inputCooldown = INPUT_COOLDOWN_TIME;
                return;
            }
            
            m_currentAction.type = selectedItem.actionType;
            m_currentAction.actor = m_playerEntity;
            
            switch (selectedItem.actionType) {
                case Components::CombatActionType::Attack:
                    m_state = CombatUIState::TargetSelection;
                    buildTargetMenu();
                    m_selectedTargetIndex = 0;
                    break;
                case Components::CombatActionType::Magic:
                case Components::CombatActionType::Skill:
                    m_state = CombatUIState::SkillSelection;
                    buildSkillMenu();
                    m_selectedSkillIndex = 0;
                    break;
                case Components::CombatActionType::Item:
                    m_state = CombatUIState::ItemSelection;
                    buildItemMenu();
                    m_selectedItemIndex = 0;
                    break;
                case Components::CombatActionType::Defend:
                    m_currentAction.target = m_playerEntity;
                    executeSelectedAction();
                    break;
                case Components::CombatActionType::Escape:
                    m_currentAction.target = 0;
                    executeSelectedAction();
                    break;
            }
        }
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_CANCEL)) {
        // Cancel combat action (if allowed)
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
}

void CombatUI::handleSkillMenuInput() {
    if (!m_inputManager) {
        return;
    }
    
    if (m_inputManager->isActionJustActivated(ACTION_UP)) {
        m_selectedSkillIndex = (m_selectedSkillIndex - 1 + m_skillMenuItems.size()) % m_skillMenuItems.size();
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_DOWN)) {
        m_selectedSkillIndex = (m_selectedSkillIndex + 1) % m_skillMenuItems.size();
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_SELECT)) {
        if (m_selectedSkillIndex < static_cast<int>(m_skillMenuItems.size())) {
            const auto* selectedSkill = m_skillMenuItems[m_selectedSkillIndex];
            m_currentAction.skillId = selectedSkill->id;
            
            // Check if player has enough MP
            if (m_componentManager) {
                auto statsComp = m_componentManager->getComponent<Components::StatsComponent>(Entity(m_playerEntity));
                if (statsComp && selectedSkill->mpCost > statsComp->getCurrentMP()) {
                    // Not enough MP - show feedback and return
                    m_inputCooldown = INPUT_COOLDOWN_TIME;
                    return;
                }
            }
            
            m_state = CombatUIState::TargetSelection;
            buildTargetMenu();
            m_selectedTargetIndex = 0;
        }
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_CANCEL)) {
        m_state = CombatUIState::ActionSelection;
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
}

void CombatUI::handleItemMenuInput() {
    if (!m_inputManager) {
        return;
    }
    
    if (m_inputManager->isActionJustActivated(ACTION_UP)) {
        m_selectedItemIndex = (m_selectedItemIndex - 1 + m_itemMenuItems.size()) % m_itemMenuItems.size();
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_DOWN)) {
        m_selectedItemIndex = (m_selectedItemIndex + 1) % m_itemMenuItems.size();
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_SELECT)) {
        if (m_selectedItemIndex < static_cast<int>(m_itemMenuItems.size())) {
            const auto& selectedItem = m_itemMenuItems[m_selectedItemIndex];
            m_currentAction.skillId = selectedItem.first; // Use item ID as skill ID
            
            m_state = CombatUIState::TargetSelection;
            buildTargetMenu();
            m_selectedTargetIndex = 0;
        }
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_CANCEL)) {
        m_state = CombatUIState::ActionSelection;
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
}

void CombatUI::handleTargetMenuInput() {
    if (!m_inputManager) {
        return;
    }
    
    if (m_inputManager->isActionJustActivated(ACTION_UP)) {
        m_selectedTargetIndex = (m_selectedTargetIndex - 1 + m_targetMenuItems.size()) % m_targetMenuItems.size();
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_DOWN)) {
        m_selectedTargetIndex = (m_selectedTargetIndex + 1) % m_targetMenuItems.size();
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_SELECT)) {
        if (m_selectedTargetIndex < static_cast<int>(m_targetMenuItems.size())) {
            m_currentAction.target = m_targetMenuItems[m_selectedTargetIndex];
            executeSelectedAction();
        }
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    } else if (m_inputManager->isActionJustActivated(ACTION_CANCEL)) {
        // Go back to previous menu based on action type
        switch (m_currentAction.type) {
            case Components::CombatActionType::Attack:
            case Components::CombatActionType::Defend:
                m_state = CombatUIState::ActionSelection;
                break;
            case Components::CombatActionType::Magic:
            case Components::CombatActionType::Skill:
                m_state = CombatUIState::SkillSelection;
                break;
            case Components::CombatActionType::Item:
                m_state = CombatUIState::ItemSelection;
                break;
            default:
                m_state = CombatUIState::ActionSelection;
                break;
        }
        m_inputCooldown = INPUT_COOLDOWN_TIME;
    }
}

void CombatUI::buildActionMenu() {
    m_actionMenuItems.clear();
    
    // Basic actions always available
    m_actionMenuItems.emplace_back("attack", "Attack", Components::CombatActionType::Attack);
    m_actionMenuItems.emplace_back("magic", "Magic", Components::CombatActionType::Magic);
    m_actionMenuItems.emplace_back("item", "Item", Components::CombatActionType::Item);
    m_actionMenuItems.emplace_back("defend", "Defend", Components::CombatActionType::Defend);
    m_actionMenuItems.emplace_back("escape", "Escape", Components::CombatActionType::Escape);
    
    // Check if actions are enabled based on player state
    if (m_componentManager && m_playerEntity != 0) {
        auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(m_playerEntity));
        auto inventoryComp = m_componentManager->getComponent<Components::InventoryComponent>(Entity(m_playerEntity));
        
        if (combatComp) {
            // Disable magic if silenced
            if (!combatComp->canUseMagic()) {
                for (auto& item : m_actionMenuItems) {
                    if (item.actionType == Components::CombatActionType::Magic) {
                        item.enabled = false;
                        break;
                    }
                }
            }
            
            // Check if player has any skills
            auto availableSkills = combatComp->getAvailableSkills();
            if (availableSkills.empty()) {
                for (auto& item : m_actionMenuItems) {
                    if (item.actionType == Components::CombatActionType::Magic) {
                        item.enabled = false;
                        break;
                    }
                }
            }
        }
        
        // Check if player has any usable items
        if (inventoryComp) {
            bool hasUsableItems = false;
            for (int i = 0; i < inventoryComp->getCapacity(); ++i) {
                const auto& slot = inventoryComp->getSlot(i);
                if (!slot.isEmpty) {
                    const auto* itemDef = Components::InventoryComponent::getItemDefinition(slot.item.itemId);
                    if (itemDef && itemDef->type == Components::ItemType::Consumable) {
                        hasUsableItems = true;
                        break;
                    }
                }
            }
            
            if (!hasUsableItems) {
                for (auto& item : m_actionMenuItems) {
                    if (item.actionType == Components::CombatActionType::Item) {
                        item.enabled = false;
                        break;
                    }
                }
            }
        }
    }
}

void CombatUI::buildSkillMenu() {
    m_skillMenuItems.clear();
    
    if (!m_componentManager || m_playerEntity == 0) {
        return;
    }
    
    auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(m_playerEntity));
    if (combatComp) {
        m_skillMenuItems = combatComp->getAvailableSkills();
    }
}

void CombatUI::buildItemMenu() {
    m_itemMenuItems.clear();
    
    if (!m_componentManager || m_playerEntity == 0) {
        return;
    }
    
    auto inventoryComp = m_componentManager->getComponent<Components::InventoryComponent>(Entity(m_playerEntity));
    if (inventoryComp) {
        for (int i = 0; i < inventoryComp->getCapacity(); ++i) {
            const auto& slot = inventoryComp->getSlot(i);
            if (!slot.isEmpty) {
                const auto* itemDef = Components::InventoryComponent::getItemDefinition(slot.item.itemId);
                if (itemDef && itemDef->type == Components::ItemType::Consumable) {
                    m_itemMenuItems.emplace_back(slot.item.itemId, slot.item.quantity);
                }
            }
        }
    }
}

void CombatUI::buildTargetMenu() {
    m_targetMenuItems.clear();
    
    if (!m_combatSystem) {
        return;
    }
    
    // Get valid targets based on current action
    if (m_currentAction.type == Components::CombatActionType::Attack ||
        m_currentAction.type == Components::CombatActionType::Magic ||
        !m_currentAction.skillId.empty()) {
        
        if (m_componentManager && m_playerEntity != 0) {
            auto combatComp = m_componentManager->getComponent<Components::CombatComponent>(Entity(m_playerEntity));
            if (combatComp) {
                if (!m_currentAction.skillId.empty()) {
                    const auto* skill = combatComp->getSkill(m_currentAction.skillId);
                    if (skill) {
                        m_targetMenuItems = m_combatSystem->getValidTargets(m_playerEntity, *skill);
                    }
                } else {
                    // Basic attack - target enemies
                    const auto* encounter = m_combatSystem->getCurrentEncounter();
                    if (encounter) {
                        for (const auto& participant : encounter->participants) {
                            if (!participant.isPlayer && participant.isAlive) {
                                m_targetMenuItems.push_back(participant.entity);
                            }
                        }
                    }
                }
            }
        }
    } else if (m_currentAction.type == Components::CombatActionType::Item) {
        // Items can target self or allies typically
        const auto* encounter = m_combatSystem->getCurrentEncounter();
        if (encounter) {
            for (const auto& participant : encounter->participants) {
                if (participant.isPlayer && participant.isAlive) {
                    m_targetMenuItems.push_back(participant.entity);
                }
            }
        }
    }
}

void CombatUI::executeSelectedAction() {
    if (m_actionSelectedCallback) {
        m_actionSelectedCallback(m_currentAction);
    }
    
    // Return to action selection for next turn
    m_state = CombatUIState::ActionSelection;
    m_selectedActionIndex = 0;
}

Graphics::Color CombatUI::getHPBarColor(float hpPercentage) const {
    if (hpPercentage <= 0.25f) {
        return m_style.hpBarCriticalColor;
    } else if (hpPercentage <= 0.5f) {
        return m_style.hpBarLowColor;
    } else {
        return m_style.hpBarForegroundColor;
    }
}

Graphics::Color CombatUI::getStatusEffectColor(const Components::StatusEffect& effect) const {
    return effect.beneficial ? m_style.statusEffectBeneficialColor : m_style.statusEffectHarmfulColor;
}

std::pair<float, float> CombatUI::getEntityScreenPosition(EntityId entity) const {
    // This would typically get the entity's world position and convert to screen coordinates
    // For now, return a default position
    return std::make_pair(400.0f, 300.0f);
}

} // namespace UI
} // namespace RPGEngine