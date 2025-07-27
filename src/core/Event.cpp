#include "Event.h"
#include <algorithm>

namespace RPGEngine {
    
    void EventDispatcher::processQueuedEvents() {
        // Process all queued events
        for (auto& event : m_eventQueue) {
            auto typeIndex = event->getType();
            auto it = m_handlers.find(typeIndex);
            
            if (it != m_handlers.end()) {
                for (const auto& wrapper : it->second) {
                    wrapper.handler(*event);
                }
            }
        }
        
        // Clear the queue after processing
        m_eventQueue.clear();
    }
    
    void EventDispatcher::clear() {
        m_handlers.clear();
        m_eventQueue.clear();
        m_nextHandlerId = 1;
    }
    
} // namespace RPGEngine