/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CALLBACK_CONTAINER_H_
#define _CALLBACK_CONTAINER_H_

#include <unordered_map>

#include <boost/any.hpp>
#include <boost/optional.hpp>

#include "Callback.h"

class CallbackContainer
{
    std::shared_ptr<CallbackContainer> self_reference;

    typedef size_t handle_t;

    size_t handle;

    std::unordered_map<decltype(handle), boost::any> container;

    template<typename _CTy, typename... Args>
    struct ProxyFactory;

    template<typename _CTy, typename... Args>
    struct ProxyFactory<_CTy, std::tuple<Args...>>
    {
        // Creates a weak proxy callback which prevents invoking to an invalid context.
        static callback_of_t<_CTy> CreateProxy(std::weak_ptr<CallbackContainer> const& weak_owner,
            size_t const handle, weak_callback_of_t<_CTy> const& weak_callback)
        {
            return [=](Args&&... args)
            {
                // Try to get a pointer to the owner
                if (auto const owner = weak_owner.lock())
                    // And to the wrapped functional itself
                    if (auto const callback = weak_callback.lock())
                    {
                        // Invoke the original callback
                        // FIXME: std::forward<Args..>(args...) causes errors when void
                        (*callback)(args...);

                        // Unregister the callback
                        owner->InvalidateCallback(handle);
                    }
            };
        }
    };

public:
    CallbackContainer()
        : self_reference(this, [](decltype(this) me) { }), handle(0L) { }

    ~CallbackContainer() = default;

    CallbackContainer(CallbackContainer const&) = delete;
    CallbackContainer(CallbackContainer&&) = delete;

    CallbackContainer& operator= (CallbackContainer const&) = delete;
    CallbackContainer& operator= (CallbackContainer&&) = delete;

    CallbackContainer& Clear()
    {
        container.clear();
        return *this;
    }

    template<typename _CTy>
    auto operator()(_CTy&& callback)
        -> callback_of_t<_CTy>
    {
        // Create the shared callback
        shared_callback_of_t<_CTy> shared_callback = make_shared_callback(std::forward<_CTy>(callback));

        // Create a weak proxy callback which removes the callback on execute
        auto const this_handle = handle++;
        callback_of_t<_CTy> proxy =
            ProxyFactory<_CTy, ::fu::argument_type_of_t<_CTy>>::
                CreateProxy(self_reference, this_handle, shared_callback);

        container.insert(std::make_pair(this_handle, boost::any(std::move(shared_callback))));
        return std::move(proxy);
    }

    boost::optional<handle_t> GetLastCallbackHandle() const
    {
        if (handle == 0L)
            return boost::none;
        else
            return boost::make_optional(handle);
    }

    CallbackContainer& InvalidateCallback(handle_t const handle)
    {
        container.erase(handle);
        return *this;
    }
};

#endif /// _CALLBACK_CONTAINER_H_