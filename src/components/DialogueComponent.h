#pragma once

#include "Component.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace RPGEngine {
namespace Components {

/**
 * Dialogue node type enumeration
 */
enum class DialogueNodeType {
    Text,           // Simple text display
    Choice,         // Player choice selection
    Condition,      // Conditional branching
    Action,         // Execute action/script
    End             // End of dialogue
};

/**
 * Dialogue condition structure
 */
struct DialogueCondition {
    std::string type;           // Condition type (flag, stat, item, etc.)
    std::string target;         // Target to check (flag name, stat name, etc.)
    std::string operation;      // Operation (==, !=, >, <, >=, <=)
    std::string value;          // Value to compare against
    bool negate;                // Whether to negate the result
    
    DialogueCondition(const std::string& condType = "", const std::string& condTarget = "",
                     const std::string& op = "==", const std::string& val = "", bool neg = false)
        : type(condType), target(condTarget), operation(op), value(val), negate(neg) {}
};

/**
 * Dialogue action structure
 */
struct DialogueAction {
    std::string type;           // Action type (set_flag, give_item, etc.)
    std::string target;         // Action target
    std::string value;          // Action value
    std::unordered_map<std::string, std::string> parameters; // Additional parameters
    
    DialogueAction(const std::string& actionType = "", const std::string& actionTarget = "",
                  const std::string& actionValue = "")
        : type(actionType), target(actionTarget), value(actionValue) {}
};

/**
 * Dialogue choice structure
 */
struct DialogueChoice {
    std::string id;                         // Choice ID
    std::string text;                       // Choice text to display
    std::string nextNodeId;                 // Next node to go to
    std::vector<DialogueCondition> conditions; // Conditions for choice availability
    std::vector<DialogueAction> actions;    // Actions to execute when chosen
    bool enabled;                           // Whether choice is currently enabled
    bool visible;                           // Whether choice is visible
    
    DialogueChoice(const std::string& choiceId = "", const std::string& choiceText = "",
                  const std::string& nextNode = "")
        : id(choiceId), text(choiceText), nextNodeId(nextNode), enabled(true), visible(true) {}
};

/**
 * Dialogue node structure
 */
struct DialogueNode {
    std::string id;                         // Node ID
    DialogueNodeType type;                  // Node type
    std::string speaker;                    // Speaker name (empty for narrator)
    std::string text;                       // Text content
    std::string nextNodeId;                 // Next node (for text nodes)
    std::vector<DialogueChoice> choices;    // Available choices (for choice nodes)
    std::vector<DialogueCondition> conditions; // Conditions for node execution
    std::vector<DialogueAction> actions;    // Actions to execute
    std::unordered_map<std::string, std::string> metadata; // Additional metadata
    
    DialogueNode(const std::string& nodeId = "", DialogueNodeType nodeType = DialogueNodeType::Text)
        : id(nodeId), type(nodeType) {}
};

/**
 * Dialogue tree structure
 */
struct DialogueTree {
    std::string id;                         // Tree ID
    std::string name;                       // Tree name
    std::string description;                // Tree description
    std::string startNodeId;                // Starting node ID
    std::unordered_map<std::string, DialogueNode> nodes; // All nodes in the tree
    std::unordered_map<std::string, std::string> variables; // Tree-specific variables
    
    DialogueTree(const std::string& treeId = "", const std::string& treeName = "")
        : id(treeId), name(treeName) {}
    
    /**
     * Get node by ID
     * @param nodeId Node ID
     * @return Node pointer, or nullptr if not found
     */
    const DialogueNode* getNode(const std::string& nodeId) const {
        auto it = nodes.find(nodeId);
        return (it != nodes.end()) ? &it->second : nullptr;
    }
    
    /**
     * Add node to tree
     * @param node Node to add
     */
    void addNode(const DialogueNode& node) {
        nodes[node.id] = node;
    }
    
    /**
     * Check if tree has node
     * @param nodeId Node ID
     * @return true if node exists
     */
    bool hasNode(const std::string& nodeId) const {
        return nodes.find(nodeId) != nodes.end();
    }
};

/**
 * Dialogue state structure
 */
struct DialogueState {
    std::string currentTreeId;              // Current dialogue tree
    std::string currentNodeId;              // Current node
    std::unordered_map<std::string, bool> flags; // Dialogue flags
    std::unordered_map<std::string, std::string> variables; // Dialogue variables
    std::vector<std::string> history;       // Dialogue history
    bool isActive;                          // Whether dialogue is active
    bool canSkip;                           // Whether current dialogue can be skipped
    
    DialogueState() : isActive(false), canSkip(true) {}
    
    /**
     * Reset dialogue state
     */
    void reset() {
        currentTreeId.clear();
        currentNodeId.clear();
        isActive = false;
        canSkip = true;
    }
    
    /**
     * Set flag
     * @param flagName Flag name
     * @param value Flag value
     */
    void setFlag(const std::string& flagName, bool value) {
        flags[flagName] = value;
    }
    
    /**
     * Get flag
     * @param flagName Flag name
     * @param defaultValue Default value if flag doesn't exist
     * @return Flag value
     */
    bool getFlag(const std::string& flagName, bool defaultValue = false) const {
        auto it = flags.find(flagName);
        return (it != flags.end()) ? it->second : defaultValue;
    }
    
    /**
     * Set variable
     * @param varName Variable name
     * @param value Variable value
     */
    void setVariable(const std::string& varName, const std::string& value) {
        variables[varName] = value;
    }
    
    /**
     * Get variable
     * @param varName Variable name
     * @param defaultValue Default value if variable doesn't exist
     * @return Variable value
     */
    std::string getVariable(const std::string& varName, const std::string& defaultValue = "") const {
        auto it = variables.find(varName);
        return (it != variables.end()) ? it->second : defaultValue;
    }
};

/**
 * Dialogue component
 * Manages dialogue trees, state, and conversation flow
 */
class DialogueComponent : public Component<DialogueComponent> {
public:
    /**
     * Constructor
     * @param entityId Entity ID
     */
    explicit DialogueComponent(EntityId entityId);
    
    /**
     * Destructor
     */
    ~DialogueComponent();
    
    // Dialogue tree management
    
    /**
     * Load dialogue tree from JSON
     * @param jsonData JSON data string
     * @return true if loaded successfully
     */
    bool loadDialogueTreeFromJSON(const std::string& jsonData);
    
    /**
     * Load dialogue tree from file
     * @param filePath Path to dialogue file
     * @return true if loaded successfully
     */
    bool loadDialogueTreeFromFile(const std::string& filePath);
    
    /**
     * Add dialogue tree
     * @param tree Dialogue tree to add
     */
    void addDialogueTree(const DialogueTree& tree);
    
    /**
     * Get dialogue tree
     * @param treeId Tree ID
     * @return Tree pointer, or nullptr if not found
     */
    const DialogueTree* getDialogueTree(const std::string& treeId) const;
    
    /**
     * Remove dialogue tree
     * @param treeId Tree ID
     * @return true if tree was removed
     */
    bool removeDialogueTree(const std::string& treeId);
    
    /**
     * Get all dialogue trees
     * @return Map of tree IDs to trees
     */
    const std::unordered_map<std::string, DialogueTree>& getDialogueTrees() const { return m_dialogueTrees; }
    
    // Dialogue state management
    
    /**
     * Start dialogue
     * @param treeId Dialogue tree ID
     * @param startNodeId Starting node ID (empty for tree default)
     * @return true if dialogue started successfully
     */
    bool startDialogue(const std::string& treeId, const std::string& startNodeId = "");
    
    /**
     * End dialogue
     */
    void endDialogue();
    
    /**
     * Advance dialogue to next node
     * @param choiceId Choice ID (for choice nodes)
     * @return true if advanced successfully
     */
    bool advanceDialogue(const std::string& choiceId = "");
    
    /**
     * Get current dialogue node
     * @return Current node, or nullptr if no active dialogue
     */
    const DialogueNode* getCurrentNode() const;
    
    /**
     * Get current dialogue tree
     * @return Current tree, or nullptr if no active dialogue
     */
    const DialogueTree* getCurrentTree() const;
    
    /**
     * Check if dialogue is active
     * @return true if dialogue is active
     */
    bool isDialogueActive() const { return m_state.isActive; }
    
    /**
     * Get dialogue state
     * @return Dialogue state
     */
    const DialogueState& getDialogueState() const { return m_state; }
    
    /**
     * Set dialogue state
     * @param state New dialogue state
     */
    void setDialogueState(const DialogueState& state) { m_state = state; }
    
    // Condition and action evaluation
    
    /**
     * Evaluate condition
     * @param condition Condition to evaluate
     * @return true if condition is met
     */
    bool evaluateCondition(const DialogueCondition& condition) const;
    
    /**
     * Execute action
     * @param action Action to execute
     * @return true if action was executed successfully
     */
    bool executeAction(const DialogueAction& action);
    
    /**
     * Get available choices for current node
     * @return Vector of available choices
     */
    std::vector<DialogueChoice> getAvailableChoices() const;
    
    // Dialogue flags and variables
    
    /**
     * Set dialogue flag
     * @param flagName Flag name
     * @param value Flag value
     */
    void setFlag(const std::string& flagName, bool value) {
        m_state.setFlag(flagName, value);
    }
    
    /**
     * Get dialogue flag
     * @param flagName Flag name
     * @param defaultValue Default value
     * @return Flag value
     */
    bool getFlag(const std::string& flagName, bool defaultValue = false) const {
        return m_state.getFlag(flagName, defaultValue);
    }
    
    /**
     * Set dialogue variable
     * @param varName Variable name
     * @param value Variable value
     */
    void setVariable(const std::string& varName, const std::string& value) {
        m_state.setVariable(varName, value);
    }
    
    /**
     * Get dialogue variable
     * @param varName Variable name
     * @param defaultValue Default value
     * @return Variable value
     */
    std::string getVariable(const std::string& varName, const std::string& defaultValue = "") const {
        return m_state.getVariable(varName, defaultValue);
    }
    
    // Dialogue history
    
    /**
     * Add to dialogue history
     * @param text Text to add to history
     */
    void addToHistory(const std::string& text);
    
    /**
     * Get dialogue history
     * @return Vector of dialogue history
     */
    const std::vector<std::string>& getDialogueHistory() const { return m_state.history; }
    
    /**
     * Clear dialogue history
     */
    void clearHistory() { m_state.history.clear(); }
    
    // Callbacks
    
    /**
     * Set dialogue started callback
     * @param callback Function called when dialogue starts
     */
    void setDialogueStartedCallback(std::function<void(const std::string&)> callback) {
        m_dialogueStartedCallback = callback;
    }
    
    /**
     * Set dialogue ended callback
     * @param callback Function called when dialogue ends
     */
    void setDialogueEndedCallback(std::function<void()> callback) {
        m_dialogueEndedCallback = callback;
    }
    
    /**
     * Set node changed callback
     * @param callback Function called when dialogue node changes
     */
    void setNodeChangedCallback(std::function<void(const std::string&)> callback) {
        m_nodeChangedCallback = callback;
    }
    
    /**
     * Set action executed callback
     * @param callback Function called when action is executed
     */
    void setActionExecutedCallback(std::function<void(const DialogueAction&)> callback) {
        m_actionExecutedCallback = callback;
    }
    
    /**
     * Set external condition evaluator
     * @param evaluator Function to evaluate external conditions
     */
    void setExternalConditionEvaluator(std::function<bool(const DialogueCondition&)> evaluator) {
        m_externalConditionEvaluator = evaluator;
    }
    
    /**
     * Set external action executor
     * @param executor Function to execute external actions
     */
    void setExternalActionExecutor(std::function<bool(const DialogueAction&)> executor) {
        m_externalActionExecutor = executor;
    }
    
    // Serialization
    
    /**
     * Serialize component data
     * @return Serialized data
     */
    std::string serialize() const;
    
    /**
     * Deserialize component data
     * @param data Serialized data
     * @return true if successful
     */
    bool deserialize(const std::string& data);
    
private:
    /**
     * Parse JSON dialogue tree
     * @param jsonData JSON data
     * @return Parsed dialogue tree
     */
    DialogueTree parseJSONDialogueTree(const std::string& jsonData) const;
    
    /**
     * Parse dialogue node from JSON
     * @param nodeJson JSON node data
     * @return Parsed dialogue node
     */
    DialogueNode parseJSONNode(const std::string& nodeJson) const;
    
    /**
     * Parse dialogue choice from JSON
     * @param choiceJson JSON choice data
     * @return Parsed dialogue choice
     */
    DialogueChoice parseJSONChoice(const std::string& choiceJson) const;
    
    /**
     * Parse dialogue condition from JSON
     * @param conditionJson JSON condition data
     * @return Parsed dialogue condition
     */
    DialogueCondition parseJSONCondition(const std::string& conditionJson) const;
    
    /**
     * Parse dialogue action from JSON
     * @param actionJson JSON action data
     * @return Parsed dialogue action
     */
    DialogueAction parseJSONAction(const std::string& actionJson) const;
    
    /**
     * Evaluate conditions list
     * @param conditions Conditions to evaluate
     * @return true if all conditions are met
     */
    bool evaluateConditions(const std::vector<DialogueCondition>& conditions) const;
    
    /**
     * Execute actions list
     * @param actions Actions to execute
     * @return true if all actions were executed successfully
     */
    bool executeActions(const std::vector<DialogueAction>& actions);
    
    /**
     * Process text with variable substitution
     * @param text Text to process
     * @return Processed text
     */
    std::string processText(const std::string& text) const;
    
    /**
     * Trigger dialogue started event
     * @param treeId Tree ID
     */
    void triggerDialogueStarted(const std::string& treeId);
    
    /**
     * Trigger dialogue ended event
     */
    void triggerDialogueEnded();
    
    /**
     * Trigger node changed event
     * @param nodeId Node ID
     */
    void triggerNodeChanged(const std::string& nodeId);
    
    /**
     * Trigger action executed event
     * @param action Executed action
     */
    void triggerActionExecuted(const DialogueAction& action);
    
    // Dialogue data
    std::unordered_map<std::string, DialogueTree> m_dialogueTrees;
    DialogueState m_state;
    
    // Callbacks
    std::function<void(const std::string&)> m_dialogueStartedCallback;
    std::function<void()> m_dialogueEndedCallback;
    std::function<void(const std::string&)> m_nodeChangedCallback;
    std::function<void(const DialogueAction&)> m_actionExecutedCallback;
    
    // External handlers
    std::function<bool(const DialogueCondition&)> m_externalConditionEvaluator;
    std::function<bool(const DialogueAction&)> m_externalActionExecutor;
};

} // namespace Components
} // namespace RPGEngine