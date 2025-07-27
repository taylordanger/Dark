#include "GameHUD.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace RPGEngine {
namespace UI {

// Static constants
const float GameHUD::HEALTH_BAR_X = 20.0f;
const float GameHUD::HEALTH_BAR_Y = 20.0f;
const float GameHUD::HEALTH_BAR_WIDTH = 200.0f;
const float GameHUD::HEALTH_BAR_HEIGHT = 20.0f;

const float GameHUD::MANA_BAR_X = 20.0f;
const float GameHUD::MANA_BAR_Y = 45.0f;
const float GameHUD::MANA_BAR_WIDTH = 200.0f;
const float GameHUD::MANA_BAR_HEIGHT = 20.0f;

const float GameHUD::EXP_BAR_X = 20.0f;
const float GameHUD::EXP_BAR_Y = 70.0f;
const float GameHUD::EXP_BAR_WIDTH = 200.0f;
const float GameHUD::EXP_BAR_HEIGHT = 15.0f;

const float GameHUD::QUICK_SLOT_SIZE = 40.0f;
const float GameHUD::QUICK_SLOT_SPACING = 45.0f;
const float GameHUD::QUICK_SLOT_START_X = 300.0f;
const float GameHUD::QUICK_SLOT_Y = 20.0f;

const float GameHUD::QUEST_TRACKER_X = 20.0f;
const float GameHUD::QUEST_TRACKER_Y = 120.0f;
const float GameHUD::QUEST_TRACKER_WIDTH = 250.0f;
const float GameHUD::QUEST_TRACKER_HEIGHT = 150.0f;

const float GameHUD::STATUS_EFFECT_SIZE = 24.0f;
const float GameHUD::STATUS_EFFECT_SPACING = 28.0f;
const float GameHUD::STATUS_EFFECT_START_X = 20.0f;
const float GameHUD::STATUS_EFFECT_Y = 95.0f;

const int GameHUD::MAX_QUICK_SLOTS = 8;
const int GameHUD::MAX_STATUS_EFFECTS = 10;

GameHUD::GameHUD(std::shared_ptr<UIRenderer> uiRenderer)
    : m_uiRenderer(uiRenderer)
    , m_visible(false)
    , m_playerEntity(INVALID_ENTITY_ID)
    , m_screenWidth(800.0f)
    , m_screenHeight(600.0f)
{
    // Initialize quick slots
    m_quickSlots.resize(MAX_QUICK_SLOTS);
}

GameHUD::~GameHUD() {
}

bool GameHUD::onInitialize() {
    return true;
}

void GameHUD::onUpdate(float deltaTime) {
    if (!m_visible) return;
    
    m_uiRenderer->beginFrame();
    
    // Render HUD elements based on visibility settings
    if (m_visibility.showHealthBar) {
        renderHealthBar();
    }
    
    if (m_visibility.showManaBar) {
        renderManaBar();
    }
    
    if (m_visibility.showExperienceBar) {
        renderExperienceBar();
    }
    
    if (m_visibility.showMinimap) {
        renderMinimap();
    }
    
    if (m_visibility.showInventoryQuickSlots) {
        renderQuickSlots();
    }
    
    if (m_visibility.showQuestTracker) {
        renderQuestTracker();
    }
    
    if (m_visibility.showStatusEffects) {
        renderStatusEffects();
    }
    
    if (m_visibility.showClock) {
        renderClock();
    }
    
    m_uiRenderer->endFrame();
}

void GameHUD::onShutdown() {
}

void GameHUD::show() {
    m_visible = true;
}

void GameHUD::hide() {
    m_visible = false;
}

void GameHUD::renderHealthBar() {
    auto playerStats = getPlayerStats();
    if (!playerStats) return;
    
    float healthPercentage = static_cast<float>(playerStats->getCurrentHP()) / static_cast<float>(playerStats->getMaxHP());
    healthPercentage = std::max(0.0f, std::min(1.0f, healthPercentage));
    
    // Draw health bar background
    UIRect healthBarBounds(HEALTH_BAR_X, HEALTH_BAR_Y, HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT);
    
    // Draw health bar
    m_uiRenderer->drawProgressBar(healthBarBounds, healthPercentage, 0.0f, 1.0f, "health_bar");
    
    // Draw health text
    std::string healthText = std::to_string(playerStats->getCurrentHP()) + " / " + std::to_string(playerStats->getMaxHP());
    UIRect healthTextBounds(HEALTH_BAR_X + 5, HEALTH_BAR_Y + 2, HEALTH_BAR_WIDTH - 10, HEALTH_BAR_HEIGHT - 4);
    m_uiRenderer->drawText(healthTextBounds, healthText, UIAlignment::Center, 12.0f, "health_text");
    
    // Draw HP label
    UIRect hpLabelBounds(HEALTH_BAR_X, HEALTH_BAR_Y - 15, 30, 12);
    m_uiRenderer->drawText(hpLabelBounds, "HP", UIAlignment::TopLeft, 12.0f, "hp_label");
}

void GameHUD::renderManaBar() {
    auto playerStats = getPlayerStats();
    if (!playerStats) return;
    
    float manaPercentage = static_cast<float>(playerStats->getCurrentMP()) / static_cast<float>(playerStats->getMaxMP());
    manaPercentage = std::max(0.0f, std::min(1.0f, manaPercentage));
    
    // Draw mana bar background
    UIRect manaBarBounds(MANA_BAR_X, MANA_BAR_Y, MANA_BAR_WIDTH, MANA_BAR_HEIGHT);
    
    // Draw mana bar
    m_uiRenderer->drawProgressBar(manaBarBounds, manaPercentage, 0.0f, 1.0f, "mana_bar");
    
    // Draw mana text
    std::string manaText = std::to_string(playerStats->getCurrentMP()) + " / " + std::to_string(playerStats->getMaxMP());
    UIRect manaTextBounds(MANA_BAR_X + 5, MANA_BAR_Y + 2, MANA_BAR_WIDTH - 10, MANA_BAR_HEIGHT - 4);
    m_uiRenderer->drawText(manaTextBounds, manaText, UIAlignment::Center, 12.0f, "mana_text");
    
    // Draw MP label
    UIRect mpLabelBounds(MANA_BAR_X, MANA_BAR_Y - 15, 30, 12);
    m_uiRenderer->drawText(mpLabelBounds, "MP", UIAlignment::TopLeft, 12.0f, "mp_label");
}

void GameHUD::renderExperienceBar() {
    auto playerStats = getPlayerStats();
    if (!playerStats) return;
    
    float expPercentage = static_cast<float>(playerStats->getCurrentExperience()) / static_cast<float>(playerStats->getExperienceToNextLevel());
    expPercentage = std::max(0.0f, std::min(1.0f, expPercentage));
    
    // Draw experience bar
    UIRect expBarBounds(EXP_BAR_X, EXP_BAR_Y, EXP_BAR_WIDTH, EXP_BAR_HEIGHT);
    m_uiRenderer->drawProgressBar(expBarBounds, expPercentage, 0.0f, 1.0f, "exp_bar");
    
    // Draw level text
    std::string levelText = "Level " + std::to_string(playerStats->getLevel());
    UIRect levelTextBounds(EXP_BAR_X + EXP_BAR_WIDTH + 10, EXP_BAR_Y, 80, EXP_BAR_HEIGHT);
    m_uiRenderer->drawText(levelTextBounds, levelText, UIAlignment::CenterLeft, 12.0f, "level_text");
}

void GameHUD::renderMinimap() {
    // Minimap placeholder
    float minimapSize = 120.0f;
    float minimapX = m_screenWidth - minimapSize - 20.0f;
    float minimapY = 20.0f;
    
    UIRect minimapBounds(minimapX, minimapY, minimapSize, minimapSize);
    m_uiRenderer->drawPanel(minimapBounds, "minimap_panel");
    
    // Draw minimap title
    UIRect minimapTitleBounds(minimapX, minimapY - 15, minimapSize, 12);
    m_uiRenderer->drawText(minimapTitleBounds, "Minimap", UIAlignment::Center, 12.0f, "minimap_title");
    
    // Draw player position (center dot)
    float playerDotSize = 4.0f;
    float playerDotX = minimapX + (minimapSize - playerDotSize) * 0.5f;
    float playerDotY = minimapY + (minimapSize - playerDotSize) * 0.5f;
    
    UIRect playerDotBounds(playerDotX, playerDotY, playerDotSize, playerDotSize);
    m_uiRenderer->drawPanel(playerDotBounds, "player_dot");
}

void GameHUD::renderQuickSlots() {
    for (int i = 0; i < MAX_QUICK_SLOTS && i < static_cast<int>(m_quickSlots.size()); ++i) {
        float slotX = QUICK_SLOT_START_X + i * QUICK_SLOT_SPACING;
        UIRect slotBounds(slotX, QUICK_SLOT_Y, QUICK_SLOT_SIZE, QUICK_SLOT_SIZE);
        
        // Draw slot background
        m_uiRenderer->drawPanel(slotBounds, "quick_slot_" + std::to_string(i));
        
        const auto& item = m_quickSlots[i];
        if (!item.itemId.empty()) {
            // Draw item (placeholder - would normally draw item icon)
            UIRect itemBounds(slotX + 2, QUICK_SLOT_Y + 2, QUICK_SLOT_SIZE - 4, QUICK_SLOT_SIZE - 4);
            m_uiRenderer->drawPanel(itemBounds, "item_" + std::to_string(i));
            
            // Draw quantity if > 1
            if (item.quantity > 1) {
                std::string quantityText = std::to_string(item.quantity);
                UIRect quantityBounds(slotX + QUICK_SLOT_SIZE - 15, QUICK_SLOT_Y + QUICK_SLOT_SIZE - 15, 12, 12);
                m_uiRenderer->drawText(quantityBounds, quantityText, UIAlignment::Center, 10.0f, "quantity_" + std::to_string(i));
            }
        }
        
        // Draw slot number
        std::string slotNumber = std::to_string(i + 1);
        UIRect numberBounds(slotX, QUICK_SLOT_Y - 15, QUICK_SLOT_SIZE, 12);
        m_uiRenderer->drawText(numberBounds, slotNumber, UIAlignment::Center, 10.0f, "slot_number_" + std::to_string(i));
    }
}

void GameHUD::renderQuestTracker() {
    if (m_activeQuest.questId.empty()) return;
    
    // Draw quest tracker panel
    UIRect trackerBounds(QUEST_TRACKER_X, QUEST_TRACKER_Y, QUEST_TRACKER_WIDTH, QUEST_TRACKER_HEIGHT);
    m_uiRenderer->drawPanel(trackerBounds, "quest_tracker_panel");
    
    // Draw quest title
    UIRect titleBounds(QUEST_TRACKER_X + 10, QUEST_TRACKER_Y + 10, QUEST_TRACKER_WIDTH - 20, 20);
    m_uiRenderer->drawText(titleBounds, m_activeQuest.questName, UIAlignment::TopLeft, 14.0f, "quest_title");
    
    // Draw current objective
    UIRect objectiveBounds(QUEST_TRACKER_X + 10, QUEST_TRACKER_Y + 35, QUEST_TRACKER_WIDTH - 20, 60);
    m_uiRenderer->drawText(objectiveBounds, m_activeQuest.currentObjective, UIAlignment::TopLeft, 12.0f, "quest_objective");
    
    // Draw progress
    std::string progressText = "Progress: " + std::to_string(m_activeQuest.completedObjectives) + 
                              " / " + std::to_string(m_activeQuest.totalObjectives);
    UIRect progressBounds(QUEST_TRACKER_X + 10, QUEST_TRACKER_Y + 100, QUEST_TRACKER_WIDTH - 20, 15);
    m_uiRenderer->drawText(progressBounds, progressText, UIAlignment::TopLeft, 11.0f, "quest_progress");
    
    // Draw progress bar
    float progressPercentage = (m_activeQuest.totalObjectives > 0) ? 
        static_cast<float>(m_activeQuest.completedObjectives) / static_cast<float>(m_activeQuest.totalObjectives) : 0.0f;
    
    UIRect progressBarBounds(QUEST_TRACKER_X + 10, QUEST_TRACKER_Y + 120, QUEST_TRACKER_WIDTH - 20, 10);
    m_uiRenderer->drawProgressBar(progressBarBounds, progressPercentage, 0.0f, 1.0f, "quest_progress_bar");
}

void GameHUD::renderStatusEffects() {
    int effectCount = std::min(static_cast<int>(m_statusEffects.size()), MAX_STATUS_EFFECTS);
    
    for (int i = 0; i < effectCount; ++i) {
        float effectX = STATUS_EFFECT_START_X + i * STATUS_EFFECT_SPACING;
        UIRect effectBounds(effectX, STATUS_EFFECT_Y, STATUS_EFFECT_SIZE, STATUS_EFFECT_SIZE);
        
        const auto& effect = m_statusEffects[i];
        
        // Draw effect background (different color for beneficial/harmful)
        m_uiRenderer->drawPanel(effectBounds, "status_effect_" + std::to_string(i));
        
        // Draw remaining time
        if (effect.remainingTime > 0.0f) {
            std::string timeText = formatTime(effect.remainingTime);
            UIRect timeBounds(effectX, STATUS_EFFECT_Y + STATUS_EFFECT_SIZE + 2, STATUS_EFFECT_SIZE, 10);
            m_uiRenderer->drawText(timeBounds, timeText, UIAlignment::Center, 8.0f, "effect_time_" + std::to_string(i));
        }
    }
}

void GameHUD::renderClock() {
    // Game time placeholder (would normally come from game time system)
    static float gameTime = 0.0f;
    gameTime += 0.016f; // Simulate time passing
    
    std::string timeText = formatTime(gameTime);
    
    UIRect clockBounds(m_screenWidth - 80, m_screenHeight - 30, 70, 20);
    m_uiRenderer->drawText(clockBounds, timeText, UIAlignment::CenterRight, 12.0f, "game_clock");
}

std::shared_ptr<Components::StatsComponent> GameHUD::getPlayerStats() {
    if (m_playerEntity == INVALID_ENTITY_ID || !m_componentManager) {
        return nullptr;
    }
    
    return m_componentManager->getComponent<Components::StatsComponent>(m_playerEntity);
}

std::shared_ptr<Components::InventoryComponent> GameHUD::getPlayerInventory() {
    if (m_playerEntity == INVALID_ENTITY_ID || !m_componentManager) {
        return nullptr;
    }
    
    return m_componentManager->getComponent<Components::InventoryComponent>(m_playerEntity);
}

std::string GameHUD::formatTime(float seconds) {
    int totalSeconds = static_cast<int>(seconds);
    int minutes = totalSeconds / 60;
    int secs = totalSeconds % 60;
    
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << minutes << ":" 
        << std::setfill('0') << std::setw(2) << secs;
    return oss.str();
}

Graphics::Color GameHUD::getHealthBarColor(float healthPercentage) {
    if (healthPercentage > 0.6f) {
        return Graphics::Color(0.0f, 0.8f, 0.0f, 1.0f); // Green
    } else if (healthPercentage > 0.3f) {
        return Graphics::Color(0.8f, 0.8f, 0.0f, 1.0f); // Yellow
    } else {
        return Graphics::Color(0.8f, 0.0f, 0.0f, 1.0f); // Red
    }
}

Graphics::Color GameHUD::getManaBarColor() {
    return Graphics::Color(0.0f, 0.0f, 0.8f, 1.0f); // Blue
}

Graphics::Color GameHUD::getExperienceBarColor() {
    return Graphics::Color(0.8f, 0.0f, 0.8f, 1.0f); // Purple
}

} // namespace UI
} // namespace RPGEngine