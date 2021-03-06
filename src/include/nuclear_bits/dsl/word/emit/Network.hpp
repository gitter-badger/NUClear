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

#ifndef NUCLEAR_DSL_WORD_EMIT_NETWORK_HPP
#define NUCLEAR_DSL_WORD_EMIT_NETWORK_HPP

#include <array>

#include "nuclear_bits/util/serialise/Serialise.hpp"

namespace NUClear {
    namespace dsl {
        namespace word {
            namespace emit {

                struct NetworkEmit {
                    NetworkEmit() : target(""), hash(), data(), reliable(false) {}

                    std::string target;
                    std::array<uint64_t, 2> hash;
                    std::vector<char> data;
                    bool reliable;
                };

                template <typename TData>
                struct Network {

                    static void emit(PowerPlant& powerplant, std::shared_ptr<TData> data, std::string s = "", bool reliable = false) {

                        auto e = std::make_unique<NetworkEmit>();

                        e->target = s;
                        e->hash = util::serialise::Serialise<TData>::hash();
                        e->data = util::serialise::Serialise<TData>::serialise(*data);
                        e->reliable = reliable;

                        powerplant.emit<Direct>(e);
                    }
                };

            }  // namespace emit
        }  // namespace word
    }  // namespace dsl
}  // namespace NUClear

#endif  // NUCLEAR_DSL_WORD_EMIT_NETWORK_HPP
