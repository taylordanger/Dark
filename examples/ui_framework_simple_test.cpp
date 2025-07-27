#include "../src/ui/UIRenderer.h"
#include <iostream>
#include <memory>

using namespace RPGEngine;

int main() {
    std::cout << "=== UI Framework Simple Test ===" << std::endl;
    
    try {
        std::cout << "✓ UI framework headers compiled successfully" << std::endl;
        
        // Test UI style configuration
        UI::UIStyle customStyle;
        customStyle.buttonNormalColor = Graphics::Color(0.2f, 0.4f, 0.8f, 1.0f);
        customStyle.buttonHoverColor = Graphics::Color(0.3f, 0.5f, 0.9f, 1.0f);
        customStyle.textColor = Graphics::Color(1.0f, 1.0f, 0.0f, 1.0f);
        std::cout << "✓ UI style configured" << std::endl;
        
        // Test UI element structures
        UI::UIRect panelBounds(50, 50, 300, 200);
        UI::UIPanel panel("main_panel");
        panel.bounds = panelBounds;
        std::cout << "✓ Panel structure created" << std::endl;
        
        // Test button structure
        UI::UIRect buttonBounds(70, 80, 100, 30);
        UI::UIButton button("Click Me", "test_button");
        button.bounds = buttonBounds;
        std::cout << "✓ Button structure created" << std::endl;
        
        // Test text structure
        UI::UIRect textBounds(70, 120, 260, 20);
        UI::UIText text("Hello, UI Framework!", "test_text");
        text.bounds = textBounds;
        text.fontSize = 16.0f;
        text.alignment = UI::UIAlignment::Center;
        std::cout << "✓ Text structure created" << std::endl;
        
        // Test progress bar structure
        UI::UIRect progressBounds(70, 150, 200, 20);
        UI::UIProgressBar progressBar("test_progress");
        progressBar.bounds = progressBounds;
        progressBar.value = 0.7f;
        progressBar.minValue = 0.0f;
        progressBar.maxValue = 1.0f;
        std::cout << "✓ Progress bar structure created (70%)" << std::endl;
        
        // Test checkbox structure
        UI::UIRect checkboxBounds(70, 180, 150, 20);
        UI::UICheckbox checkbox("Enable Feature", "test_checkbox");
        checkbox.bounds = checkboxBounds;
        checkbox.checked = false;
        std::cout << "✓ Checkbox structure created (checked: " << (checkbox.checked ? "yes" : "no") << ")" << std::endl;
        
        // Test slider structure
        UI::UIRect sliderBounds(70, 210, 200, 20);
        UI::UISlider slider("test_slider");
        slider.bounds = sliderBounds;
        slider.value = 0.5f;
        slider.minValue = 0.0f;
        slider.maxValue = 1.0f;
        std::cout << "✓ Slider structure created (value: " << slider.value << ")" << std::endl;
        
        // Test layout system (structure only)
        std::vector<std::shared_ptr<UI::UIElement>> layoutElements;
        for (int i = 0; i < 3; ++i) {
            auto element = std::make_shared<UI::UIButton>("Button " + std::to_string(i + 1));
            layoutElements.push_back(element);
        }
        
        UI::UIRect layoutBounds(400, 50, 200, 150);
        std::cout << "✓ Layout elements created (" << layoutElements.size() << " elements)" << std::endl;
        
        // Test alignment calculation (manual implementation for testing)
        UI::UIRect bounds(0, 0, 100, 100);
        float elementWidth = 50, elementHeight = 25;
        
        // Center alignment
        float centerX = bounds.x + (bounds.width - elementWidth) * 0.5f;
        float centerY = bounds.y + (bounds.height - elementHeight) * 0.5f;
        std::cout << "✓ Center alignment calculated: (" << centerX << ", " << centerY << ")" << std::endl;
        
        // TopLeft alignment
        float topLeftX = bounds.x;
        float topLeftY = bounds.y;
        std::cout << "✓ TopLeft alignment: (" << topLeftX << ", " << topLeftY << ")" << std::endl;
        
        // BottomRight alignment
        float bottomRightX = bounds.x + bounds.width - elementWidth;
        float bottomRightY = bounds.y + bounds.height - elementHeight;
        std::cout << "✓ BottomRight alignment: (" << bottomRightX << ", " << bottomRightY << ")" << std::endl;
        
        // Test UI rectangle bounds checking
        UI::UIRect testBounds(10, 10, 50, 50);
        bool pointInside = testBounds.contains(30, 30);
        bool pointOutside = testBounds.contains(100, 100);
        std::cout << "✓ Bounds checking test (inside: " << (pointInside ? "yes" : "no") << ", outside: " << (pointOutside ? "yes" : "no") << ")" << std::endl;
        
        // Test UI element states
        std::cout << "✓ UI element states:" << std::endl;
        std::cout << "  - Normal: " << static_cast<int>(UI::UIElementState::Normal) << std::endl;
        std::cout << "  - Hovered: " << static_cast<int>(UI::UIElementState::Hovered) << std::endl;
        std::cout << "  - Pressed: " << static_cast<int>(UI::UIElementState::Pressed) << std::endl;
        std::cout << "  - Disabled: " << static_cast<int>(UI::UIElementState::Disabled) << std::endl;
        
        // Test UI element types
        std::cout << "✓ UI element types:" << std::endl;
        std::cout << "  - Panel: " << static_cast<int>(UI::UIElementType::Panel) << std::endl;
        std::cout << "  - Button: " << static_cast<int>(UI::UIElementType::Button) << std::endl;
        std::cout << "  - Text: " << static_cast<int>(UI::UIElementType::Text) << std::endl;
        std::cout << "  - ProgressBar: " << static_cast<int>(UI::UIElementType::ProgressBar) << std::endl;
        std::cout << "  - Checkbox: " << static_cast<int>(UI::UIElementType::Checkbox) << std::endl;
        std::cout << "  - Slider: " << static_cast<int>(UI::UIElementType::Slider) << std::endl;
        
        std::cout << "\n=== UI Framework Simple Test Completed Successfully ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error during UI framework test: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}