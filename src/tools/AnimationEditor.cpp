#include "AnimationEditor.h"
#include <fstream>
#include <algorithm>
#include <iostream>

namespace Engine {
    namespace Tools {

        AnimationEditor::AnimationEditor()
            : m_spriteSheet{"", 32, 32, 1, 1, 1}
            , m_previewPlaying(false)
            , m_previewPaused(false)
            , m_previewTime(0.0f)
            , m_currentPreviewFrame(0)
            , m_maxUndoSteps(100) {
        }

        AnimationEditor::~AnimationEditor() {
        }

        bool AnimationEditor::createNewProject() {
            m_animations.clear();
            m_spriteSheet = {"", 32, 32, 1, 1, 1};
            m_undoStack.clear();
            m_redoStack.clear();
            stopPreview();
            return true;
        }

        bool AnimationEditor::loadProject(const std::string& filepath) {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            // Simplified project loading
            createNewProject();
            
            // In a real implementation, this would parse JSON or XML
            std::string line;
            while (std::getline(file, line)) {
                if (line.find("spritesheet=") == 0) {
                    m_spriteSheet.filepath = line.substr(12);
                } else if (line.find("framesize=") == 0) {
                    // Parse frame size
                    size_t comma = line.find(',', 10);
                    if (comma != std::string::npos) {
                        m_spriteSheet.frameWidth = std::stoi(line.substr(10, comma - 10));
                        m_spriteSheet.frameHeight = std::stoi(line.substr(comma + 1));
                    }
                }
            }
            
            return true;
        }

        bool AnimationEditor::saveProject(const std::string& filepath) {
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            // Simplified project saving
            file << "# Animation Editor Project\n";
            file << "spritesheet=" << m_spriteSheet.filepath << "\n";
            file << "framesize=" << m_spriteSheet.frameWidth << "," << m_spriteSheet.frameHeight << "\n";
            file << "columns=" << m_spriteSheet.columns << "\n";
            file << "rows=" << m_spriteSheet.rows << "\n";
            
            for (const auto& animation : m_animations) {
                file << "animation=" << animation.name << ",looping=" << animation.looping << "\n";
                for (size_t i = 0; i < animation.frames.size(); ++i) {
                    const auto& frame = animation.frames[i];
                    file << "frame=" << i << ",sprite=" << frame.spriteIndex 
                         << ",duration=" << frame.duration 
                         << ",offset=" << frame.offsetX << "," << frame.offsetY;
                    if (!frame.eventName.empty()) {
                        file << ",event=" << frame.eventName;
                    }
                    file << "\n";
                }
            }
            
            return true;
        }

        bool AnimationEditor::loadSpriteSheet(const std::string& filepath) {
            m_spriteSheet.filepath = filepath;
            // In a real implementation, this would load the image and detect dimensions
            return true;
        }

        void AnimationEditor::setSpriteSheetProperties(int frameWidth, int frameHeight, int columns, int rows) {
            m_spriteSheet.frameWidth = frameWidth;
            m_spriteSheet.frameHeight = frameHeight;
            m_spriteSheet.columns = columns;
            m_spriteSheet.rows = rows;
            m_spriteSheet.totalFrames = columns * rows;
        }

        SpriteSheet AnimationEditor::getSpriteSheetInfo() const {
            return m_spriteSheet;
        }

        bool AnimationEditor::hasSpriteSheet() const {
            return !m_spriteSheet.filepath.empty();
        }

        void AnimationEditor::createAnimation(const std::string& name) {
            if (hasAnimation(name)) {
                return; // Animation already exists
            }
            
            AnimationSequence newAnimation;
            newAnimation.name = name;
            newAnimation.looping = true;
            newAnimation.totalDuration = 0.0f;
            
            m_animations.push_back(newAnimation);
            
            EditorAction action;
            action.type = EditorAction::CreateAnimation;
            action.animationName = name;
            addUndoAction(action);
        }

        void AnimationEditor::deleteAnimation(const std::string& name) {
            auto it = std::find_if(m_animations.begin(), m_animations.end(),
                [&name](const AnimationSequence& anim) { return anim.name == name; });
            
            if (it != m_animations.end()) {
                EditorAction action;
                action.type = EditorAction::DeleteAnimation;
                action.animationName = name;
                
                m_animations.erase(it);
                
                // Stop preview if this animation was being previewed
                if (m_previewAnimation == name) {
                    stopPreview();
                }
                
                addUndoAction(action);
            }
        }

        void AnimationEditor::renameAnimation(const std::string& oldName, const std::string& newName) {
            if (oldName == newName || hasAnimation(newName)) {
                return;
            }
            
            auto* animation = findAnimation(oldName);
            if (animation) {
                EditorAction action;
                action.type = EditorAction::RenameAnimation;
                action.oldName = oldName;
                action.newName = newName;
                
                animation->name = newName;
                
                // Update preview if necessary
                if (m_previewAnimation == oldName) {
                    m_previewAnimation = newName;
                }
                
                addUndoAction(action);
            }
        }

        std::vector<std::string> AnimationEditor::getAnimationNames() const {
            std::vector<std::string> names;
            for (const auto& animation : m_animations) {
                names.push_back(animation.name);
            }
            return names;
        }

        bool AnimationEditor::hasAnimation(const std::string& name) const {
            return findAnimation(name) != nullptr;
        }

        void AnimationEditor::addFrame(const std::string& animationName, const AnimationFrame& frame) {
            auto* animation = findAnimation(animationName);
            if (animation) {
                EditorAction action;
                action.type = EditorAction::AddFrame;
                action.animationName = animationName;
                action.frameIndex = animation->frames.size();
                action.newFrame = frame;
                
                animation->frames.push_back(frame);
                updateAnimationDuration(*animation);
                
                addUndoAction(action);
            }
        }

        void AnimationEditor::insertFrame(const std::string& animationName, int index, const AnimationFrame& frame) {
            auto* animation = findAnimation(animationName);
            if (animation && index >= 0 && index <= static_cast<int>(animation->frames.size())) {
                EditorAction action;
                action.type = EditorAction::AddFrame;
                action.animationName = animationName;
                action.frameIndex = index;
                action.newFrame = frame;
                
                animation->frames.insert(animation->frames.begin() + index, frame);
                updateAnimationDuration(*animation);
                
                addUndoAction(action);
            }
        }

        void AnimationEditor::removeFrame(const std::string& animationName, int index) {
            auto* animation = findAnimation(animationName);
            if (animation && isValidFrameIndex(animationName, index)) {
                EditorAction action;
                action.type = EditorAction::RemoveFrame;
                action.animationName = animationName;
                action.frameIndex = index;
                action.oldFrame = animation->frames[index];
                
                animation->frames.erase(animation->frames.begin() + index);
                updateAnimationDuration(*animation);
                
                addUndoAction(action);
            }
        }

        void AnimationEditor::moveFrame(const std::string& animationName, int fromIndex, int toIndex) {
            auto* animation = findAnimation(animationName);
            if (animation && isValidFrameIndex(animationName, fromIndex) && 
                toIndex >= 0 && toIndex < static_cast<int>(animation->frames.size()) && 
                fromIndex != toIndex) {
                
                EditorAction action;
                action.type = EditorAction::MoveFrame;
                action.animationName = animationName;
                action.frameIndex = fromIndex;
                action.targetIndex = toIndex;
                
                AnimationFrame frame = animation->frames[fromIndex];
                animation->frames.erase(animation->frames.begin() + fromIndex);
                animation->frames.insert(animation->frames.begin() + toIndex, frame);
                
                addUndoAction(action);
            }
        }

        void AnimationEditor::updateFrame(const std::string& animationName, int index, const AnimationFrame& frame) {
            auto* animation = findAnimation(animationName);
            if (animation && isValidFrameIndex(animationName, index)) {
                EditorAction action;
                action.type = EditorAction::UpdateFrame;
                action.animationName = animationName;
                action.frameIndex = index;
                action.oldFrame = animation->frames[index];
                action.newFrame = frame;
                
                animation->frames[index] = frame;
                updateAnimationDuration(*animation);
                
                addUndoAction(action);
            }
        }

        AnimationFrame AnimationEditor::getFrame(const std::string& animationName, int index) const {
            const auto* animation = findAnimation(animationName);
            if (animation && isValidFrameIndex(animationName, index)) {
                return animation->frames[index];
            }
            return {0, 0.1f, 0, 0, ""};
        }

        int AnimationEditor::getFrameCount(const std::string& animationName) const {
            const auto* animation = findAnimation(animationName);
            return animation ? static_cast<int>(animation->frames.size()) : 0;
        }

        std::vector<AnimationFrame> AnimationEditor::getFrames(const std::string& animationName) const {
            const auto* animation = findAnimation(animationName);
            return animation ? animation->frames : std::vector<AnimationFrame>();
        }

        void AnimationEditor::setAnimationLooping(const std::string& animationName, bool looping) {
            auto* animation = findAnimation(animationName);
            if (animation) {
                EditorAction action;
                action.type = EditorAction::SetLooping;
                action.animationName = animationName;
                action.oldValue = animation->looping;
                action.newValue = looping;
                
                animation->looping = looping;
                
                addUndoAction(action);
            }
        }

        bool AnimationEditor::isAnimationLooping(const std::string& animationName) const {
            const auto* animation = findAnimation(animationName);
            return animation ? animation->looping : false;
        }

        float AnimationEditor::getAnimationDuration(const std::string& animationName) const {
            const auto* animation = findAnimation(animationName);
            return animation ? animation->totalDuration : 0.0f;
        }

        void AnimationEditor::startPreview(const std::string& animationName) {
            if (hasAnimation(animationName)) {
                m_previewAnimation = animationName;
                m_previewPlaying = true;
                m_previewPaused = false;
                m_previewTime = 0.0f;
                m_currentPreviewFrame = 0;
            }
        }

        void AnimationEditor::stopPreview() {
            m_previewPlaying = false;
            m_previewPaused = false;
            m_previewTime = 0.0f;
            m_currentPreviewFrame = 0;
            m_previewAnimation.clear();
        }

        void AnimationEditor::pausePreview() {
            m_previewPaused = true;
        }

        void AnimationEditor::resumePreview() {
            m_previewPaused = false;
        }

        bool AnimationEditor::isPreviewPlaying() const {
            return m_previewPlaying && !m_previewPaused;
        }

        std::string AnimationEditor::getCurrentPreviewAnimation() const {
            return m_previewAnimation;
        }

        int AnimationEditor::getCurrentPreviewFrame() const {
            return m_currentPreviewFrame;
        }

        float AnimationEditor::getPreviewTime() const {
            return m_previewTime;
        }

        void AnimationEditor::updatePreview(float deltaTime) {
            if (!isPreviewPlaying() || m_previewAnimation.empty()) {
                return;
            }
            
            const auto* animation = findAnimation(m_previewAnimation);
            if (!animation || animation->frames.empty()) {
                return;
            }
            
            m_previewTime += deltaTime;
            
            // Find current frame based on time
            float accumulatedTime = 0.0f;
            for (size_t i = 0; i < animation->frames.size(); ++i) {
                accumulatedTime += animation->frames[i].duration;
                if (m_previewTime <= accumulatedTime) {
                    m_currentPreviewFrame = static_cast<int>(i);
                    return;
                }
            }
            
            // Animation finished
            if (animation->looping) {
                m_previewTime = 0.0f;
                m_currentPreviewFrame = 0;
            } else {
                stopPreview();
            }
        }

        bool AnimationEditor::exportAnimation(const std::string& animationName, const std::string& filepath) {
            const auto* animation = findAnimation(animationName);
            if (!animation) {
                return false;
            }
            
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            // Export in a simple format
            file << "# Animation: " << animationName << "\n";
            file << "looping=" << animation->looping << "\n";
            file << "duration=" << animation->totalDuration << "\n";
            file << "frames=" << animation->frames.size() << "\n";
            
            for (size_t i = 0; i < animation->frames.size(); ++i) {
                const auto& frame = animation->frames[i];
                file << "frame=" << i << ",sprite=" << frame.spriteIndex 
                     << ",duration=" << frame.duration 
                     << ",offset=" << frame.offsetX << "," << frame.offsetY;
                if (!frame.eventName.empty()) {
                    file << ",event=" << frame.eventName;
                }
                file << "\n";
            }
            
            return true;
        }

        bool AnimationEditor::exportAllAnimations(const std::string& directory) {
            for (const auto& animation : m_animations) {
                std::string filepath = directory + "/" + animation.name + ".anim";
                if (!exportAnimation(animation.name, filepath)) {
                    return false;
                }
            }
            return true;
        }

        bool AnimationEditor::validateAnimation(const std::string& animationName) const {
            const auto* animation = findAnimation(animationName);
            if (!animation) {
                return false;
            }
            
            if (animation->frames.empty()) {
                return false;
            }
            
            for (const auto& frame : animation->frames) {
                if (frame.duration <= 0.0f) {
                    return false;
                }
                if (frame.spriteIndex < 0 || frame.spriteIndex >= m_spriteSheet.totalFrames) {
                    return false;
                }
            }
            
            return true;
        }

        std::vector<std::string> AnimationEditor::getValidationErrors(const std::string& animationName) const {
            std::vector<std::string> errors;
            
            const auto* animation = findAnimation(animationName);
            if (!animation) {
                errors.push_back("Animation not found");
                return errors;
            }
            
            if (animation->frames.empty()) {
                errors.push_back("Animation has no frames");
            }
            
            for (size_t i = 0; i < animation->frames.size(); ++i) {
                const auto& frame = animation->frames[i];
                if (frame.duration <= 0.0f) {
                    errors.push_back("Frame " + std::to_string(i) + " has invalid duration");
                }
                if (frame.spriteIndex < 0 || frame.spriteIndex >= m_spriteSheet.totalFrames) {
                    errors.push_back("Frame " + std::to_string(i) + " has invalid sprite index");
                }
            }
            
            return errors;
        }

        void AnimationEditor::undo() {
            if (m_undoStack.empty()) {
                return;
            }
            
            EditorAction action = m_undoStack.back();
            m_undoStack.pop_back();
            
            // Reverse the action (simplified implementation)
            switch (action.type) {
                case EditorAction::CreateAnimation:
                    deleteAnimation(action.animationName);
                    break;
                case EditorAction::DeleteAnimation:
                    createAnimation(action.animationName);
                    break;
                case EditorAction::RenameAnimation:
                    renameAnimation(action.newName, action.oldName);
                    break;
                // Add other action reversals as needed
                default:
                    break;
            }
            
            m_redoStack.push_back(action);
        }

        void AnimationEditor::redo() {
            if (m_redoStack.empty()) {
                return;
            }
            
            EditorAction action = m_redoStack.back();
            m_redoStack.pop_back();
            
            // Reapply the action (simplified implementation)
            switch (action.type) {
                case EditorAction::CreateAnimation:
                    createAnimation(action.animationName);
                    break;
                case EditorAction::DeleteAnimation:
                    deleteAnimation(action.animationName);
                    break;
                case EditorAction::RenameAnimation:
                    renameAnimation(action.oldName, action.newName);
                    break;
                // Add other action reapplications as needed
                default:
                    break;
            }
            
            m_undoStack.push_back(action);
        }

        bool AnimationEditor::canUndo() const {
            return !m_undoStack.empty();
        }

        bool AnimationEditor::canRedo() const {
            return !m_redoStack.empty();
        }

        AnimationSequence* AnimationEditor::findAnimation(const std::string& name) {
            auto it = std::find_if(m_animations.begin(), m_animations.end(),
                [&name](const AnimationSequence& anim) { return anim.name == name; });
            return it != m_animations.end() ? &(*it) : nullptr;
        }

        const AnimationSequence* AnimationEditor::findAnimation(const std::string& name) const {
            auto it = std::find_if(m_animations.begin(), m_animations.end(),
                [&name](const AnimationSequence& anim) { return anim.name == name; });
            return it != m_animations.end() ? &(*it) : nullptr;
        }

        void AnimationEditor::updateAnimationDuration(AnimationSequence& animation) {
            animation.totalDuration = 0.0f;
            for (const auto& frame : animation.frames) {
                animation.totalDuration += frame.duration;
            }
        }

        void AnimationEditor::addUndoAction(const EditorAction& action) {
            m_undoStack.push_back(action);
            
            if (m_undoStack.size() > m_maxUndoSteps) {
                m_undoStack.erase(m_undoStack.begin());
            }
            
            clearRedoStack();
        }

        void AnimationEditor::clearRedoStack() {
            m_redoStack.clear();
        }

        bool AnimationEditor::isValidFrameIndex(const std::string& animationName, int index) const {
            const auto* animation = findAnimation(animationName);
            return animation && index >= 0 && index < static_cast<int>(animation->frames.size());
        }

    } // namespace Tools
} // namespace Engine