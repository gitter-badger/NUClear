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

#include "NUClear/Internal/Reaction.h"

namespace NUClear {
namespace Internal {
    
    std::atomic<uint64_t> Reaction::reactionIdSource(0);
    std::atomic<uint64_t> Reaction::Task::taskIdSource(0);
    
    
    Reaction::Reaction(std::string name, std::function<std::function<void (Reaction::Task&)> ()> callback, Reaction::Options options) :
    m_name(name),
    m_options(options),
    m_reactionId(++reactionIdSource),
    m_running(false),
    m_enabled(true),
    m_callback(callback) {
    }
    
    std::unique_ptr<Reaction::Task> Reaction::getTask(const Task* cause) {
        // Build a new data bound task using our callback generator
        return std::unique_ptr<Reaction::Task>(new Reaction::Task(this, cause, m_callback()));
    }
    
    bool Reaction::isEnabled() {
        return m_enabled;
    }
    
    Reaction::OnHandle::OnHandle(Reaction* context) : m_context(context) {
    };
    
    bool Reaction::OnHandle::isEnabled() {
        return m_context->m_enabled;
    }
    
    void Reaction::OnHandle::enable() {
        m_context->m_enabled = true;
    }
    
    void Reaction::OnHandle::disable() {
        m_context->m_enabled = false;
    }
    
    Reaction::Task::Task(Reaction* parent, const Task* cause, std::function<void (Task&)> callback) :
    m_callback(callback),
    m_parent(parent),
    m_taskId(++taskIdSource),
    m_stats(new NUClearTaskEvent{parent->m_name, parent->m_reactionId, m_taskId, cause ? cause->m_parent->m_reactionId : -1, cause ? cause->m_taskId : -1, clock::now()}) {
    }
    
    void Reaction::Task::operator()() {
        // Call our callback
        m_callback(*this);
    }
}
}
