#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace Engine {
    namespace Tools {

        struct DialogueChoice {
            std::string text;
            std::string targetNodeId;
            std::string condition; // Optional condition script
            bool enabled;
        };

        struct DialogueNode {
            std::string id;
            std::string speakerName;
            std::string text;
            std::vector<DialogueChoice> choices;
            std::string nextNodeId; // For linear dialogue
            std::string script; // Optional script to execute
            bool isEndNode;
            float x, y; // Position in editor
        };

        struct DialogueTree {
            std::string name;
            std::string startNodeId;
            std::unordered_map<std::string, DialogueNode> nodes;
            std::unordered_map<std::string, std::string> variables; // Dialogue variables
        };

        class DialogueEditor {
        public:
            DialogueEditor();
            ~DialogueEditor();

            // Project operations
            bool createNewProject();
            bool loadProject(const std::string& filepath);
            bool saveProject(const std::string& filepath);

            // Dialogue tree operations
            void createDialogueTree(const std::string& name);
            void deleteDialogueTree(const std::string& name);
            void renameDialogueTree(const std::string& oldName, const std::string& newName);
            std::vector<std::string> getDialogueTreeNames() const;
            bool hasDialogueTree(const std::string& name) const;
            void setCurrentDialogueTree(const std::string& name);
            std::string getCurrentDialogueTree() const;

            // Node operations
            std::string createNode(float x = 0.0f, float y = 0.0f);
            void deleteNode(const std::string& nodeId);
            void moveNode(const std::string& nodeId, float x, float y);
            bool hasNode(const std::string& nodeId) const;
            DialogueNode getNode(const std::string& nodeId) const;
            std::vector<std::string> getAllNodeIds() const;

            // Node properties
            void setNodeSpeaker(const std::string& nodeId, const std::string& speaker);
            void setNodeText(const std::string& nodeId, const std::string& text);
            void setNodeScript(const std::string& nodeId, const std::string& script);
            void setNodeAsEnd(const std::string& nodeId, bool isEnd);
            void setNodeNext(const std::string& nodeId, const std::string& nextNodeId);

            // Choice operations
            void addChoice(const std::string& nodeId, const DialogueChoice& choice);
            void removeChoice(const std::string& nodeId, int choiceIndex);
            void updateChoice(const std::string& nodeId, int choiceIndex, const DialogueChoice& choice);
            void moveChoice(const std::string& nodeId, int fromIndex, int toIndex);
            std::vector<DialogueChoice> getChoices(const std::string& nodeId) const;
            int getChoiceCount(const std::string& nodeId) const;

            // Tree properties
            void setStartNode(const std::string& nodeId);
            std::string getStartNode() const;
            void setVariable(const std::string& name, const std::string& value);
            std::string getVariable(const std::string& name) const;
            void removeVariable(const std::string& name);
            std::unordered_map<std::string, std::string> getAllVariables() const;

            // Validation
            bool validateDialogueTree(const std::string& treeName) const;
            std::vector<std::string> getValidationErrors(const std::string& treeName) const;
            bool validateNode(const std::string& nodeId) const;
            std::vector<std::string> getNodeValidationErrors(const std::string& nodeId) const;

            // Navigation and connections
            std::vector<std::string> getConnectedNodes(const std::string& nodeId) const;
            std::vector<std::string> getNodesConnectedTo(const std::string& nodeId) const;
            bool areNodesConnected(const std::string& fromNodeId, const std::string& toNodeId) const;

            // Export
            bool exportDialogueTree(const std::string& treeName, const std::string& filepath);
            bool exportAllDialogueTrees(const std::string& directory);

            // Preview and testing
            void startPreview(const std::string& treeName);
            void stopPreview();
            bool isPreviewActive() const;
            std::string getCurrentPreviewNode() const;
            std::vector<DialogueChoice> getCurrentPreviewChoices() const;
            void selectPreviewChoice(int choiceIndex);
            void advancePreview();

            // Undo/Redo
            void undo();
            void redo();
            bool canUndo() const;
            bool canRedo() const;

        private:
            struct EditorAction {
                enum Type {
                    CreateNode, DeleteNode, MoveNode,
                    SetNodeProperty, AddChoice, RemoveChoice, UpdateChoice,
                    CreateTree, DeleteTree, RenameTree,
                    SetStartNode, SetVariable
                };
                Type type;
                std::string treeName;
                std::string nodeId;
                std::string oldValue, newValue;
                float oldX, oldY, newX, newY;
                int choiceIndex;
                DialogueChoice oldChoice, newChoice;
            };

            std::vector<DialogueTree> m_dialogueTrees;
            std::string m_currentTreeName;
            
            // Preview state
            std::string m_previewTreeName;
            std::string m_currentPreviewNodeId;
            bool m_previewActive;
            
            // Undo/Redo
            std::vector<EditorAction> m_undoStack;
            std::vector<EditorAction> m_redoStack;
            size_t m_maxUndoSteps;
            
            // ID generation
            int m_nextNodeId;

            // Helper methods
            DialogueTree* findDialogueTree(const std::string& name);
            const DialogueTree* findDialogueTree(const std::string& name) const;
            DialogueTree* getCurrentTree();
            const DialogueTree* getCurrentTree() const;
            std::string generateNodeId();
            void addUndoAction(const EditorAction& action);
            void clearRedoStack();
            bool isValidChoiceIndex(const std::string& nodeId, int index) const;
        };

    } // namespace Tools
} // namespace Engine