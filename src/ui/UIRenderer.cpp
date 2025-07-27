#include "UIRenderer.h"
#include <algorithm>
#include <cmath>

namespace RPGEngine {
namespace UI {

// Static constants
const std::string UIRenderer::ACTION_MOUSE_LEFT = "mouse_left";
const std::string UIRenderer::ACTION_MOUSE_RIGHT = "mouse_right";

UIRenderer::UIRenderer(std::shared_ptr<Graphics::SpriteRenderer> spriteRenderer,
                       std::shared_ptr<Input::InputManager> inputManager)
    : m_spriteRenderer(spriteRenderer)
    , m_inputManager(inputManager)
    , m_frameActive(false)
    , m_mouseX(0.0f)
    , m_mouseY(0.0f)
    , m_mousePressed(false)
    , m_mouseClicked(false)
    , m_previousMousePressed(false)
{
}

UIRenderer::~UIRenderer() {
}

bool UIRenderer::onInitialize() {
    return true;
}

void UIRenderer::onUpdate(float deltaTime) {
    updateInput(deltaTime);
}

void UIRenderer::onShutdown() {
    m_elements.clear();
}

void UIRenderer::beginFrame() {
    m_frameActive = true;
    m_elements.clear();
    m_hotElementId.clear();
}

void UIRenderer::endFrame() {
    m_frameActive = false;
}

void UIRenderer::setStyle(const UIStyle& style) {
    m_style = style;
}

std::shared_ptr<UIPanel> UIRenderer::drawPanel(const UIRect& bounds, const std::string& id) {
    auto panel = std::make_shared<UIPanel>(id);
    panel->bounds = bounds;
    
    // Render panel background
    renderRectangle(bounds, m_style.panelBackgroundColor, true);
    
    // Render panel border
    if (m_style.panelBorderWidth > 0.0f) {
        renderRectangle(bounds, m_style.panelBorderColor, false, m_style.panelBorderWidth, m_style.panelBorderColor);
    }
    
    m_elements.push_back(panel);
    return panel;
}

bool UIRenderer::drawButton(const UIRect& bounds, const std::string& text, const std::string& id) {
    auto button = std::make_shared<UIButton>(text, id);
    button->bounds = bounds;
    button->state = getElementState(bounds, button->enabled);
    
    // Get button colors based on state
    Graphics::Color backgroundColor = getStateColor(
        m_style.buttonNormalColor,
        m_style.buttonHoverColor,
        m_style.buttonPressedColor,
        m_style.buttonDisabledColor,
        button->state
    );
    
    Graphics::Color textColor = (button->state == UIElementState::Disabled) 
        ? m_style.buttonDisabledTextColor 
        : m_style.buttonTextColor;
    
    // Render button background
    renderRectangle(bounds, backgroundColor, true);
    
    // Render button border
    if (m_style.buttonBorderWidth > 0.0f) {
        renderRectangle(bounds, m_style.buttonBorderColor, false, m_style.buttonBorderWidth, m_style.buttonBorderColor);
    }
    
    // Render button text
    renderText(text, bounds.x, bounds.y, textColor, m_style.textSize, UIAlignment::Center, &bounds);
    
    m_elements.push_back(button);
    
    // Return true if button was clicked
    return button->state == UIElementState::Pressed && m_mouseClicked;
}

void UIRenderer::drawText(const UIRect& bounds, const std::string& text, UIAlignment alignment, 
                         float fontSize, const std::string& id) {
    auto textElement = std::make_shared<UIText>(text, id);
    textElement->bounds = bounds;
    textElement->fontSize = (fontSize > 0.0f) ? fontSize : m_style.textSize;
    textElement->alignment = alignment;
    
    Graphics::Color color = textElement->enabled ? m_style.textColor : m_style.textDisabledColor;
    
    renderText(text, bounds.x, bounds.y, color, textElement->fontSize, alignment, &bounds);
    
    m_elements.push_back(textElement);
}

void UIRenderer::drawProgressBar(const UIRect& bounds, float value, float minValue, float maxValue, const std::string& id) {
    auto progressBar = std::make_shared<UIProgressBar>(id);
    progressBar->bounds = bounds;
    progressBar->value = value;
    progressBar->minValue = minValue;
    progressBar->maxValue = maxValue;
    
    // Clamp value to range
    float clampedValue = std::max(minValue, std::min(maxValue, value));
    float progress = (clampedValue - minValue) / (maxValue - minValue);
    
    // Render background
    renderRectangle(bounds, m_style.progressBarBackgroundColor, true);
    
    // Render progress fill
    if (progress > 0.0f) {
        UIRect fillBounds = bounds;
        fillBounds.width *= progress;
        renderRectangle(fillBounds, m_style.progressBarForegroundColor, true);
    }
    
    // Render border
    if (m_style.progressBarBorderWidth > 0.0f) {
        renderRectangle(bounds, m_style.progressBarBorderColor, false, 
                       m_style.progressBarBorderWidth, m_style.progressBarBorderColor);
    }
    
    m_elements.push_back(progressBar);
}

bool UIRenderer::drawCheckbox(const UIRect& bounds, const std::string& label, bool checked, const std::string& id) {
    auto checkbox = std::make_shared<UICheckbox>(label, id);
    checkbox->bounds = bounds;
    checkbox->checked = checked;
    checkbox->state = getElementState(bounds, checkbox->enabled);
    
    // Calculate checkbox box bounds
    UIRect boxBounds;
    boxBounds.x = bounds.x;
    boxBounds.y = bounds.y + (bounds.height - m_style.checkboxSize) * 0.5f;
    boxBounds.width = m_style.checkboxSize;
    boxBounds.height = m_style.checkboxSize;
    
    // Render checkbox background
    renderRectangle(boxBounds, m_style.checkboxBackgroundColor, true);
    
    // Render check mark if checked
    if (checked) {
        UIRect checkBounds = boxBounds;
        checkBounds.x += 2.0f;
        checkBounds.y += 2.0f;
        checkBounds.width -= 4.0f;
        checkBounds.height -= 4.0f;
        renderRectangle(checkBounds, m_style.checkboxCheckedColor, true);
    }
    
    // Render checkbox border
    if (m_style.checkboxBorderWidth > 0.0f) {
        renderRectangle(boxBounds, m_style.checkboxBorderColor, false, 
                       m_style.checkboxBorderWidth, m_style.checkboxBorderColor);
    }
    
    // Render label
    if (!label.empty()) {
        float textX = boxBounds.x + boxBounds.width + 5.0f;
        float textY = bounds.y;
        Graphics::Color textColor = checkbox->enabled ? m_style.textColor : m_style.textDisabledColor;
        renderText(label, textX, textY, textColor, m_style.textSize, UIAlignment::CenterLeft);
    }
    
    m_elements.push_back(checkbox);
    
    // Return new checked state if clicked
    if (checkbox->state == UIElementState::Pressed && m_mouseClicked) {
        return !checked;
    }
    return checked;
}

float UIRenderer::drawSlider(const UIRect& bounds, float value, float minValue, float maxValue, const std::string& id) {
    auto slider = std::make_shared<UISlider>(id);
    slider->bounds = bounds;
    slider->value = value;
    slider->minValue = minValue;
    slider->maxValue = maxValue;
    slider->state = getElementState(bounds, slider->enabled);
    
    // Clamp value to range
    float clampedValue = std::max(minValue, std::min(maxValue, value));
    float progress = (clampedValue - minValue) / (maxValue - minValue);
    
    // Calculate track bounds
    UIRect trackBounds;
    trackBounds.x = bounds.x;
    trackBounds.y = bounds.y + (bounds.height - m_style.sliderTrackHeight) * 0.5f;
    trackBounds.width = bounds.width;
    trackBounds.height = m_style.sliderTrackHeight;
    
    // Render track
    renderRectangle(trackBounds, m_style.sliderTrackColor, true);
    
    // Render track border
    if (m_style.sliderBorderWidth > 0.0f) {
        renderRectangle(trackBounds, m_style.sliderBorderColor, false, 
                       m_style.sliderBorderWidth, m_style.sliderBorderColor);
    }
    
    // Calculate handle position
    float handleX = bounds.x + (bounds.width - m_style.sliderHandleSize) * progress;
    float handleY = bounds.y + (bounds.height - m_style.sliderHandleSize) * 0.5f;
    
    UIRect handleBounds(handleX, handleY, m_style.sliderHandleSize, m_style.sliderHandleSize);
    
    // Get handle color based on state
    Graphics::Color handleColor = (slider->state == UIElementState::Hovered || slider->state == UIElementState::Pressed)
        ? m_style.sliderHandleHoverColor
        : m_style.sliderHandleColor;
    
    // Render handle
    renderRectangle(handleBounds, handleColor, true);
    
    // Render handle border
    if (m_style.sliderBorderWidth > 0.0f) {
        renderRectangle(handleBounds, m_style.sliderBorderColor, false, 
                       m_style.sliderBorderWidth, m_style.sliderBorderColor);
    }
    
    m_elements.push_back(slider);
    
    // Handle dragging
    if (slider->state == UIElementState::Pressed && m_mousePressed) {
        float newProgress = (m_mouseX - bounds.x) / bounds.width;
        newProgress = std::max(0.0f, std::min(1.0f, newProgress));
        return minValue + newProgress * (maxValue - minValue);
    }
    
    return value;
}

void UIRenderer::drawImage(const UIRect& bounds, std::shared_ptr<Graphics::Texture> texture, 
                          const Graphics::Color& color, const std::string& id) {
    if (!texture) return;
    
    auto image = std::make_shared<UIElement>(UIElementType::Image, id);
    image->bounds = bounds;
    
    m_spriteRenderer->drawTexture(texture, bounds.x, bounds.y, bounds.width, bounds.height, color);
    
    m_elements.push_back(image);
}

bool UIRenderer::isMouseOver(const UIRect& bounds) const {
    return bounds.contains(m_mouseX, m_mouseY);
}

bool UIRenderer::isMouseClicked() const {
    return m_mouseClicked;
}

bool UIRenderer::isMousePressed() const {
    return m_mousePressed;
}

std::pair<float, float> UIRenderer::getMousePosition() const {
    return std::make_pair(m_mouseX, m_mouseY);
}

void UIRenderer::updateInput(float deltaTime) {
    // Get mouse position
    if (m_inputManager) {
        // Note: This is a simplified implementation
        // In a real implementation, you'd get actual mouse coordinates from the input manager
        m_mouseX = 0.0f; // m_inputManager->getMouseX();
        m_mouseY = 0.0f; // m_inputManager->getMouseY();
        
        m_previousMousePressed = m_mousePressed;
        m_mousePressed = m_inputManager->isActionPressed(ACTION_MOUSE_LEFT);
        m_mouseClicked = m_mousePressed && !m_previousMousePressed;
    }
}

void UIRenderer::renderRectangle(const UIRect& bounds, const Graphics::Color& color, bool filled, 
                                float borderWidth, const Graphics::Color& borderColor) {
    if (filled) {
        m_spriteRenderer->drawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, color, true);
    } else {
        // Draw border as four rectangles
        // Top
        m_spriteRenderer->drawRectangle(bounds.x, bounds.y, bounds.width, borderWidth, borderColor, true);
        // Bottom
        m_spriteRenderer->drawRectangle(bounds.x, bounds.y + bounds.height - borderWidth, bounds.width, borderWidth, borderColor, true);
        // Left
        m_spriteRenderer->drawRectangle(bounds.x, bounds.y, borderWidth, bounds.height, borderColor, true);
        // Right
        m_spriteRenderer->drawRectangle(bounds.x + bounds.width - borderWidth, bounds.y, borderWidth, bounds.height, borderColor, true);
    }
}

float UIRenderer::renderText(const std::string& text, float x, float y, const Graphics::Color& color, 
                            float fontSize, UIAlignment alignment, const UIRect* bounds) {
    // This is a simplified text rendering implementation
    // In a real implementation, you'd use a proper font rendering system
    
    float textX = x;
    float textY = y;
    
    if (bounds && alignment != UIAlignment::TopLeft) {
        auto textDims = getTextDimensions(text, fontSize);
        auto alignedPos = UILayout::calculateAlignedPosition(*bounds, textDims.first, textDims.second, alignment);
        textX = alignedPos.first;
        textY = alignedPos.second;
    }
    
    // For now, just draw a simple rectangle to represent text
    // In a real implementation, you'd render actual text glyphs
    auto textDims = getTextDimensions(text, fontSize);
    m_spriteRenderer->drawRectangle(textX, textY, textDims.first, textDims.second, color, true);
    
    return textDims.second;
}

std::pair<float, float> UIRenderer::getTextDimensions(const std::string& text, float fontSize) {
    // Simplified text dimension calculation
    // In a real implementation, you'd measure actual font metrics
    float width = text.length() * fontSize * 0.6f; // Approximate character width
    float height = fontSize;
    return std::make_pair(width, height);
}

UIElementState UIRenderer::getElementState(const UIRect& bounds, bool enabled) {
    if (!enabled) {
        return UIElementState::Disabled;
    }
    
    bool mouseOver = isMouseOver(bounds);
    
    if (mouseOver && m_mousePressed) {
        return UIElementState::Pressed;
    } else if (mouseOver) {
        return UIElementState::Hovered;
    }
    
    return UIElementState::Normal;
}

Graphics::Color UIRenderer::getStateColor(const Graphics::Color& normalColor, const Graphics::Color& hoverColor,
                                         const Graphics::Color& pressedColor, const Graphics::Color& disabledColor,
                                         UIElementState state) {
    switch (state) {
        case UIElementState::Hovered:
            return hoverColor;
        case UIElementState::Pressed:
            return pressedColor;
        case UIElementState::Disabled:
            return disabledColor;
        default:
            return normalColor;
    }
}

// UILayout implementation
std::pair<float, float> UILayout::calculateAlignedPosition(const UIRect& bounds, float elementWidth, float elementHeight, UIAlignment alignment) {
    float x = bounds.x;
    float y = bounds.y;
    
    switch (alignment) {
        case UIAlignment::TopLeft:
            break;
        case UIAlignment::TopCenter:
            x = bounds.x + (bounds.width - elementWidth) * 0.5f;
            break;
        case UIAlignment::TopRight:
            x = bounds.x + bounds.width - elementWidth;
            break;
        case UIAlignment::CenterLeft:
            y = bounds.y + (bounds.height - elementHeight) * 0.5f;
            break;
        case UIAlignment::Center:
            x = bounds.x + (bounds.width - elementWidth) * 0.5f;
            y = bounds.y + (bounds.height - elementHeight) * 0.5f;
            break;
        case UIAlignment::CenterRight:
            x = bounds.x + bounds.width - elementWidth;
            y = bounds.y + (bounds.height - elementHeight) * 0.5f;
            break;
        case UIAlignment::BottomLeft:
            y = bounds.y + bounds.height - elementHeight;
            break;
        case UIAlignment::BottomCenter:
            x = bounds.x + (bounds.width - elementWidth) * 0.5f;
            y = bounds.y + bounds.height - elementHeight;
            break;
        case UIAlignment::BottomRight:
            x = bounds.x + bounds.width - elementWidth;
            y = bounds.y + bounds.height - elementHeight;
            break;
    }
    
    return std::make_pair(x, y);
}

void UILayout::layoutVertical(std::vector<std::shared_ptr<UIElement>>& elements, const UIRect& bounds, float spacing) {
    if (elements.empty()) return;
    
    float currentY = bounds.y;
    float elementWidth = bounds.width;
    float elementHeight = (bounds.height - spacing * (elements.size() - 1)) / elements.size();
    
    for (auto& element : elements) {
        element->bounds.x = bounds.x;
        element->bounds.y = currentY;
        element->bounds.width = elementWidth;
        element->bounds.height = elementHeight;
        
        currentY += elementHeight + spacing;
    }
}

void UILayout::layoutHorizontal(std::vector<std::shared_ptr<UIElement>>& elements, const UIRect& bounds, float spacing) {
    if (elements.empty()) return;
    
    float currentX = bounds.x;
    float elementWidth = (bounds.width - spacing * (elements.size() - 1)) / elements.size();
    float elementHeight = bounds.height;
    
    for (auto& element : elements) {
        element->bounds.x = currentX;
        element->bounds.y = bounds.y;
        element->bounds.width = elementWidth;
        element->bounds.height = elementHeight;
        
        currentX += elementWidth + spacing;
    }
}

void UILayout::layoutGrid(std::vector<std::shared_ptr<UIElement>>& elements, const UIRect& bounds, int columns, float spacing) {
    if (elements.empty() || columns <= 0) return;
    
    int rows = (elements.size() + columns - 1) / columns;
    float elementWidth = (bounds.width - spacing * (columns - 1)) / columns;
    float elementHeight = (bounds.height - spacing * (rows - 1)) / rows;
    
    for (size_t i = 0; i < elements.size(); ++i) {
        int row = i / columns;
        int col = i % columns;
        
        float x = bounds.x + col * (elementWidth + spacing);
        float y = bounds.y + row * (elementHeight + spacing);
        
        elements[i]->bounds.x = x;
        elements[i]->bounds.y = y;
        elements[i]->bounds.width = elementWidth;
        elements[i]->bounds.height = elementHeight;
    }
}

} // namespace UI
} // namespace RPGEngine