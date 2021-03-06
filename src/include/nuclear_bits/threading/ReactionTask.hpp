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

#ifndef NUCLEAR_THREADING_REACTIONTASK_HPP
#define NUCLEAR_THREADING_REACTIONTASK_HPP

#include <functional>
#include <atomic>
#include <vector>
#include <typeindex>
#include <memory>

#include "nuclear_bits/util/platform.hpp"
#include "nuclear_bits/message/ReactionStatistics.hpp"

namespace NUClear {
    namespace threading {
        // Forward declare reaction
        class Reaction;

        /**
         * @brief This is a databound call of a Reaction ready to be executed.
         *
         * @details
         *  This class holds a reaction that is ready to be executed. It is a Reaction object which has had it's callback
         *  parameters bound with data. This can then be executed as a function to run the call inside it.
         *
         * @author Trent Houliston
         */
        class ReactionTask {
        private:
            /// @brief a source for reactionIds, atomically creates longs
            static std::atomic<std::uint64_t> taskIdSource;

            /// @brief the current task that is being executed by this thread (or nullptr if none is)
            static ATTRIBUTE_TLS ReactionTask* currentTask;
        public:

            /**
             * @brief Gets the current executing task, or nullptr if there isn't one.
             *
             * @return the current executing task or nullptr if there isn't one
             */
            static const ReactionTask* getCurrentTask();

            /**
             * @brief Creates a new ReactionTask object bound with the parent Reaction object (that created it) and task.
             *
             * @param parent    the Reaction object that spawned this ReactionTask.
             * @param priority  the priority to use when executing this task.
             * @param callback  the data bound callback to be executed in the threadpool.
             */
            ReactionTask(Reaction& parent, int priority, std::function<std::unique_ptr<ReactionTask> (std::unique_ptr<ReactionTask>&&)> callback);

            /**
             * @brief Runs the internal data bound task and times it.
             *
             * @details
             *  This runs the internal data bound task and times how long the execution takes. These figures can then be
             *  used in a debugging context to calculate how long callbacks are taking to run.
             */
            std::unique_ptr<ReactionTask> run(std::unique_ptr<ReactionTask>&& us);

            /// @brief the parent Reaction object which spawned this
            Reaction& parent;
            /// @brief the taskId of this task (the sequence number of this paticular task)
            uint64_t taskId;
            /// @brief the priority to run this task at
            int priority;
            /// @brief the statistics object that persists after this for information and debugging
            std::unique_ptr<message::ReactionStatistics> stats;

            /// @brief the data bound callback to be executed
            /// @attention note this must be last in the list as the this pointer is passed to the callback generator
            std::function<std::unique_ptr<ReactionTask> (std::unique_ptr<ReactionTask>&&)> callback;
        };

        /**
         * @brief This overload is used to sort reactions by priority.
         *
         * @param a the reaction task a
         * @param b the reaction task b
         *
         * @return true if a has lower priority than b, false otherwise
         */
        inline bool operator<(const std::unique_ptr<ReactionTask>& a, const std::unique_ptr<ReactionTask>& b) {

            // If we ever have a null pointer, we move it to the top of the queue as it is being removed
            return a == nullptr ? false
                 : b == nullptr ? true
                 : a->priority == b->priority ? a->stats->emitted < b->stats->emitted
                 : a->priority < b->priority;
            
        }

    }  // namespace threading
 }  // namespace NUClear

#endif  // NUCLEAR_THREADING_REACTIONTASK_HPP
