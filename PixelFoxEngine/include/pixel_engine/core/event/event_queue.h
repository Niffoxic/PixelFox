// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include "PixelFoxEngineAPI.h"

#include <sal.h>  
#include <functional>
#include <typeindex>

#include "core/vector.h"
#include "core/unordered_map.h"

namespace pixel_engine
{
    //~ For Every Type
    template<typename EventT>
    struct Channel
    {
        using Callback = std::function<void(_In_ const EventT&)>;

        inline static fox::vector<Callback> Subscribers{};   // subscribers
        inline static fox::vector<EventT>   Queue{};  // pending events
    };

    //~ Subscription token
    struct SubToken
    {
        std::type_index type{ typeid(void) };
        std::size_t     index{ static_cast<std::size_t>(-1) };
        bool            valid{ false };

        size_t operator()()
        {
            return index;
        }
    };

    //~ Event queue Facade
    class EventQueue
    {
        struct TypeOps
        {
            void (*dispatch)();                    // calls DispatchType<T>()
            void (*clear)();                       // clears Channel<T>::Queue
            bool (*unsubscribe)(_In_ std::size_t); // clears a subscriber slot
        };

    public:
        template<typename EventT>
        _Ret_valid_ static SubToken Subscribe(_In_ typename Channel<EventT>::Callback cb)
        {
            RegisterIfNeeded<EventT>(); // first time stuff

            auto& subs = Channel<EventT>::Subscribers; // respective channel
            subs.push_back(std::move(cb));

            return { std::type_index(typeid(EventT)), subs.size() - 1, true };
        }

        template<typename EventT>
        static void Post(_In_ const EventT& event)
        {
            RegisterIfNeeded<EventT>();
            Channel<EventT>::Queue.push_back(event);
        }

        static void DispatchAll()
        {
            for (const auto& kv : s_mapRegistry) kv.second.dispatch();
        }

        static void ClearAll()
        {
            for (const auto& kv : s_mapRegistry) kv.second.clear();
        }

        static void Unsubscribe(_Inout_ SubToken& sub)
        {
            if (!sub.valid) return;

            auto& reg = s_mapRegistry;
            if (reg.contains(sub.type)) 
            {
                reg[sub.type].unsubscribe(sub.index);
            }

            sub.valid = false;
        }

        //~ If wanted to dispatch single known type only
        template<typename EventT>
        static void DispatchType()
        {
            auto& queue = Channel<EventT>::Queue;
            auto& subscribers = Channel<EventT>::Subscribers;

            for (std::size_t i = 0; i < queue.size(); ++i)
            {
                const EventT& event = queue[i];
                for (std::size_t subscriber = 0; subscriber < subscribers.size(); subscriber++)
                {
                    auto& callbackFn = subscribers[subscriber];
                    if (callbackFn) callbackFn(event);
                }
            }

            queue.clear();
        }

    private:
        template<typename EventT>
        static void DispatchThunk() { DispatchType<EventT>(); }

        template<typename EventT>
        static void ClearThunk() { Channel<EventT>::Queue.clear(); }

        template<typename EventT>
        _Success_(return)
            static bool UnsubThunk(_In_ std::size_t idx)
        {
            auto& subscribers = Channel<EventT>::Subscribers;

            if (idx >= subscribers.size()) return false;

            subscribers[idx] = nullptr; // just ignore if nullptr saves realloc
            return true;
        }

        template<typename EventT>
        static void RegisterIfNeeded()
        {
            auto& reg = s_mapRegistry;
            const std::type_index key(typeid(EventT)); // Handle
            
            if (reg.contains(key)) return;

            // Initialize type operations
            TypeOps op =
            {
                &DispatchThunk<EventT>,
                &ClearThunk   <EventT>,
                &UnsubThunk   <EventT>
            };
            reg[key] = std::move(op);
        }

    private:
        static fox::unordered_map<std::type_index, TypeOps> s_mapRegistry;
    };
} // namespace pixel_engine
