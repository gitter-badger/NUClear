/*
 * Copyright (C) 2013-2016 Trent Houliston <trent@houliston.me>, Jake Woods <jake.f.woods@gmail.com>
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

#ifndef NUCLEAR_DSL_WORD_NETWORK_HPP
#define NUCLEAR_DSL_WORD_NETWORK_HPP

#include "nuclear_bits/dsl/trait/is_transient.hpp"
#include "nuclear_bits/dsl/store/ThreadStore.hpp"
#include "nuclear_bits/util/serialise/Serialise.hpp"

namespace NUClear {
    namespace dsl {
        namespace word {

            template <typename TData>
            struct NetworkData : public std::shared_ptr<TData> {
                using std::shared_ptr<TData>::shared_ptr;
            };

            struct NetworkSource {
                NetworkSource() : name(""), address(0), port(0), reliable(false), multicast(false) {}

                std::string name;
                in_addr_t address;
                in_port_t port;
                bool reliable;
                bool multicast;
            };

            struct NetworkListen {
                NetworkListen() : hash(), reaction() {}

                std::array<uint64_t, 2> hash;
                std::shared_ptr<threading::Reaction> reaction;
            };

            template <typename TData>
            struct Network {

                template <typename DSL, typename TFunc>
                static inline threading::ReactionHandle bind(Reactor& reactor, const std::string& label, TFunc&& callback) {

                    auto task = std::make_unique<NetworkListen>();

                    task->hash = util::serialise::Serialise<TData>::hash();
                    task->reaction = util::generate_reaction<DSL, NetworkListen>(reactor, label, std::forward<TFunc>(callback));

                    threading::ReactionHandle handle(task->reaction);

                    reactor.powerplant.emit<emit::Direct>(task);

                    return handle;
                }

                template <typename DSL>
                static inline std::tuple<std::shared_ptr<NetworkSource>, NetworkData<TData>> get(threading::Reaction&) {

                    auto data = store::ThreadStore<std::vector<char>>::value;
                    auto source = store::ThreadStore<NetworkSource>::value;

                    if(data && source) {

                        // Return our deserialised data
                        return std::make_tuple(std::make_shared<NetworkSource>(*source), std::make_shared<TData>(util::serialise::Serialise<TData>::deserialise(*data)));
                    }
                    else {

                        // Return invalid data
                        return std::make_tuple(std::shared_ptr<NetworkSource>(nullptr), std::shared_ptr<TData>(nullptr));
                    }
                }
            };

        }  // namespace word

        namespace trait {

            template <typename T>
            struct is_transient<typename word::NetworkData<T>> : public std::true_type {};

            template <>
            struct is_transient<typename std::shared_ptr<word::NetworkSource>> : public std::true_type {};

        }  // namespace trait
    }  // namespace dsl
}  // namespace NUClear

#endif  // NUCLEAR_DSL_WORD_NETWORK_HPP
