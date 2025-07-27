#include "DialogueComponent.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>

namespace RPGEngine {
namespace Components {

DialogueComponent::DialogueComponent(EntityId entityId)
    : Component<DialogueComponent>(entityId)
{
}

DialogueComponent::~DialogueComponent() {
    // Cleanup handled automatically
}

bool DialogueComponent::loadDialogueTreeFromJSON(const std::string& jsonData) {
    try {
        DialogueTree tree = parseJSONDialogueTree(jsonData);
        if (!tree.id.empty()) {
            addDialogueTree(tree);
            std::cout << "Loaded dialogue tree: " << tree.name << " (" << tree.id << ")" << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse dialogue JSON: " << e.what() << std::endl;
    }
    
    return false;
}

bool DialogueComponent::loadDialogueTreeFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open dialogue file: " << filePath << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return loadDialogueTreeFromJSON(buffer.str());
}

void DialogueComponent::addDialogueTree(const DialogueTree& tree) {
    m_dialogueTrees[tree.id] = tree;
    std::cout << "Added dialogue tree: " << tree.name << " with " << tree.nodes.size() << " nodes" << std::endl;
}

const DialogueTree* DialogueComponent::getDialogueTree(const std::string& treeId) const {
    auto it = m_dialogueTrees.find(treeId);
    return (it != m_dialogueTrees.end()) ? &it->second : nullptr;
}

bool DialogueComponent::removeDialogueTree(const std::string& treeId) {
    auto it = m_dialogueTrees.find(treeId);
    if (it != m_dialogueTrees.end()) {
        std::cout << "Removed dialogue tree: " << treeId << std::endl;
        m_dialogueTrees.erase(it);
        return true;
    }
    return false;
}

bool DialogueComponent::startDialogue(const std::string& treeId, const std::string& startNodeId) {
    const DialogueTree* tree = getDialogueTree(treeId);
    if (!tree) {
        std::cerr << "Dialogue tree not found: " << treeId << std::endl;
        return false;
    }
    
    std::string nodeId = startNodeId.empty() ? tree->startNodeId : startNodeId;
    if (!tree->hasNode(nodeId)) {
        std::cerr << "Start node not found: " << nodeId << std::endl;
        return false;
    }
    
    // Reset state and start dialogue
    m_state.reset();
    m_state.currentTreeId = treeId;
    m_state.currentNodeId = nodeId;
    m_state.isActive = true;
    
    // Execute node actions
    const DialogueNode* node = tree->getNode(nodeId);
    if (node) {
        executeActions(node->actions);
    }
    
    triggerDialogueStarted(treeId);
    triggerNodeChanged(nodeId);
    
    std::cout << "Started dialogue: " << treeId << " at node: " << nodeId << std::endl;
    return true;
}

void DialogueComponent::endDialogue() {
    if (!m_state.isActive) {
        return;
    }
    
    std::cout << "Ended dialogue: " << m_state.currentTreeId << std::endl;
    
    triggerDialogueEnded();
    m_state.reset();
}

bool DialogueComponent::advanceDialogue(const std::string& choiceId) {
    if (!m_state.isActive) {
        return false;
    }
    
    const DialogueTree* tree = getCurrentTree();
    const DialogueNode* currentNode = getCurrentNode();
    
    if (!tree || !currentNode) {
        return false;
    }
    
    std::string nextNodeId;
    
    // Determine next node based on current node type
    switch (currentNode->type) {
        case DialogueNodeType::Text: {
            nextNodeId = currentNode->nextNodeId;
            break;
        }
        
        case DialogueNodeType::Choice: {
            if (choiceId.empty()) {
                std::cerr << "Choice ID required for choice node" << std::endl;
                return false;
            }
            
            // Find the selected choice
            auto it = std::find_if(currentNode->choices.begin(), currentNode->choices.end(),
                [&choiceId](const DialogueChoice& choice) {
                    return choice.id == choiceId;
                });
            
            if (it == currentNode->choices.end()) {
                std::cerr << "Choice not found: " << choiceId << std::endl;
                return false;
            }
            
            const DialogueChoice& choice = *it;
            
            // Check if choice is available
            if (!choice.enabled || !choice.visible || !evaluateConditions(choice.conditions)) {
                std::cerr << "Choice not available: " << choiceId << std::endl;
                return false;
            }
            
            // Execute choice actions
            executeActions(choice.actions);
            
            nextNodeId = choice.nextNodeId;
            
            // Add choice to history
            addToHistory("Choice: " + choice.text);
            break;
        }
        
        case DialogueNodeType::Condition: {
            // Evaluate conditions and choose next node
            if (evaluateConditions(currentNode->conditions)) {
                nextNodeId = currentNode->nextNodeId;
            } else {
                // Look for alternative path or end dialogue
                // For now, just end dialogue if condition fails
                endDialogue();
                return true;
            }
            break;
        }
        
        case DialogueNodeType::Action: {
            // Execute actions and move to next node
            executeActions(currentNode->actions);
            nextNodeId = currentNode->nextNodeId;
            break;
        }
        
        case DialogueNodeType::End: {
            endDialogue();
            return true;
        }
    }
    
    // Check if we should end dialogue
    if (nextNodeId.empty() || nextNodeId == "end" || !tree->hasNode(nextNodeId)) {
        endDialogue();
        return true;
    }
    
    // Move to next node
    m_state.currentNodeId = nextNodeId;
    
    const DialogueNode* nextNode = tree->getNode(nextNodeId);
    if (nextNode) {
        // Check node conditions
        if (!evaluateConditions(nextNode->conditions)) {
            // Skip this node and try to advance again
            return advanceDialogue();
        }
        
        // Execute node actions
        executeActions(nextNode->actions);
        
        // Add text to history
        if (!nextNode->text.empty()) {
            std::string processedText = processText(nextNode->text);
            std::string speaker = nextNode->speaker.empty() ? "Narrator" : nextNode->speaker;
            addToHistory(speaker + ": " + processedText);
        }
    }
    
    triggerNodeChanged(nextNodeId);
    
    std::cout << "Advanced dialogue to node: " << nextNodeId << std::endl;
    return true;
}

const DialogueNode* DialogueComponent::getCurrentNode() const {
    const DialogueTree* tree = getCurrentTree();
    return tree ? tree->getNode(m_state.currentNodeId) : nullptr;
}

const DialogueTree* DialogueComponent::getCurrentTree() const {
    return getDialogueTree(m_state.currentTreeId);
}

bool DialogueComponent::evaluateCondition(const DialogueCondition& condition) const {
    bool result = false;
    
    if (condition.type == "flag") {
        bool flagValue = getFlag(condition.target, false);
        
        if (condition.operation == "==" || condition.operation == "equals") {
            result = flagValue == (condition.value == "true");
        } else if (condition.operation == "!=" || condition.operation == "not_equals") {
            result = flagValue != (condition.value == "true");
        }
    } else if (condition.type == "variable") {
        std::string varValue = getVariable(condition.target, "");
        
        if (condition.operation == "==" || condition.operation == "equals") {
            result = varValue == condition.value;
        } else if (condition.operation == "!=" || condition.operation == "not_equals") {
            result = varValue != condition.value;
        } else if (condition.operation == "contains") {
            result = varValue.find(condition.value) != std::string::npos;
        }
    } else if (condition.type == "stat") {
        // TODO: Integrate with StatsComponent
        // For now, just return true
        result = true;
    } else if (condition.type == "item") {
        // TODO: Integrate with InventoryComponent
        // For now, just return true
        result = true;
    } else if (condition.type == "quest_active" || condition.type == "quest_completed" || 
               condition.type == "objective_completed" || condition.type == "quest_variable") {
        // Quest-related conditions - delegate to external evaluator
        if (m_externalConditionEvaluator) {
            result = m_externalConditionEvaluator(condition);
        } else {
            std::cerr << "No external condition evaluator set for quest condition: " << condition.type << std::endl;
            result = false;
        }
    } else {
        // Unknown condition type
        std::cerr << "Unknown condition type: " << condition.type << std::endl;
        result = false;
    }
    
    return condition.negate ? !result : result;
}

bool DialogueComponent::executeAction(const DialogueAction& action) {
    std::cout << "Executing action: " << action.type << " " << action.target << " " << action.value << std::endl;
    
    bool success = false;
    
    if (action.type == "set_flag") {
        setFlag(action.target, action.value == "true");
        success = true;
    } else if (action.type == "set_variable") {
        setVariable(action.target, action.value);
        success = true;
    } else if (action.type == "give_item") {
        // TODO: Integrate with InventoryComponent
        std::cout << "Would give item: " << action.target << " x" << action.value << std::endl;
        success = true;
    } else if (action.type == "remove_item") {
        // TODO: Integrate with InventoryComponent
        std::cout << "Would remove item: " << action.target << " x" << action.value << std::endl;
        success = true;
    } else if (action.type == "modify_stat") {
        // TODO: Integrate with StatsComponent
        std::cout << "Would modify stat: " << action.target << " by " << action.value << std::endl;
        success = true;
    } else if (action.type == "play_sound") {
        // TODO: Integrate with AudioManager
        std::cout << "Would play sound: " << action.target << std::endl;
        success = true;
    } else if (action.type == "start_quest" || action.type == "complete_quest" || 
               action.type == "update_objective" || action.type == "set_quest_variable" ||
               action.type == "track_npc_interaction" || action.type == "track_location_visit" ||
               action.type == "track_custom_objective") {
        // Quest-related actions - delegate to external executor
        if (m_externalActionExecutor) {
            success = m_externalActionExecutor(action);
        } else {
            std::cerr << "No external action executor set for quest action: " << action.type << std::endl;
            success = false;
        }
    } else {
        std::cerr << "Unknown action type: " << action.type << std::endl;
        success = false;
    }
    
    triggerActionExecuted(action);
    return success;
}

std::vector<DialogueChoice> DialogueComponent::getAvailableChoices() const {
    const DialogueNode* currentNode = getCurrentNode();
    if (!currentNode || currentNode->type != DialogueNodeType::Choice) {
        return {};
    }
    
    std::vector<DialogueChoice> availableChoices;
    
    for (const auto& choice : currentNode->choices) {
        if (choice.visible && choice.enabled && evaluateConditions(choice.conditions)) {
            availableChoices.push_back(choice);
        }
    }
    
    return availableChoices;
}

void DialogueComponent::addToHistory(const std::string& text) {
    m_state.history.push_back(text);
    
    // Limit history size to prevent memory issues
    const size_t maxHistorySize = 100;
    if (m_state.history.size() > maxHistorySize) {
        m_state.history.erase(m_state.history.begin());
    }
}

std::string DialogueComponent::serialize() const {
    std::ostringstream oss;
    
    // Serialize dialogue state
    oss << m_state.currentTreeId << "," << m_state.currentNodeId << "," 
        << (m_state.isActive ? "1" : "0") << "," << (m_state.canSkip ? "1" : "0") << ",";
    
    // Serialize flags
    oss << m_state.flags.size() << ",";
    for (const auto& pair : m_state.flags) {
        oss << pair.first << "|" << (pair.second ? "1" : "0") << ";";
    }
    oss << ",";
    
    // Serialize variables
    oss << m_state.variables.size() << ",";
    for (const auto& pair : m_state.variables) {
        oss << pair.first << "|" << pair.second << ";";
    }
    oss << ",";
    
    // Serialize history
    oss << m_state.history.size() << ",";
    for (const auto& entry : m_state.history) {
        oss << entry << ";";
    }
    
    return oss.str();
}

bool DialogueComponent::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string token;
    
    try {
        // Deserialize dialogue state
        std::getline(iss, token, ',');
        m_state.currentTreeId = token;
        
        std::getline(iss, token, ',');
        m_state.currentNodeId = token;
        
        std::getline(iss, token, ',');
        m_state.isActive = (token == "1");
        
        std::getline(iss, token, ',');
        m_state.canSkip = (token == "1");
        
        // Deserialize flags
        std::getline(iss, token, ',');
        int flagCount = std::stoi(token);
        
        m_state.flags.clear();
        
        std::getline(iss, token, ',');
        if (!token.empty() && flagCount > 0) {
            std::istringstream flagStream(token);
            std::string flagToken;
            
            while (std::getline(flagStream, flagToken, ';') && flagCount > 0) {
                if (!flagToken.empty()) {
                    size_t pipePos = flagToken.find('|');
                    if (pipePos != std::string::npos) {
                        std::string flagName = flagToken.substr(0, pipePos);
                        bool flagValue = (flagToken.substr(pipePos + 1) == "1");
                        m_state.flags[flagName] = flagValue;
                    }
                    flagCount--;
                }
            }
        }
        
        // Deserialize variables
        std::getline(iss, token, ',');
        int varCount = std::stoi(token);
        
        m_state.variables.clear();
        
        std::getline(iss, token, ',');
        if (!token.empty() && varCount > 0) {
            std::istringstream varStream(token);
            std::string varToken;
            
            while (std::getline(varStream, varToken, ';') && varCount > 0) {
                if (!varToken.empty()) {
                    size_t pipePos = varToken.find('|');
                    if (pipePos != std::string::npos) {
                        std::string varName = varToken.substr(0, pipePos);
                        std::string varValue = varToken.substr(pipePos + 1);
                        m_state.variables[varName] = varValue;
                    }
                    varCount--;
                }
            }
        }
        
        // Deserialize history
        std::getline(iss, token, ',');
        int historyCount = std::stoi(token);
        
        m_state.history.clear();
        
        std::string historyData;
        std::getline(iss, historyData);
        if (!historyData.empty() && historyCount > 0) {
            std::istringstream historyStream(historyData);
            std::string historyToken;
            
            while (std::getline(historyStream, historyToken, ';') && historyCount > 0) {
                if (!historyToken.empty()) {
                    m_state.history.push_back(historyToken);
                    historyCount--;
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to deserialize DialogueComponent: " << e.what() << std::endl;
        return false;
    }
}

DialogueTree DialogueComponent::parseJSONDialogueTree(const std::string& jsonData) const {
    // Simple JSON parsing - in a real implementation, you'd use a proper JSON library
    DialogueTree tree;
    
    // For this example, create a simple dialogue tree programmatically
    // In a real implementation, this would parse actual JSON
    
    tree.id = "test_dialogue";
    tree.name = "Test Dialogue";
    tree.description = "A simple test dialogue";
    tree.startNodeId = "start";
    
    // Create start node
    DialogueNode startNode("start", DialogueNodeType::Text);
    startNode.speaker = "NPC";
    startNode.text = "Hello, traveler! How can I help you?";
    startNode.nextNodeId = "choice1";
    tree.addNode(startNode);
    
    // Create choice node
    DialogueNode choiceNode("choice1", DialogueNodeType::Choice);
    choiceNode.text = "What would you like to do?";
    
    DialogueChoice choice1("buy", "I'd like to buy something", "shop");
    DialogueChoice choice2("quest", "Do you have any quests?", "quest_check");
    DialogueChoice choice3("goodbye", "Goodbye", "end");
    
    choiceNode.choices.push_back(choice1);
    choiceNode.choices.push_back(choice2);
    choiceNode.choices.push_back(choice3);
    tree.addNode(choiceNode);
    
    // Create shop node
    DialogueNode shopNode("shop", DialogueNodeType::Text);
    shopNode.speaker = "NPC";
    shopNode.text = "Welcome to my shop! Take a look around.";
    shopNode.nextNodeId = "end";
    tree.addNode(shopNode);
    
    // Create quest check node
    DialogueNode questNode("quest_check", DialogueNodeType::Condition);
    questNode.conditions.push_back(DialogueCondition("flag", "has_quest", "==", "false"));
    questNode.nextNodeId = "give_quest";
    tree.addNode(questNode);
    
    // Create give quest node
    DialogueNode giveQuestNode("give_quest", DialogueNodeType::Text);
    giveQuestNode.speaker = "NPC";
    giveQuestNode.text = "I have a quest for you! Can you help me find my lost ring?";
    giveQuestNode.actions.push_back(DialogueAction("set_flag", "has_quest", "true"));
    giveQuestNode.actions.push_back(DialogueAction("start_quest", "lost_ring", ""));
    giveQuestNode.nextNodeId = "end";
    tree.addNode(giveQuestNode);
    
    // Create end node
    DialogueNode endNode("end", DialogueNodeType::End);
    endNode.text = "Farewell!";
    tree.addNode(endNode);
    
    return tree;
}

DialogueNode DialogueComponent::parseJSONNode(const std::string& nodeJson) const {
    // Placeholder for JSON node parsing
    return DialogueNode();
}

DialogueChoice DialogueComponent::parseJSONChoice(const std::string& choiceJson) const {
    // Placeholder for JSON choice parsing
    return DialogueChoice();
}

DialogueCondition DialogueComponent::parseJSONCondition(const std::string& conditionJson) const {
    // Placeholder for JSON condition parsing
    return DialogueCondition();
}

DialogueAction DialogueComponent::parseJSONAction(const std::string& actionJson) const {
    // Placeholder for JSON action parsing
    return DialogueAction();
}

bool DialogueComponent::evaluateConditions(const std::vector<DialogueCondition>& conditions) const {
    for (const auto& condition : conditions) {
        if (!evaluateCondition(condition)) {
            return false;
        }
    }
    return true;
}

bool DialogueComponent::executeActions(const std::vector<DialogueAction>& actions) {
    bool allSucceeded = true;
    
    for (const auto& action : actions) {
        if (!executeAction(action)) {
            allSucceeded = false;
        }
    }
    
    return allSucceeded;
}

std::string DialogueComponent::processText(const std::string& text) const {
    std::string processedText = text;
    
    // Replace variables in format {variable_name}
    std::regex variableRegex(R"(\{([^}]+)\})");
    std::smatch match;
    
    while (std::regex_search(processedText, match, variableRegex)) {
        std::string varName = match[1].str();
        std::string varValue = getVariable(varName, "{" + varName + "}");
        processedText = std::regex_replace(processedText, variableRegex, varValue, std::regex_constants::format_first_only);
    }
    
    return processedText;
}

void DialogueComponent::triggerDialogueStarted(const std::string& treeId) {
    if (m_dialogueStartedCallback) {
        m_dialogueStartedCallback(treeId);
    }
}

void DialogueComponent::triggerDialogueEnded() {
    if (m_dialogueEndedCallback) {
        m_dialogueEndedCallback();
    }
}

void DialogueComponent::triggerNodeChanged(const std::string& nodeId) {
    if (m_nodeChangedCallback) {
        m_nodeChangedCallback(nodeId);
    }
}

void DialogueComponent::triggerActionExecuted(const DialogueAction& action) {
    if (m_actionExecutedCallback) {
        m_actionExecutedCallback(action);
    }
}

} // namespace Components
} // namespace RPGEngine