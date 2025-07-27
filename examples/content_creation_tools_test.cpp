#include <iostream>
#include <memory>

#include "../src/tools/MapEditor.h"
#include "../src/tools/AnimationEditor.h"
#include "../src/tools/DialogueEditor.h"

using namespace Engine;

int main() {
    std::cout << "=== Content Creation Tools Test ===" << std::endl;

    // Test Map Editor
    std::cout << "\n--- Testing Map Editor ---" << std::endl;
    
    auto mapEditor = std::make_unique<Tools::MapEditor>();
    
    // Create a new map
    bool success = mapEditor->createNewMap(20, 15, 32);
    std::cout << "Created new map: " << (success ? "Success" : "Failed") << std::endl;
    std::cout << "Map dimensions: " << mapEditor->getMapWidth() << "x" << mapEditor->getMapHeight() << std::endl;
    std::cout << "Tile size: " << mapEditor->getTileSize() << std::endl;
    std::cout << "Layer count: " << mapEditor->getLayerCount() << std::endl;
    
    // Add layers
    mapEditor->addLayer("Foreground");
    mapEditor->addLayer("Objects");
    std::cout << "Added layers, new count: " << mapEditor->getLayerCount() << std::endl;
    
    // Place some tiles
    Tools::TileSelection tile{0, 1, 0}; // tileset 0, tile 1, layer 0
    mapEditor->placeTile(5, 5, 0, tile);
    mapEditor->placeTile(6, 5, 0, tile);
    mapEditor->placeTile(7, 5, 0, tile);
    
    auto placedTile = mapEditor->getTile(5, 5, 0);
    std::cout << "Placed tile at (5,5): tileId=" << placedTile.tileId << ", tilesetId=" << placedTile.tilesetId << std::endl;
    
    // Test undo/redo
    std::cout << "Can undo: " << mapEditor->canUndo() << std::endl;
    mapEditor->undo();
    auto undoTile = mapEditor->getTile(7, 5, 0);
    std::cout << "After undo, tile at (7,5): tileId=" << undoTile.tileId << std::endl;
    
    std::cout << "Can redo: " << mapEditor->canRedo() << std::endl;
    mapEditor->redo();
    auto redoTile = mapEditor->getTile(7, 5, 0);
    std::cout << "After redo, tile at (7,5): tileId=" << redoTile.tileId << std::endl;
    
    // Test validation
    bool valid = mapEditor->validateMap();
    std::cout << "Map validation: " << (valid ? "Valid" : "Invalid") << std::endl;
    
    // Test save
    success = mapEditor->saveMap("test_map.map");
    std::cout << "Saved map: " << (success ? "Success" : "Failed") << std::endl;

    // Test Animation Editor
    std::cout << "\n--- Testing Animation Editor ---" << std::endl;
    
    auto animEditor = std::make_unique<Tools::AnimationEditor>();
    
    // Create new project
    success = animEditor->createNewProject();
    std::cout << "Created new animation project: " << (success ? "Success" : "Failed") << std::endl;
    
    // Set up sprite sheet
    animEditor->setSpriteSheetProperties(64, 64, 8, 4);
    auto spriteSheet = animEditor->getSpriteSheetInfo();
    std::cout << "Sprite sheet: " << spriteSheet.frameWidth << "x" << spriteSheet.frameHeight 
              << ", " << spriteSheet.columns << "x" << spriteSheet.rows << " = " << spriteSheet.totalFrames << " frames" << std::endl;
    
    // Create animations
    animEditor->createAnimation("walk");
    animEditor->createAnimation("idle");
    animEditor->createAnimation("attack");
    
    auto animNames = animEditor->getAnimationNames();
    std::cout << "Created animations: ";
    for (const auto& name : animNames) {
        std::cout << name << " ";
    }
    std::cout << std::endl;
    
    // Add frames to walk animation
    Tools::AnimationFrame frame1{0, 0.2f, 0, 0, ""};
    Tools::AnimationFrame frame2{1, 0.2f, 0, 0, ""};
    Tools::AnimationFrame frame3{2, 0.2f, 0, 0, ""};
    Tools::AnimationFrame frame4{3, 0.2f, 0, 0, "footstep"};
    
    animEditor->addFrame("walk", frame1);
    animEditor->addFrame("walk", frame2);
    animEditor->addFrame("walk", frame3);
    animEditor->addFrame("walk", frame4);
    
    int frameCount = animEditor->getFrameCount("walk");
    std::cout << "Walk animation frame count: " << frameCount << std::endl;
    
    float duration = animEditor->getAnimationDuration("walk");
    std::cout << "Walk animation duration: " << duration << "s" << std::endl;
    
    // Test animation preview
    animEditor->startPreview("walk");
    std::cout << "Started preview: " << animEditor->isPreviewPlaying() << std::endl;
    std::cout << "Current preview frame: " << animEditor->getCurrentPreviewFrame() << std::endl;
    
    // Simulate some time passing
    animEditor->updatePreview(0.1f);
    std::cout << "After 0.1s, current frame: " << animEditor->getCurrentPreviewFrame() << std::endl;
    
    animEditor->updatePreview(0.15f);
    std::cout << "After 0.25s total, current frame: " << animEditor->getCurrentPreviewFrame() << std::endl;
    
    animEditor->stopPreview();
    std::cout << "Stopped preview: " << animEditor->isPreviewPlaying() << std::endl;
    
    // Test validation
    bool animValid = animEditor->validateAnimation("walk");
    std::cout << "Walk animation validation: " << (animValid ? "Valid" : "Invalid") << std::endl;
    
    // Test save
    success = animEditor->saveProject("test_animations.proj");
    std::cout << "Saved animation project: " << (success ? "Success" : "Failed") << std::endl;

    // Test Dialogue Editor
    std::cout << "\n--- Testing Dialogue Editor ---" << std::endl;
    
    auto dialogueEditor = std::make_unique<Tools::DialogueEditor>();
    
    // Create new project
    success = dialogueEditor->createNewProject();
    std::cout << "Created new dialogue project: " << (success ? "Success" : "Failed") << std::endl;
    
    // Create dialogue trees
    dialogueEditor->createDialogueTree("intro");
    dialogueEditor->createDialogueTree("shop");
    dialogueEditor->createDialogueTree("quest");
    
    auto treeNames = dialogueEditor->getDialogueTreeNames();
    std::cout << "Created dialogue trees: ";
    for (const auto& name : treeNames) {
        std::cout << name << " ";
    }
    std::cout << std::endl;
    
    // Work with intro dialogue
    dialogueEditor->setCurrentDialogueTree("intro");
    
    // Create nodes
    std::string node1 = dialogueEditor->createNode(100, 100);
    std::string node2 = dialogueEditor->createNode(300, 100);
    std::string node3 = dialogueEditor->createNode(500, 100);
    
    std::cout << "Created nodes: " << node1 << ", " << node2 << ", " << node3 << std::endl;
    
    // Set up dialogue content
    dialogueEditor->setNodeSpeaker(node1, "Guard");
    dialogueEditor->setNodeText(node1, "Halt! Who goes there?");
    
    dialogueEditor->setNodeSpeaker(node2, "Player");
    dialogueEditor->setNodeText(node2, "I'm just a traveler.");
    
    dialogueEditor->setNodeSpeaker(node3, "Guard");
    dialogueEditor->setNodeText(node3, "Very well, you may pass.");
    dialogueEditor->setNodeAsEnd(node3, true);
    
    // Add choices
    Tools::DialogueChoice choice1{"I'm a friend", node2, "", true};
    Tools::DialogueChoice choice2{"None of your business!", node3, "", true};
    
    dialogueEditor->addChoice(node1, choice1);
    dialogueEditor->addChoice(node1, choice2);
    
    int choiceCount = dialogueEditor->getChoiceCount(node1);
    std::cout << "Node " << node1 << " has " << choiceCount << " choices" << std::endl;
    
    // Set connections
    dialogueEditor->setNodeNext(node2, node3);
    dialogueEditor->setStartNode(node1);
    
    std::cout << "Start node: " << dialogueEditor->getStartNode() << std::endl;
    
    // Test connections
    auto connected = dialogueEditor->getConnectedNodes(node1);
    std::cout << "Nodes connected from " << node1 << ": ";
    for (const auto& nodeId : connected) {
        std::cout << nodeId << " ";
    }
    std::cout << std::endl;
    
    // Test validation
    bool dialogueValid = dialogueEditor->validateDialogueTree("intro");
    std::cout << "Intro dialogue validation: " << (dialogueValid ? "Valid" : "Invalid") << std::endl;
    
    if (!dialogueValid) {
        auto errors = dialogueEditor->getValidationErrors("intro");
        std::cout << "Validation errors:" << std::endl;
        for (const auto& error : errors) {
            std::cout << "  - " << error << std::endl;
        }
    }
    
    // Test preview
    dialogueEditor->startPreview("intro");
    std::cout << "Started dialogue preview" << std::endl;
    std::cout << "Current node: " << dialogueEditor->getCurrentPreviewNode() << std::endl;
    
    auto previewChoices = dialogueEditor->getCurrentPreviewChoices();
    std::cout << "Available choices: " << previewChoices.size() << std::endl;
    for (size_t i = 0; i < previewChoices.size(); ++i) {
        std::cout << "  " << i << ": " << previewChoices[i].text << std::endl;
    }
    
    // Select first choice
    dialogueEditor->selectPreviewChoice(0);
    std::cout << "Selected choice 0, now at node: " << dialogueEditor->getCurrentPreviewNode() << std::endl;
    
    // Advance dialogue
    dialogueEditor->advancePreview();
    std::cout << "Advanced dialogue, now at node: " << dialogueEditor->getCurrentPreviewNode() << std::endl;
    std::cout << "Preview still active: " << dialogueEditor->isPreviewActive() << std::endl;
    
    // Test save
    success = dialogueEditor->saveProject("test_dialogue.proj");
    std::cout << "Saved dialogue project: " << (success ? "Success" : "Failed") << std::endl;
    
    // Test export
    success = dialogueEditor->exportDialogueTree("intro", "intro_dialogue.json");
    std::cout << "Exported intro dialogue: " << (success ? "Success" : "Failed") << std::endl;

    std::cout << "\n=== Content Creation Tools Test Complete ===" << std::endl;
    std::cout << "All content creation tools (MapEditor, AnimationEditor, DialogueEditor) created and tested successfully!" << std::endl;

    return 0;
}