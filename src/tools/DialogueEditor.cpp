#include "DialogueEditor.h"
#include <fstream>
#include <algorithm>
#include <sstream>

namespace Engine {
    namespace Tools {

        DialogueEditor::DialogueEditor()
            : m_previewActive(false)
            , m_maxUndoSteps(100)
            , m_nextNodeId(1) {
        }

        DialogueEditor::~DialogueEditor() {
        }

        bool DialogueEditor::createNewProject() {
            m_dialogueTrees.clear();
            m_currentTreeName.clear();
            m_undoStack.clear();
            m_redoStack.clear();
            m_nextNodeId = 1;
            stopPreview();
            return true;
        }

        bool DialogueEditor::loadProject(const std::string& filepath) {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            createNewProject();
            
            // Simplified project loading
            std::string line;
            DialogueTree* currentTree = nullptr;
            
            while (std::getline(file, line)) {
                if (line.find("tree=") == 0) {
                    std::string treeName = line.substr(5);
                    createDialogueTree(treeName);
                    currentTree = findDialogueTree(treeName);
                } else if (line.find("startnode=") == 0 && currentTree) {
                    currentTree->startNodeId = line.substr(10);
                } else if (line.find("node=") == 0 && currentTree) {
                    // Parse node data (simplified)
                    std::string nodeData = line.substr(5);
                    size_t comma1 = nodeData.find(',');
                    if (comma1 != std::string::npos) {
                        std::string nodeId = nodeData.substr(0, comma1);
                        DialogueNode node;
                        node.id = nodeId;
                        node.text = "Sample text";
                        node.isEndNode = false;
                        node.x = 0.0f;
                        node.y = 0.0f;
                        currentTree->nodes[nodeId] = node;
                    }
                }
            }
            
            return true;
        }

        bool DialogueEditor::saveProject(const std::string& filepath) {
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            file << "# Dialogue Editor Project\n";
            
            for (const auto& tree : m_dialogueTrees) {
                file << "tree=" << tree.name << "\n";
                file << "startnode=" << tree.startNodeId << "\n";
                
                for (const auto& nodePair : tree.nodes) {
                    const auto& node = nodePair.second;
                    file << "node=" << node.id << ",speaker=" << node.speakerName 
                         << ",x=" << node.x << ",y=" << node.y << "\n";
                    file << "text=" << node.text << "\n";
                    
                    if (!node.script.empty()) {
                        file << "script=" << node.script << "\n";
                    }
                    
                    if (!node.nextNodeId.empty()) {
                        file << "next=" << node.nextNodeId << "\n";
                    }
                    
                    for (size_t i = 0; i < node.choices.size(); ++i) {
                        const auto& choice = node.choices[i];
                        file << "choice=" << i << ",text=" << choice.text 
                             << ",target=" << choice.targetNodeId;
                        if (!choice.condition.empty()) {
                            file << ",condition=" << choice.condition;
                        }
                        file << "\n";
                    }
                }
                
                for (const auto& varPair : tree.variables) {
                    file << "var=" << varPair.first << "=" << varPair.second << "\n";
                }
            }
            
            return true;
        }

        void DialogueEditor::createDialogueTree(const std::string& name) {
            if (hasDialogueTree(name)) {
                return;
            }
            
            DialogueTree newTree;
            newTree.name = name;
            
            m_dialogueTrees.push_back(newTree);
            
            EditorAction action;
            action.type = EditorAction::CreateTree;
            action.treeName = name;
            addUndoAction(action);
        }

        void DialogueEditor::deleteDialogueTree(const std::string& name) {
            auto it = std::find_if(m_dialogueTrees.begin(), m_dialogueTrees.end(),
                [&name](const DialogueTree& tree) { return tree.name == name; });
            
            if (it != m_dialogueTrees.end()) {
                EditorAction action;
                action.type = EditorAction::DeleteTree;
                action.treeName = name;
                
                m_dialogueTrees.erase(it);
                
                if (m_currentTreeName == name) {
                    m_currentTreeName.clear();
                }
                
                if (m_previewTreeName == name) {
                    stopPreview();
                }
                
                addUndoAction(action);
            }
        }

        void DialogueEditor::renameDialogueTree(const std::string& oldName, const std::string& newName) {
            if (oldName == newName || hasDialogueTree(newName)) {
                return;
            }
            
            auto* tree = findDialogueTree(oldName);
            if (tree) {
                EditorAction action;
                action.type = EditorAction::RenameTree;
                action.oldValue = oldName;
                action.newValue = newName;
                
                tree->name = newName;
                
                if (m_currentTreeName == oldName) {
                    m_currentTreeName = newName;
                }
                
                if (m_previewTreeName == oldName) {
                    m_previewTreeName = newName;
                }
                
                addUndoAction(action);
            }
        }

        std::vector<std::string> DialogueEditor::getDialogueTreeNames() const {
            std::vector<std::string> names;
            for (const auto& tree : m_dialogueTrees) {
                names.push_back(tree.name);
            }
            return names;
        }

        bool DialogueEditor::hasDialogueTree(const std::string& name) const {
            return findDialogueTree(name) != nullptr;
        }

        void DialogueEditor::setCurrentDialogueTree(const std::string& name) {
            if (hasDialogueTree(name)) {
                m_currentTreeName = name;
            }
        }

        std::string DialogueEditor::getCurrentDialogueTree() const {
            return m_currentTreeName;
        }

        std::string DialogueEditor::createNode(float x, float y) {
            auto* tree = getCurrentTree();
            if (!tree) {
                return "";
            }
            
            std::string nodeId = generateNodeId();
            
            DialogueNode node;
            node.id = nodeId;
            node.speakerName = "Speaker";
            node.text = "New dialogue text";
            node.isEndNode = false;
            node.x = x;
            node.y = y;
            
            tree->nodes[nodeId] = node;
            
            // Set as start node if this is the first node
            if (tree->startNodeId.empty()) {
                tree->startNodeId = nodeId;
            }
            
            EditorAction action;
            action.type = EditorAction::CreateNode;
            action.treeName = m_currentTreeName;
            action.nodeId = nodeId;
            action.newX = x;
            action.newY = y;
            addUndoAction(action);
            
            return nodeId;
        }

        void DialogueEditor::deleteNode(const std::string& nodeId) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                return;
            }
            
            EditorAction action;
            action.type = EditorAction::DeleteNode;
            action.treeName = m_currentTreeName;
            action.nodeId = nodeId;
            
            tree->nodes.erase(nodeId);
            
            // Clear start node if this was it
            if (tree->startNodeId == nodeId) {
                tree->startNodeId.clear();
            }
            
            // Remove references from other nodes
            for (auto& nodePair : tree->nodes) {
                auto& node = nodePair.second;
                if (node.nextNodeId == nodeId) {
                    node.nextNodeId.clear();
                }
                for (auto& choice : node.choices) {
                    if (choice.targetNodeId == nodeId) {
                        choice.targetNodeId.clear();
                    }
                }
            }
            
            addUndoAction(action);
        }

        void DialogueEditor::moveNode(const std::string& nodeId, float x, float y) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                return;
            }
            
            auto& node = tree->nodes[nodeId];
            
            EditorAction action;
            action.type = EditorAction::MoveNode;
            action.treeName = m_currentTreeName;
            action.nodeId = nodeId;
            action.oldX = node.x;
            action.oldY = node.y;
            action.newX = x;
            action.newY = y;
            
            node.x = x;
            node.y = y;
            
            addUndoAction(action);
        }

        bool DialogueEditor::hasNode(const std::string& nodeId) const {
            const auto* tree = getCurrentTree();
            return tree && tree->nodes.find(nodeId) != tree->nodes.end();
        }

        DialogueNode DialogueEditor::getNode(const std::string& nodeId) const {
            const auto* tree = getCurrentTree();
            if (tree) {
                auto it = tree->nodes.find(nodeId);
                if (it != tree->nodes.end()) {
                    return it->second;
                }
            }
            return DialogueNode{};
        }

        std::vector<std::string> DialogueEditor::getAllNodeIds() const {
            std::vector<std::string> nodeIds;
            const auto* tree = getCurrentTree();
            if (tree) {
                for (const auto& nodePair : tree->nodes) {
                    nodeIds.push_back(nodePair.first);
                }
            }
            return nodeIds;
        }

        void DialogueEditor::setNodeSpeaker(const std::string& nodeId, const std::string& speaker) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                return;
            }
            
            auto& node = tree->nodes[nodeId];
            
            EditorAction action;
            action.type = EditorAction::SetNodeProperty;
            action.treeName = m_currentTreeName;
            action.nodeId = nodeId;
            action.oldValue = node.speakerName;
            action.newValue = speaker;
            
            node.speakerName = speaker;
            
            addUndoAction(action);
        }

        void DialogueEditor::setNodeText(const std::string& nodeId, const std::string& text) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                return;
            }
            
            auto& node = tree->nodes[nodeId];
            
            EditorAction action;
            action.type = EditorAction::SetNodeProperty;
            action.treeName = m_currentTreeName;
            action.nodeId = nodeId;
            action.oldValue = node.text;
            action.newValue = text;
            
            node.text = text;
            
            addUndoAction(action);
        }

        void DialogueEditor::setNodeScript(const std::string& nodeId, const std::string& script) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                return;
            }
            
            tree->nodes[nodeId].script = script;
        }

        void DialogueEditor::setNodeAsEnd(const std::string& nodeId, bool isEnd) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                return;
            }
            
            tree->nodes[nodeId].isEndNode = isEnd;
        }

        void DialogueEditor::setNodeNext(const std::string& nodeId, const std::string& nextNodeId) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                return;
            }
            
            tree->nodes[nodeId].nextNodeId = nextNodeId;
        }

        void DialogueEditor::addChoice(const std::string& nodeId, const DialogueChoice& choice) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                return;
            }
            
            auto& node = tree->nodes[nodeId];
            
            EditorAction action;
            action.type = EditorAction::AddChoice;
            action.treeName = m_currentTreeName;
            action.nodeId = nodeId;
            action.choiceIndex = node.choices.size();
            action.newChoice = choice;
            
            node.choices.push_back(choice);
            
            addUndoAction(action);
        }

        void DialogueEditor::removeChoice(const std::string& nodeId, int choiceIndex) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId) || !isValidChoiceIndex(nodeId, choiceIndex)) {
                return;
            }
            
            auto& node = tree->nodes[nodeId];
            
            EditorAction action;
            action.type = EditorAction::RemoveChoice;
            action.treeName = m_currentTreeName;
            action.nodeId = nodeId;
            action.choiceIndex = choiceIndex;
            action.oldChoice = node.choices[choiceIndex];
            
            node.choices.erase(node.choices.begin() + choiceIndex);
            
            addUndoAction(action);
        }

        void DialogueEditor::updateChoice(const std::string& nodeId, int choiceIndex, const DialogueChoice& choice) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId) || !isValidChoiceIndex(nodeId, choiceIndex)) {
                return;
            }
            
            auto& node = tree->nodes[nodeId];
            
            EditorAction action;
            action.type = EditorAction::UpdateChoice;
            action.treeName = m_currentTreeName;
            action.nodeId = nodeId;
            action.choiceIndex = choiceIndex;
            action.oldChoice = node.choices[choiceIndex];
            action.newChoice = choice;
            
            node.choices[choiceIndex] = choice;
            
            addUndoAction(action);
        }

        void DialogueEditor::moveChoice(const std::string& nodeId, int fromIndex, int toIndex) {
            auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId) || 
                !isValidChoiceIndex(nodeId, fromIndex) || 
                !isValidChoiceIndex(nodeId, toIndex) || 
                fromIndex == toIndex) {
                return;
            }
            
            auto& node = tree->nodes[nodeId];
            DialogueChoice choice = node.choices[fromIndex];
            node.choices.erase(node.choices.begin() + fromIndex);
            node.choices.insert(node.choices.begin() + toIndex, choice);
        }

        std::vector<DialogueChoice> DialogueEditor::getChoices(const std::string& nodeId) const {
            const auto* tree = getCurrentTree();
            if (tree && hasNode(nodeId)) {
                auto it = tree->nodes.find(nodeId);
                if (it != tree->nodes.end()) {
                    return it->second.choices;
                }
            }
            return std::vector<DialogueChoice>();
        }

        int DialogueEditor::getChoiceCount(const std::string& nodeId) const {
            return static_cast<int>(getChoices(nodeId).size());
        }

        void DialogueEditor::setStartNode(const std::string& nodeId) {
            auto* tree = getCurrentTree();
            if (tree && (nodeId.empty() || hasNode(nodeId))) {
                EditorAction action;
                action.type = EditorAction::SetStartNode;
                action.treeName = m_currentTreeName;
                action.oldValue = tree->startNodeId;
                action.newValue = nodeId;
                
                tree->startNodeId = nodeId;
                
                addUndoAction(action);
            }
        }

        std::string DialogueEditor::getStartNode() const {
            const auto* tree = getCurrentTree();
            return tree ? tree->startNodeId : "";
        }

        void DialogueEditor::setVariable(const std::string& name, const std::string& value) {
            auto* tree = getCurrentTree();
            if (tree) {
                EditorAction action;
                action.type = EditorAction::SetVariable;
                action.treeName = m_currentTreeName;
                action.oldValue = tree->variables[name];
                action.newValue = value;
                
                tree->variables[name] = value;
                
                addUndoAction(action);
            }
        }

        std::string DialogueEditor::getVariable(const std::string& name) const {
            const auto* tree = getCurrentTree();
            if (tree) {
                auto it = tree->variables.find(name);
                if (it != tree->variables.end()) {
                    return it->second;
                }
            }
            return "";
        }

        void DialogueEditor::removeVariable(const std::string& name) {
            auto* tree = getCurrentTree();
            if (tree) {
                tree->variables.erase(name);
            }
        }

        std::unordered_map<std::string, std::string> DialogueEditor::getAllVariables() const {
            const auto* tree = getCurrentTree();
            return tree ? tree->variables : std::unordered_map<std::string, std::string>();
        }

        bool DialogueEditor::validateDialogueTree(const std::string& treeName) const {
            return getValidationErrors(treeName).empty();
        }

        std::vector<std::string> DialogueEditor::getValidationErrors(const std::string& treeName) const {
            std::vector<std::string> errors;
            
            const auto* tree = findDialogueTree(treeName);
            if (!tree) {
                errors.push_back("Dialogue tree not found");
                return errors;
            }
            
            if (tree->nodes.empty()) {
                errors.push_back("Dialogue tree has no nodes");
                return errors;
            }
            
            if (tree->startNodeId.empty()) {
                errors.push_back("No start node set");
            } else if (tree->nodes.find(tree->startNodeId) == tree->nodes.end()) {
                errors.push_back("Start node does not exist");
            }
            
            // Check each node
            for (const auto& nodePair : tree->nodes) {
                auto nodeErrors = getNodeValidationErrors(nodePair.first);
                errors.insert(errors.end(), nodeErrors.begin(), nodeErrors.end());
            }
            
            return errors;
        }

        bool DialogueEditor::validateNode(const std::string& nodeId) const {
            return getNodeValidationErrors(nodeId).empty();
        }

        std::vector<std::string> DialogueEditor::getNodeValidationErrors(const std::string& nodeId) const {
            std::vector<std::string> errors;
            
            const auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                errors.push_back("Node not found");
                return errors;
            }
            
            const auto& node = tree->nodes.at(nodeId);
            
            if (node.text.empty()) {
                errors.push_back("Node " + nodeId + " has empty text");
            }
            
            if (!node.isEndNode && node.choices.empty() && node.nextNodeId.empty()) {
                errors.push_back("Node " + nodeId + " has no exit (choices or next node)");
            }
            
            // Validate choice targets
            for (size_t i = 0; i < node.choices.size(); ++i) {
                const auto& choice = node.choices[i];
                if (!choice.targetNodeId.empty() && tree->nodes.find(choice.targetNodeId) == tree->nodes.end()) {
                    errors.push_back("Node " + nodeId + " choice " + std::to_string(i) + " targets non-existent node");
                }
            }
            
            // Validate next node
            if (!node.nextNodeId.empty() && tree->nodes.find(node.nextNodeId) == tree->nodes.end()) {
                errors.push_back("Node " + nodeId + " next node does not exist");
            }
            
            return errors;
        }

        std::vector<std::string> DialogueEditor::getConnectedNodes(const std::string& nodeId) const {
            std::vector<std::string> connected;
            
            const auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                return connected;
            }
            
            const auto& node = tree->nodes.at(nodeId);
            
            if (!node.nextNodeId.empty()) {
                connected.push_back(node.nextNodeId);
            }
            
            for (const auto& choice : node.choices) {
                if (!choice.targetNodeId.empty()) {
                    connected.push_back(choice.targetNodeId);
                }
            }
            
            return connected;
        }

        std::vector<std::string> DialogueEditor::getNodesConnectedTo(const std::string& nodeId) const {
            std::vector<std::string> connectedTo;
            
            const auto* tree = getCurrentTree();
            if (!tree) {
                return connectedTo;
            }
            
            for (const auto& nodePair : tree->nodes) {
                const auto& node = nodePair.second;
                
                if (node.nextNodeId == nodeId) {
                    connectedTo.push_back(node.id);
                    continue;
                }
                
                for (const auto& choice : node.choices) {
                    if (choice.targetNodeId == nodeId) {
                        connectedTo.push_back(node.id);
                        break;
                    }
                }
            }
            
            return connectedTo;
        }

        bool DialogueEditor::areNodesConnected(const std::string& fromNodeId, const std::string& toNodeId) const {
            auto connected = getConnectedNodes(fromNodeId);
            return std::find(connected.begin(), connected.end(), toNodeId) != connected.end();
        }

        bool DialogueEditor::exportDialogueTree(const std::string& treeName, const std::string& filepath) {
            const auto* tree = findDialogueTree(treeName);
            if (!tree) {
                return false;
            }
            
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            // Export in JSON-like format
            file << "{\n";
            file << "  \"name\": \"" << tree->name << "\",\n";
            file << "  \"startNode\": \"" << tree->startNodeId << "\",\n";
            file << "  \"nodes\": {\n";
            
            bool firstNode = true;
            for (const auto& nodePair : tree->nodes) {
                if (!firstNode) file << ",\n";
                firstNode = false;
                
                const auto& node = nodePair.second;
                file << "    \"" << node.id << "\": {\n";
                file << "      \"speaker\": \"" << node.speakerName << "\",\n";
                file << "      \"text\": \"" << node.text << "\",\n";
                file << "      \"isEnd\": " << (node.isEndNode ? "true" : "false") << ",\n";
                
                if (!node.nextNodeId.empty()) {
                    file << "      \"next\": \"" << node.nextNodeId << "\",\n";
                }
                
                if (!node.choices.empty()) {
                    file << "      \"choices\": [\n";
                    for (size_t i = 0; i < node.choices.size(); ++i) {
                        if (i > 0) file << ",\n";
                        const auto& choice = node.choices[i];
                        file << "        {\n";
                        file << "          \"text\": \"" << choice.text << "\",\n";
                        file << "          \"target\": \"" << choice.targetNodeId << "\"\n";
                        file << "        }";
                    }
                    file << "\n      ]\n";
                } else {
                    file << "      \"choices\": []\n";
                }
                
                file << "    }";
            }
            
            file << "\n  }\n";
            file << "}\n";
            
            return true;
        }

        bool DialogueEditor::exportAllDialogueTrees(const std::string& directory) {
            for (const auto& tree : m_dialogueTrees) {
                std::string filepath = directory + "/" + tree.name + ".json";
                if (!exportDialogueTree(tree.name, filepath)) {
                    return false;
                }
            }
            return true;
        }

        void DialogueEditor::startPreview(const std::string& treeName) {
            const auto* tree = findDialogueTree(treeName);
            if (tree && !tree->startNodeId.empty()) {
                m_previewTreeName = treeName;
                m_currentPreviewNodeId = tree->startNodeId;
                m_previewActive = true;
            }
        }

        void DialogueEditor::stopPreview() {
            m_previewActive = false;
            m_previewTreeName.clear();
            m_currentPreviewNodeId.clear();
        }

        bool DialogueEditor::isPreviewActive() const {
            return m_previewActive;
        }

        std::string DialogueEditor::getCurrentPreviewNode() const {
            return m_currentPreviewNodeId;
        }

        std::vector<DialogueChoice> DialogueEditor::getCurrentPreviewChoices() const {
            if (!m_previewActive) {
                return std::vector<DialogueChoice>();
            }
            
            const auto* tree = findDialogueTree(m_previewTreeName);
            if (tree) {
                auto it = tree->nodes.find(m_currentPreviewNodeId);
                if (it != tree->nodes.end()) {
                    return it->second.choices;
                }
            }
            
            return std::vector<DialogueChoice>();
        }

        void DialogueEditor::selectPreviewChoice(int choiceIndex) {
            if (!m_previewActive) {
                return;
            }
            
            auto choices = getCurrentPreviewChoices();
            if (choiceIndex >= 0 && choiceIndex < static_cast<int>(choices.size())) {
                m_currentPreviewNodeId = choices[choiceIndex].targetNodeId;
            }
        }

        void DialogueEditor::advancePreview() {
            if (!m_previewActive) {
                return;
            }
            
            const auto* tree = findDialogueTree(m_previewTreeName);
            if (tree) {
                auto it = tree->nodes.find(m_currentPreviewNodeId);
                if (it != tree->nodes.end()) {
                    const auto& node = it->second;
                    if (!node.nextNodeId.empty()) {
                        m_currentPreviewNodeId = node.nextNodeId;
                    } else if (node.isEndNode) {
                        stopPreview();
                    }
                }
            }
        }

        void DialogueEditor::undo() {
            if (m_undoStack.empty()) {
                return;
            }
            
            // Simplified undo implementation
            EditorAction action = m_undoStack.back();
            m_undoStack.pop_back();
            m_redoStack.push_back(action);
            
            // Reverse the action (simplified)
            switch (action.type) {
                case EditorAction::CreateTree:
                    deleteDialogueTree(action.treeName);
                    break;
                case EditorAction::DeleteTree:
                    createDialogueTree(action.treeName);
                    break;
                // Add other action reversals as needed
                default:
                    break;
            }
        }

        void DialogueEditor::redo() {
            if (m_redoStack.empty()) {
                return;
            }
            
            // Simplified redo implementation
            EditorAction action = m_redoStack.back();
            m_redoStack.pop_back();
            m_undoStack.push_back(action);
            
            // Reapply the action (simplified)
            switch (action.type) {
                case EditorAction::CreateTree:
                    createDialogueTree(action.treeName);
                    break;
                case EditorAction::DeleteTree:
                    deleteDialogueTree(action.treeName);
                    break;
                // Add other action reapplications as needed
                default:
                    break;
            }
        }

        bool DialogueEditor::canUndo() const {
            return !m_undoStack.empty();
        }

        bool DialogueEditor::canRedo() const {
            return !m_redoStack.empty();
        }

        DialogueTree* DialogueEditor::findDialogueTree(const std::string& name) {
            auto it = std::find_if(m_dialogueTrees.begin(), m_dialogueTrees.end(),
                [&name](const DialogueTree& tree) { return tree.name == name; });
            return it != m_dialogueTrees.end() ? &(*it) : nullptr;
        }

        const DialogueTree* DialogueEditor::findDialogueTree(const std::string& name) const {
            auto it = std::find_if(m_dialogueTrees.begin(), m_dialogueTrees.end(),
                [&name](const DialogueTree& tree) { return tree.name == name; });
            return it != m_dialogueTrees.end() ? &(*it) : nullptr;
        }

        DialogueTree* DialogueEditor::getCurrentTree() {
            return findDialogueTree(m_currentTreeName);
        }

        const DialogueTree* DialogueEditor::getCurrentTree() const {
            return findDialogueTree(m_currentTreeName);
        }

        std::string DialogueEditor::generateNodeId() {
            return "node_" + std::to_string(m_nextNodeId++);
        }

        void DialogueEditor::addUndoAction(const EditorAction& action) {
            m_undoStack.push_back(action);
            
            if (m_undoStack.size() > m_maxUndoSteps) {
                m_undoStack.erase(m_undoStack.begin());
            }
            
            clearRedoStack();
        }

        void DialogueEditor::clearRedoStack() {
            m_redoStack.clear();
        }

        bool DialogueEditor::isValidChoiceIndex(const std::string& nodeId, int index) const {
            const auto* tree = getCurrentTree();
            if (!tree || !hasNode(nodeId)) {
                return false;
            }
            
            const auto& node = tree->nodes.at(nodeId);
            return index >= 0 && index < static_cast<int>(node.choices.size());
        }

    } // namespace Tools
} // namespace Engine