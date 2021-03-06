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

#ifndef NUCLEAR_DSL_WORD_WITH_HPP
#define NUCLEAR_DSL_WORD_WITH_HPP

#include "nuclear_bits/dsl/Fusion.hpp"

#include "nuclear_bits/dsl/operation/CacheGet.hpp"

namespace NUClear {
    namespace dsl {
        namespace word {

            /**
             * @ingroup Wrappers
             * @brief This is a wrapper class which is used to list the data types to use in addtion on a callback.
             *
             * @details
             *  This class is used in the on binding to specify additional data which is used in a call, but does not
             *  trigger the call to run. When the call is run, the latest version of the data available will be obtained.
             *  from the cache, and used as a paramter in this function call.
             *
             * @tparam TWiths the datatypes to get from the cache and use in the callback.
             */
            template <typename... TWiths>
            struct With : public Fusion<operation::CacheGet<TWiths>...> {};

        }  // namespace word
    }  // namespace dsl
}  // namespace NUClear

#endif  // NUCLEAR_DSL_WORD_WITH_HPP
