/**
 * Copyright (C) 2013 Jake Woods <jake.f.woods@gmail.com>, Trent Houliston <trent@houliston.me>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

namespace NUClear {
    
    // TODO enhance fill so that it only runs on the FillTypes
    // TODO enhance fill so that it will repeatedly run until all fill types are filled
    // TODO build the function signature deducer
    //          run get on everything and pass it into another one along with the NeedsFill status
    //          run Fill on everything until NeedsFill is false
    //          Get the final type by making it a const reference
    
    /* Meta functions */
    
    template <typename... TTypes>
    struct NeedsFill : std::false_type {};
    
    template <typename... TTypes>
    struct NeedsFill<std::tuple<TTypes...>> : NeedsFill<TTypes...> {};
    
    template <typename THead, typename... TTypes>
    struct NeedsFill<THead, TTypes...> :
    Meta::If<std::is_base_of<Internal::CommandTypes::FillType, THead>, std::true_type, NeedsFill<TTypes...>> {};
    
    // This metafunction extracts the type that is expected by the final function
    template <typename TType, bool Dereferenceable>
    struct FinalType;
    template <typename TType>
    struct FinalType<TType, false> {
        using type = typename std::add_const<typename std::add_lvalue_reference<TType>::type>::type;
    };
    template <typename TType>
    struct FinalType<TType, true> {
        using type = typename std::add_const<typename std::add_lvalue_reference<decltype(*std::declval<TType>())>::type>::type;
    };
    template <typename TType>
    struct GetFinalType : public FinalType<TType, Internal::Magic::Dereferenceable<TType>::value> {};
    
    // This metafunction builds a proper On given a series of function arguments
    template <typename TFunc, typename... TParams>
    struct Reactor::On : public OnBuilder<TFunc, Reactor::Trigger<>, Reactor::With<>, Reactor::Options<>, std::tuple<>, TParams...> {};
    
    template <
        typename TFunc,
        typename... TTriggers,
        typename... TWiths,
        typename... TOptions,
        typename... TFuncArgs,
        typename... TNewTriggers,
        typename... TParams>
    struct Reactor::OnBuilder<
        TFunc, Reactor::Trigger<TTriggers...>,
        Reactor::With<TWiths...>,
        Reactor::Options<TOptions...>,
        std::tuple<TFuncArgs...>,
        Reactor::Trigger<TNewTriggers...>,
        TParams...> :
    public Reactor::OnBuilder<
        TFunc,
        Reactor::Trigger<TTriggers..., TNewTriggers...>,
        Reactor::With<TWiths...>,
        Reactor::Options<TOptions...>,
        std::tuple<TFuncArgs..., TNewTriggers...>,
        TParams...> {};
    
    template <
        typename TFunc,
        typename... TTriggers,
        typename... TWiths,
        typename... TOptions,
        typename... TFuncArgs,
        typename... TNewWiths,
        typename... TParams>
    struct Reactor::OnBuilder<
        TFunc,
        Reactor::Trigger<TTriggers...>,
        Reactor::With<TWiths...>,
        Reactor::Options<TOptions...>,
        std::tuple<TFuncArgs...>,
        Reactor::With<TNewWiths...>,
        TParams...> :
    public Reactor::OnBuilder<
        TFunc,
        Reactor::Trigger<TTriggers...>,
        Reactor::With<TWiths..., TNewWiths...>,
        Reactor::Options<TOptions...>,
        std::tuple<TFuncArgs..., TNewWiths...>,
        TParams...> {};
    
    template <
        typename TFunc,
        typename... TTriggers,
        typename... TWiths,
        typename... TOptions,
        typename... TFuncArgs,
        typename... TNewOptions,
        typename... TParams>
    struct Reactor::OnBuilder<
        TFunc,
        Reactor::Trigger<TTriggers...>,
        Reactor::With<TWiths...>,
        Reactor::Options<TOptions...>,
        Reactor::Options<TNewOptions...>,
        std::tuple<TFuncArgs...>,
        TParams...> :
    public Reactor::OnBuilder<
        TFunc,
        Reactor::Trigger<TTriggers...>,
        Reactor::With<TWiths...>,
        Reactor::Options<TOptions..., TNewOptions...>,
        std::tuple<TFuncArgs...>,
        TParams...> {};
    
    template <
        typename TFunc,
        typename... TTriggers,
        typename... TWiths,
        typename... TOptions,
        typename... TFuncArgs>
    struct Reactor::OnBuilder<
        TFunc,
        Reactor::Trigger<TTriggers...>,
        Reactor::With<TWiths...>,
        Reactor::Options<TOptions...>,
        std::tuple<TFuncArgs...>> :
    public Reactor::On<
        TFunc,
        Reactor::Trigger<TTriggers...>,
        Reactor::With<TWiths...>,
        Reactor::Options<TOptions...>,
        std::tuple<TFuncArgs...>> {};
    
    template <typename TFunc, typename... TTypes>
    struct Reactor::CheckFunctionSignature<TFunc, std::tuple<TTypes...>, 0> :
    public Reactor::CheckFunctionSignature<TFunc, std::tuple<decltype(std::declval<PowerPlant::CacheMaster>().get<TTypes>())...>, 1> {};
    
    template <typename TFunc, typename... TTypes>
    struct Reactor::CheckFunctionSignature<TFunc, std::tuple<TTypes...>, 1> :
    public Reactor::CheckFunctionSignature<TFunc, std::tuple<decltype(std::declval<PowerPlant::CacheMaster>().fill(std::tuple<TTypes...>()))>, 2> {};
    
    template <typename TFunc, typename... TTypes>
    struct Reactor::CheckFunctionSignature<TFunc, std::tuple<TTypes...>, 2> :
    public Reactor::CheckFunctionSignature<TFunc, std::tuple<decltype(Internal::Magic::Dereferenceable<TTypes>::dereference(std::declval<TTypes>()))...>, 3> {};
    
    template <typename TFunc, typename... TTypes>
    struct Reactor::CheckFunctionSignature<TFunc, std::tuple<TTypes...>, 3> :
    public Reactor::CheckFunctionSignature<TFunc, std::tuple<Meta::AddConst<TTypes>...>, 4> {};
    
    template <typename TFunc, typename... TTypes>
    struct Reactor::CheckFunctionSignature<TFunc, std::tuple<TTypes...>, 4> :
    public Meta::If<std::is_assignable<std::function<void (TTypes...)>, TFunc>, std::true_type, std::false_type> {};
    
    
    /* End Meta functions */
    
    template <typename... TParams, typename TFunc>
    void Reactor::on(TFunc callback) {
        
        // There must be some parameters
        static_assert(sizeof...(TParams) > 0, "TODO not enough parameters message");
        
        On<TFunc, TParams...>::on(this, callback);
    }
    
    template <typename... THandlers, typename TData>
    void Reactor::emit(TData* data) {
        powerPlant.emit<THandlers...>(data);
    }
    
    // This is our final On statement
    template <typename TFunc, typename... TTriggers, typename... TWiths, typename... TOptions, typename... TFuncArgs>
    void Reactor::On<
        TFunc,
        Reactor::Trigger<TTriggers...>,
        Reactor::With<TWiths...>,
        Reactor::Options<TOptions...>,
        std::tuple<TFuncArgs...>>::on(Reactor* context, TFunc callback) {
            
            // TODO our static asserts go here
            // Static assert that our Triggers and withs are compatible with TFunc
            
            std::tuple<decltype(PowerPlant::CacheMaster::Get<TFuncArgs>::get(std::declval<PowerPlant*>()))...> x;
            
            // TODO this currently does not work, but the logic is there
            //static_assert(Reactor::CheckFunctionSignature<TFunc, std::tuple<TFuncArgs...>>::value, "Your callback function does not match the types in the On statement");
            static_assert(sizeof...(TTriggers) > 0, "You must have at least one Trigger in a callback");
            
            // Build up our options
            Internal::Reaction::Options options;
            context->buildOptions<TOptions...>(options);
            
            // Run any existence commands needed (running because a type exists)
            Internal::Magic::unpack((Exists<TFuncArgs>::exists(context), 0)...);
            
            // Bind all of our trigger events to a reaction
            context->bindTriggers<TTriggers...>(context->buildReaction<TFunc, TFuncArgs...>(callback, options));
    }
    
    template <typename... TOption>
    void Reactor::buildOptions(Internal::Reaction::Options& options) {
        // See unpack.h for explanation
        Internal::Magic::unpack((buildOptionsImpl(options, reinterpret_cast<TOption*>(0)), 0)...);
    }
    
    template <typename TSync>
    void Reactor::buildOptionsImpl(Internal::Reaction::Options& options, Sync<TSync>* /*placeholder*/) {
        options.m_syncQueue = Sync<TSync>::SyncQueue;
    }
    
    template <enum EPriority P>
    void Reactor::buildOptionsImpl(Internal::Reaction::Options& options, Priority<P>* /*placeholder*/) {
        options.m_priority = P;
    }
    
    template <>
    struct Reactor::buildCallback<true> {
        template <typename TFunc, typename... TData>
        static const std::function<void ()> get(Reactor* parent, TFunc callback, std::tuple<TData...> data) {
            return [parent, callback, data] {
                
                // Perform our second pass over the data
                auto&& newData = parent->powerPlant.cachemaster.fill(data);
                
                // TODO do a static assert that TFunc can take arguments of type decltype(newData)
                
                parent->powerPlant.cachemaster.setThreadArgs(std::this_thread::get_id(), std::move(Internal::Magic::buildVector(newData)));
                
                Internal::Magic::apply(callback, std::move(newData));
            };
        }
    };
    
    template <>
    struct Reactor::buildCallback<false> {
        
        template <typename TFunc, typename... TData>
        static const std::function<void ()> get(Reactor* parent, TFunc callback, std::tuple<TData...> data) {
            return [parent, callback, data] {
                
                // TODO do a static assert that TFunc can take arguments of type decltype(data)
                
                parent->powerPlant.cachemaster.setThreadArgs(std::this_thread::get_id(), std::move(Internal::Magic::buildVector(data)));
                
                Internal::Magic::apply(callback, std::move(data));
            };
        }
    };

    template <typename TFunc, typename... TTriggersAndWiths>
    Internal::Reaction* Reactor::buildReaction(TFunc callback, Internal::Reaction::Options& options) {
        
        // Return a reaction object that gets and runs with the correct paramters
        return new Internal::Reaction(typeid(TFunc).name(), [this, callback]() -> std::function<void ()> {
            
            auto&& data = std::make_tuple(powerPlant.cachemaster.get<TTriggersAndWiths>()...);
            
            return buildCallback<NeedsFill<typename std::remove_reference<decltype(data)>::type>::value>::get(this, callback, data);
        }, options);
    }
    
    template <typename TData>
    struct Reactor::Exists {
        static void exists(Reactor* context) {};
    };

    template <typename TData>
    struct Reactor::TriggerType {
        using type = TData;
    };

    /**
     * @brief calls bindTriggersImpl on every trigger in the list. Minimum list size is 1
     * @tparam TTrigger the required first trigger
     * @tparam TTriggers the rest of the triggers
     * @param callback the callback to bind to these triggers
     */
    template <typename TTrigger, typename... TTriggers>
    void Reactor::bindTriggers(Internal::Reaction* callback) {
        
        // Single trigger that is not ignored
        if(sizeof...(TTriggers) == 0 && !std::is_same<typename TriggerType<TTrigger>::type, std::nullptr_t>::value) {
            CallbackCache<typename TriggerType<TTrigger>::type>::set(callback);
        }
        
        // Multi Trigger (and)
        else {
            // TODO Generate a sequence
            // TODO set a callback for each of TTrigger and TTriggers along with their number
            // TODO have a tuple of atomic booleans for each TTrigger
            // TODO when a trigger comes in, use the int to set that boolean to true
            // TODO when all the booleans are true, run the callback and set all booleans to false
        }
    }
}
