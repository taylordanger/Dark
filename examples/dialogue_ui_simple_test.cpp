#include "../src/ui/DialogueUI.h"
#include <iostream>

using namespace RPGEngine;
using namespace RPGEngine::UI;

int main() {
    std::cout << "Testing DialogueUI header compilation..." << std::endl;
    
    // Test that the header compiles and basic structures are defined
    DialogueUIStyle style;
    style.windowWidth = 800.0f;
    style.windowHeight = 200.0f;
    style.textColor = Graphics::Color(1.0f, 1.0f, 1.0f, 1.0f);
    style.enableTypewriter = true;
    style.typewriterSpeed = 50.0f;
    
    std::cout << "DialogueUIStyle created successfully:" << std::endl;
    std::cout << "  Window size: " << style.windowWidth << "x" << style.windowHeight << std::endl;
    std::cout << "  Text color: (" << style.textColor.r << ", " << style.textColor.g 
              << ", " << style.textColor.b << ", " << style.textColor.a << ")" << std::endl;
    std::cout << "  Typewriter enabled: " << (style.enableTypewriter ? "true" : "false") << std::endl;
    std::cout << "  Typewriter speed: " << style.typewriterSpeed << " chars/sec" << std::endl;
    
    // Test enum values
    DialogueUIState state = DialogueUIState::Hidden;
    std::cout << "DialogueUIState enum works correctly" << std::endl;
    
    state = DialogueUIState::Showing;
    std::cout << "State changed to Showing" << std::endl;
    
    std::cout << "DialogueUI header test passed!" << std::endl;
    return 0;
}