#pragma once

#include "../graphics/SpriteRenderer.h"
#include "../input/InputManager.h"
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
 * UI element types
 */
enum class UIElementType {
    Panel,
    Button,
    Text,
    Image,
    ProgressBar,
    Checkbox,
    Slider
};

/**
 * UI element state
 */
enum class UIElementState {
    Normal,
    Hovered,
    Pressed,
    Disabled
};

/**
 * UI layout alignment
 */
enum class UIAlignment {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

/**
 * UI style configuration
 */
struct UIStyle {
    // Panel styling
    Graphics::Color panelBackgroundColor = Graphics::Color(0.2f, 0.2f, 0.2f, 0.8f);
    Graphics::Color panelBorderColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    float panelBorderWidth = 1.0f;
    float panelPadding = 10.0f;
    
    // Button styling
    Graphics::Color buttonNormalColor = Graphics::Color(0.3f, 0.3f, 0.3f, 1.0f);
    Graphics::Color buttonHoverColor = Graphics::Color(0.4f, 0.4f, 0.4f, 1.0f);
    Graphics::Color buttonPressedColor = Graphics::Color(0.2f, 0.2f, 0.2f, 1.0f);
    Graphics::Color buttonDisabledColor = Graphics::Color(0.1f, 0.1f, 0.1f, 0.5f);
    Graphics::Color buttonBorderColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    Graphics::Color buttonTextColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    Graphics::Color buttonDisabledTextColor = Graphics::Color(0.5f, 0.5f, 0.5f, 1.0f);
    float buttonBorderWidth = 1.0f;
    float buttonPadding = 8.0f;
    
    // Text styling
    Graphics::Color textColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    Graphics::Color textDisabledColor = Graphics::Color(0.5f, 0.5f, 0.5f, 1.0f);
    float textSize = 14.0f;
    float lineSpacing = 18.0f;
    
    // Progress bar styling
    Graphics::Color progressBarBackgroundColor = Graphics::Color(0.2f, 0.2f, 0.2f, 0.8f);
    Graphics::Color progressBarForegroundColor = Graphics::Color(0.0f, 0.8f, 0.0f, 1.0f);
    Graphics::Color progressBarBorderColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    float progressBarBorderWidth = 1.0f;
    
    // Checkbox styling
    Graphics::Color checkboxBackgroundColor = Graphics::Color(0.3f, 0.3f, 0.3f, 1.0f);
    Graphics::Color checkboxCheckedColor = Graphics::Color(0.0f, 0.8f, 0.0f, 1.0f);
    Graphics::Color checkboxBorderColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    float checkboxBorderWidth = 1.0f;
    float checkboxSize = 16.0f;
    
    // Slider styling
    Graphics::Color sliderTrackColor = Graphics::Color(0.2f, 0.2f, 0.2f, 0.8f);
    Graphics::Color sliderHandleColor = Graphics::Color(0.6f, 0.6f, 0.6f, 1.0f);
    Graphics::Color sliderHandleHoverColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    Graphics::Color sliderBorderColor = Graphics::Color(0.8f, 0.8f, 0.8f, 1.0f);
    float sliderBorderWidth = 1.0f;
    float sliderTrackHeight = 4.0f;
    float sliderHandleSize = 12.0f;
};

/**
 * UI rectangle structure
 */
struct UIRect {
    float x, y, width, height;
    
    UIRect() : x(0), y(0), width(0), height(0) {}
    UIRect(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}
    
    bool contains(float px, float py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

/**
 * UI element base structure
 */
struct UIElement {
    UIElementType type;
    UIRect bounds;
    UIElementState state;
    bool visible;
    bool enabled;
    std::string id;
    
    UIElement(UIElementType elementType, const std::string& elementId = "")
        : type(elementType), state(UIElementState::Normal), visible(true), enabled(true), id(elementId) {}
};

/**
 * UI button element
 */
struct UIButton : public UIElement {
    std::string text;
    std::function<void()> onClick;
    
    UIButton(const std::string& buttonText, const std::string& buttonId = "")
        : UIElement(UIElementType::Button, buttonId), text(buttonText) {}
};

/**
 * UI text element
 */
struct UIText : public UIElement {
    std::string text;
    float fontSize;
    UIAlignment alignment;
    
    UIText(const std::string& textContent, const std::string& textId = "")
        : UIElement(UIElementType::Text, textId), text(textContent), fontSize(14.0f), alignment(UIAlignment::TopLeft) {}
};

/**
 * UI panel element
 */
struct UIPanel : public UIElement {
    std::vector<std::shared_ptr<UIElement>> children;
    
    UIPanel(const std::string& panelId = "")
        : UIElement(UIElementType::Panel, panelId) {}
};

/**
 * UI progress bar element
 */
struct UIProgressBar : public UIElement {
    float value;
    float minValue;
    float maxValue;
    
    UIProgressBar(const std::string& progressId = "")
        : UIElement(UIElementType::ProgressBar, progressId), value(0.0f), minValue(0.0f), maxValue(1.0f) {}
};

/**
 * UI checkbox element
 */
struct UICheckbox : public UIElement {
    bool checked;
    std::string label;
    std::function<void(bool)> onChanged;
    
    UICheckbox(const std::string& checkboxLabel, const std::string& checkboxId = "")
        : UIElement(UIElementType::Checkbox, checkboxId), checked(false), label(checkboxLabel) {}
};

/**
 * UI slider element
 */
struct UISlider : public UIElement {
    float value;
    float minValue;
    float maxValue;
    bool isDragging;
    std::function<void(float)> onChanged;
    
    UISlider(const std::string& sliderId = "")
        : UIElement(UIElementType::Slider, sliderId), value(0.0f), minValue(0.0f), maxValue(1.0f), isDragging(false) {}
};

/**
 * UI layout manager
 */
class UILayout {
public:
    /**
     * Calculate aligned position within bounds
     * @param bounds Container bounds
     * @param elementWidth Element width
     * @param elementHeight Element height
     * @param alignment Alignment type
     * @return Aligned position
     */
    static std::pair<float, float> calculateAlignedPosition(const UIRect& bounds, float elementWidth, float elementHeight, UIAlignment alignment);
    
    /**
     * Layout elements vertically
     * @param elements Elements to layout
     * @param bounds Container bounds
     * @param spacing Spacing between elements
     */
    static void layoutVertical(std::vector<std::shared_ptr<UIElement>>& elements, const UIRect& bounds, float spacing = 5.0f);
    
    /**
     * Layout elements horizontally
     * @param elements Elements to layout
     * @param bounds Container bounds
     * @param spacing Spacing between elements
     */
    static void layoutHorizontal(std::vector<std::shared_ptr<UIElement>>& elements, const UIRect& bounds, float spacing = 5.0f);
    
    /**
     * Layout elements in a grid
     * @param elements Elements to layout
     * @param bounds Container bounds
     * @param columns Number of columns
     * @param spacing Spacing between elements
     */
    static void layoutGrid(std::vector<std::shared_ptr<UIElement>>& elements, const UIRect& bounds, int columns, float spacing = 5.0f);
};

/**
 * UI Renderer - Immediate Mode GUI System
 * Handles rendering and interaction for UI elements
 */
class UIRenderer : public System {
public:
    /**
     * Constructor
     * @param spriteRenderer Sprite renderer for drawing
     * @param inputManager Input manager for handling input
     */
    UIRenderer(std::shared_ptr<Graphics::SpriteRenderer> spriteRenderer,
               std::shared_ptr<Input::InputManager> inputManager);
    
    /**
     * Destructor
     */
    ~UIRenderer();
    
    // System interface implementation
    bool onInitialize() override;
    void onUpdate(float deltaTime) override;
    void onShutdown() override;
    
    /**
     * Begin UI frame
     * Call this at the start of UI rendering
     */
    void beginFrame();
    
    /**
     * End UI frame
     * Call this at the end of UI rendering
     */
    void endFrame();
    
    /**
     * Set UI style
     * @param style New style configuration
     */
    void setStyle(const UIStyle& style);
    
    /**
     * Get UI style
     * @return Current style configuration
     */
    const UIStyle& getStyle() const { return m_style; }
    
    // Immediate mode UI functions
    
    /**
     * Draw a panel
     * @param bounds Panel bounds
     * @param id Panel ID
     * @return Panel element
     */
    std::shared_ptr<UIPanel> drawPanel(const UIRect& bounds, const std::string& id = "");
    
    /**
     * Draw a button
     * @param bounds Button bounds
     * @param text Button text
     * @param id Button ID
     * @return true if button was clicked
     */
    bool drawButton(const UIRect& bounds, const std::string& text, const std::string& id = "");
    
    /**
     * Draw text
     * @param bounds Text bounds
     * @param text Text content
     * @param alignment Text alignment
     * @param fontSize Font size
     * @param id Text ID
     */
    void drawText(const UIRect& bounds, const std::string& text, UIAlignment alignment = UIAlignment::TopLeft, 
                  float fontSize = 0.0f, const std::string& id = "");
    
    /**
     * Draw a progress bar
     * @param bounds Progress bar bounds
     * @param value Current value
     * @param minValue Minimum value
     * @param maxValue Maximum value
     * @param id Progress bar ID
     */
    void drawProgressBar(const UIRect& bounds, float value, float minValue = 0.0f, float maxValue = 1.0f, const std::string& id = "");
    
    /**
     * Draw a checkbox
     * @param bounds Checkbox bounds
     * @param label Checkbox label
     * @param checked Current checked state
     * @param id Checkbox ID
     * @return New checked state
     */
    bool drawCheckbox(const UIRect& bounds, const std::string& label, bool checked, const std::string& id = "");
    
    /**
     * Draw a slider
     * @param bounds Slider bounds
     * @param value Current value
     * @param minValue Minimum value
     * @param maxValue Maximum value
     * @param id Slider ID
     * @return New value
     */
    float drawSlider(const UIRect& bounds, float value, float minValue = 0.0f, float maxValue = 1.0f, const std::string& id = "");
    
    /**
     * Draw an image
     * @param bounds Image bounds
     * @param texture Texture to draw
     * @param color Tint color
     * @param id Image ID
     */
    void drawImage(const UIRect& bounds, std::shared_ptr<Graphics::Texture> texture, 
                   const Graphics::Color& color = Graphics::Color::White, const std::string& id = "");
    
    /**
     * Check if mouse is over element
     * @param bounds Element bounds
     * @return true if mouse is over element
     */
    bool isMouseOver(const UIRect& bounds) const;
    
    /**
     * Check if mouse button was clicked
     * @return true if left mouse button was clicked this frame
     */
    bool isMouseClicked() const;
    
    /**
     * Check if mouse button is pressed
     * @return true if left mouse button is currently pressed
     */
    bool isMousePressed() const;
    
    /**
     * Get mouse position
     * @return Mouse position
     */
    std::pair<float, float> getMousePosition() const;
    
private:
    /**
     * Update input state
     * @param deltaTime Time elapsed since last update
     */
    void updateInput(float deltaTime);
    
    /**
     * Render a rectangle
     * @param bounds Rectangle bounds
     * @param color Rectangle color
     * @param filled Whether to fill the rectangle
     * @param borderWidth Border width (if not filled)
     * @param borderColor Border color (if not filled)
     */
    void renderRectangle(const UIRect& bounds, const Graphics::Color& color, bool filled = true, 
                        float borderWidth = 1.0f, const Graphics::Color& borderColor = Graphics::Color::White);
    
    /**
     * Render text
     * @param text Text to render
     * @param x X position
     * @param y Y position
     * @param color Text color
     * @param fontSize Font size
     * @param alignment Text alignment
     * @param bounds Bounds for alignment calculation
     * @return Height of rendered text
     */
    float renderText(const std::string& text, float x, float y, const Graphics::Color& color, 
                    float fontSize, UIAlignment alignment = UIAlignment::TopLeft, const UIRect* bounds = nullptr);
    
    /**
     * Get text dimensions (approximation)
     * @param text Text to measure
     * @param fontSize Font size
     * @return Text dimensions (width, height)
     */
    std::pair<float, float> getTextDimensions(const std::string& text, float fontSize);
    
    /**
     * Get element state based on interaction
     * @param bounds Element bounds
     * @param enabled Whether element is enabled
     * @return Element state
     */
    UIElementState getElementState(const UIRect& bounds, bool enabled = true);
    
    /**
     * Get color for element state
     * @param normalColor Normal state color
     * @param hoverColor Hover state color
     * @param pressedColor Pressed state color
     * @param disabledColor Disabled state color
     * @param state Current element state
     * @return Color for current state
     */
    Graphics::Color getStateColor(const Graphics::Color& normalColor, const Graphics::Color& hoverColor,
                                 const Graphics::Color& pressedColor, const Graphics::Color& disabledColor,
                                 UIElementState state);
    
    // Dependencies
    std::shared_ptr<Graphics::SpriteRenderer> m_spriteRenderer;
    std::shared_ptr<Input::InputManager> m_inputManager;
    
    // UI state
    UIStyle m_style;
    bool m_frameActive;
    
    // Input state
    float m_mouseX;
    float m_mouseY;
    bool m_mousePressed;
    bool m_mouseClicked;
    bool m_previousMousePressed;
    
    // Element tracking
    std::vector<std::shared_ptr<UIElement>> m_elements;
    std::string m_hotElementId;
    std::string m_activeElementId;
    
    // Input action names
    static const std::string ACTION_MOUSE_LEFT;
    static const std::string ACTION_MOUSE_RIGHT;
};

} // namespace UI
} // namespace RPGEngine