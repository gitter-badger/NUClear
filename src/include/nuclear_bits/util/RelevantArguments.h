/*
 * Copyright (C) 2013 Trent Houliston <trent@houliston.me>, Jake Woods <jake.f.woods@gmail.com>
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

#ifndef NUCLEAR_UTIL_RELEVANTARGUMENTS_H
#define NUCLEAR_UTIL_RELEVANTARGUMENTS_H

#include "nuclear_bits/util/CallableInfo.h"

namespace NUClear {
    namespace util {

        template <typename Required, typename Available, typename Used = Sequence<>, int Index = 0>
        struct RelevantArguments;

        // We were given a function instead of a tuple of arguments (extract the information)
        template <
        typename Callee,
        typename... Available,
        int... Used,
        int Index
        >
        struct RelevantArguments<Callee, std::tuple<Available...>, Sequence<Used...>, Index>
        : public RelevantArguments<
        typename CallableInfo<Callee>::arguments,
        std::tuple<Available...>,
        Sequence<Used...>, Index> {};

        // Process our next layer of functions
        template <
        typename Required,
        typename... RList,
        typename Available,
        typename... AList,
        int... Used,
        int Index
        >
        struct RelevantArguments<std::tuple<Required, RList...>, std::tuple<Available, AList...>, Sequence<Used...>, Index>
        : public RelevantArguments<
        Meta::If<std::is_convertible<Available, Required>, std::tuple<RList...>, std::tuple<Required, RList...>>,
        std::tuple<AList...>,
        Meta::If<std::is_convertible<Available, Required>, Sequence<Used..., Index>, Sequence<Used...>>,
        Index + 1
        > {};

        // Fail, we ran out of arguments before we filled the requirements
        template <
        typename... Required,
        int... Used,
        int Index
        >
        struct RelevantArguments<std::tuple<Required...>, std::tuple<>, Sequence<Used...>, Index> {
            static_assert(sizeof...(Required) == 0, "There are not enough arguments of the correct type to satisfy this function");
        };

        // We found a match (and have some args left over)
        template <
        typename... Available,
        int... Used,
        int Index
        >
        struct RelevantArguments<std::tuple<>, std::tuple<Available...>, Sequence<Used...>, Index> {
            using type = Sequence<Used...>;
        };

        // We found a match (no args left over)
        template <
        int... Used,
        int Index
        >
        struct RelevantArguments<std::tuple<>, std::tuple<>, Sequence<Used...>, Index> {
            using type = Sequence<Used...>;
        };
    }
}
#endif
