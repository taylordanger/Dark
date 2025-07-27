#pragma once

#include <vector>
#include <string>
#include <memory>
#include "../graphics/AnimationComponent.h"

namespace Engine {
    namespace Tools {

        struct AnimationFrame {
            int spriteIndex;
            float duration;
            int offsetX;
            int offsetY;
            std::string eventName; // Optional event to trigger
        };

        struct AnimationSequence {
            std::string name;
            std::vector<AnimationFrame> frames;
            bool looping;
            float totalDuration;
        };

        struct SpriteSheet {
            std::string filepath;
            int frameWidth;
            int frameHeight;
            int columns;
            int rows;
            int totalFrames;
        };

        class AnimationEditor {
        public:
            AnimationEditor();
            ~AnimationEditor();

            // Project operations
            bool createNewProject();
            bool loadProject(const std::string& filepath);
            bool saveProject(const std::string& filepath);

            // Sprite sheet operations
            bool loadSpriteSheet(const std::string& filepath);
            void setSpriteSheetProperties(int frameWidth, int frameHeight, int columns, int rows);
            SpriteSheet getSpriteSheetInfo() const;
            bool hasSpriteSheet() const;

            // Animation sequence operations
            void createAnimation(const std::string& name);
            void deleteAnimation(const std::string& name);
            void renameAnimation(const std::string& oldName, const std::string& newName);
            std::vector<std::string> getAnimationNames() const;
            bool hasAnimation(const std::string& name) const;

            // Frame operations
            void addFrame(const std::string& animationName, const AnimationFrame& frame);
            void insertFrame(const std::string& animationName, int index, const AnimationFrame& frame);
            void removeFrame(const std::string& animationName, int index);
            void moveFrame(const std::string& animationName, int fromIndex, int toIndex);
            void updateFrame(const std::string& animationName, int index, const AnimationFrame& frame);
            
            AnimationFrame getFrame(const std::string& animationName, int index) const;
            int getFrameCount(const std::string& animationName) const;
            std::vector<AnimationFrame> getFrames(const std::string& animationName) const;

            // Animation properties
            void setAnimationLooping(const std::string& animationName, bool looping);
            bool isAnimationLooping(const std::string& animationName) const;
            float getAnimationDuration(const std::string& animationName) const;

            // Preview and playback
            void startPreview(const std::string& animationName);
            void stopPreview();
            void pausePreview();
            void resumePreview();
            bool isPreviewPlaying() const;
            std::string getCurrentPreviewAnimation() const;
            int getCurrentPreviewFrame() const;
            float getPreviewTime() const;
            void updatePreview(float deltaTime);

            // Export
            bool exportAnimation(const std::string& animationName, const std::string& filepath);
            bool exportAllAnimations(const std::string& directory);

            // Validation
            bool validateAnimation(const std::string& animationName) const;
            std::vector<std::string> getValidationErrors(const std::string& animationName) const;

            // Undo/Redo
            void undo();
            void redo();
            bool canUndo() const;
            bool canRedo() const;

        private:
            struct EditorAction {
                enum Type { 
                    AddFrame, RemoveFrame, UpdateFrame, MoveFrame,
                    CreateAnimation, DeleteAnimation, RenameAnimation,
                    SetLooping
                };
                Type type;
                std::string animationName;
                std::string oldName, newName;
                int frameIndex, targetIndex;
                AnimationFrame oldFrame, newFrame;
                bool oldValue, newValue;
            };

            SpriteSheet m_spriteSheet;
            std::vector<AnimationSequence> m_animations;
            
            // Preview state
            std::string m_previewAnimation;
            bool m_previewPlaying;
            bool m_previewPaused;
            float m_previewTime;
            int m_currentPreviewFrame;
            
            // Undo/Redo
            std::vector<EditorAction> m_undoStack;
            std::vector<EditorAction> m_redoStack;
            size_t m_maxUndoSteps;

            // Helper methods
            AnimationSequence* findAnimation(const std::string& name);
            const AnimationSequence* findAnimation(const std::string& name) const;
            void updateAnimationDuration(AnimationSequence& animation);
            void addUndoAction(const EditorAction& action);
            void clearRedoStack();
            bool isValidFrameIndex(const std::string& animationName, int index) const;
        };

    } // namespace Tools
} // namespace Engine