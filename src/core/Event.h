#pragma once

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace RPGEngine {
    
    /**
     * Base event class that all events must inherit from
     */
    class IEvent {
    public:
        virtual ~IEvent() = default;
        virtual std::type_index getType() const = 0;
        virtual const std::string& getName() const = 0;
    };
    
    /**
     * Template base class for typed events
     */
    template<typename T>
    class Event : public IEvent {
    public:
        std::type_index getType() const override {
            return std::type_index(typeid(T));
        }
        
        const std::string& getName() const override {
            static std::string name = typeid(T).name();
            return name;
        }
        
        static std::type_index getStaticType() {
            return std::type_index(typeid(T));
        }
    };
    
    /**
     * Event handler function type
     */
    using EventHandler = std::function<void(const IEvent&)>;
    
    /**
     * Event dispatcher for decoupled communication between systems
     */
    class EventDispatcher {
    public:
        EventDispatcher() = default;
        ~EventDispatcher() = default;
        
        template<typename T>
        void getInstance();
        
        /**
         * Subscribe to an event type
         * @param handler Function to call when event is dispatched
         * @return Subscription ID for unsubscribing
         */
        template<typename T>
        size_t subscribe(std::function<void(const T&)> handler);
        
        /**
         * Unsubscribe from an event type
         * @param subscriptionId ID returned from subscribe
         */
        template<typename T>
        void unsubscribe(size_t subscriptionId);
        
        /**
         * Dispatch an event immediately
         * @param event Event to dispatch
         */
        template<typename T>
        void dispatch(const T& event);
        
        /**
         * Queue an event for later processing
         * @param event Event to queue
         */
        template<typename T>
        void queueEvent(std::unique_ptr<T> event);
        
        /**
         * Process all queued events
         */
        void processQueuedEvents();
        
        /**
         * Clear all event handlers and queued events
         */
        void clear();
        
    private:
        struct HandlerWrapper {
            EventHandler handler;
            size_t id;
        };
        
        std::unordered_map<std::type_index, std::vector<HandlerWrapper>> m_handlers;
        std::vector<std::unique_ptr<IEvent>> m_eventQueue;
        size_t m_nextHandlerId = 1;
    };
    
    // Template implementations
    template<typename T>
    size_t EventDispatcher::subscribe(std::function<void(const T&)> handler) {
        static_assert(std::is_base_of_v<IEvent, T>, "T must inherit from IEvent");
        
        auto typeIndex = std::type_index(typeid(T));
        size_t handlerId = m_nextHandlerId++;
        
        // Wrap the typed handler in a generic EventHandler
        EventHandler wrappedHandler = [handler](const IEvent& event) {
            handler(static_cast<const T&>(event));
        };
        
        m_handlers[typeIndex].push_back({wrappedHandler, handlerId});
        return handlerId;
    }
    
    template<typename T>
    void EventDispatcher::unsubscribe(size_t subscriptionId) {
        auto typeIndex = std::type_index(typeid(T));
        auto it = m_handlers.find(typeIndex);
        
        if (it != m_handlers.end()) {
            auto& handlers = it->second;
            handlers.erase(
                std::remove_if(handlers.begin(), handlers.end(),
                    [subscriptionId](const HandlerWrapper& wrapper) {
                        return wrapper.id == subscriptionId;
                    }),
                handlers.end()
            );
        }
    }
    
    template<typename T>
    void EventDispatcher::dispatch(const T& event) {
        static_assert(std::is_base_of_v<IEvent, T>, "T must inherit from IEvent");
        
        auto typeIndex = std::type_index(typeid(T));
        auto it = m_handlers.find(typeIndex);
        
        if (it != m_handlers.end()) {
            for (const auto& wrapper : it->second) {
                wrapper.handler(event);
            }
        }
    }
    
    template<typename T>
    void EventDispatcher::queueEvent(std::unique_ptr<T> event) {
        static_assert(std::is_base_of_v<IEvent, T>, "T must inherit from IEvent");
        m_eventQueue.push_back(std::move(event));
    }
    
} // namespace RPGEngine