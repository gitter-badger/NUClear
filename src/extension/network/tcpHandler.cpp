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

#include "nuclear_bits/extension/NetworkController.hpp"
#include "nuclear_bits/extension/network/WireProtocol.hpp"

#include <algorithm>
#include <cerrno>

namespace NUClear {
    namespace extension {

        ssize_t recv_all(fd_t fd, char* buff, size_t len) {
            ssize_t index = 0;

            while (size_t(index) < len) {

                // Receive as much data as we can
                auto v = recv(fd, buff + index, len - size_t(index), 0);

                // Work out what to do based on this v
                switch (v) {
                    case 0: {
                        // Socket shut down
                        return index;
                    }
                    case -1: {
                        // Get our error to see if it's because the socket is non blocking (happens on windows)
                        auto error = network_errno;
                        #ifdef _WIN32
                        if (error == WSAEWOULDBLOCK) { continue; }
                        #else
                        if (error == EAGAIN || error == EWOULDBLOCK) { continue; }
                        #endif
                        return -1;

                        // Was an error, we don't care if it's WSAEWOULDBLOCK on windows or EAGAIN/EWOULDBLOCK on other os's
                    }
                    default: {
                        // Otherwise we move on to more bytes
                        index += v;
                    }
                }
            }

            return index;
        }

        void NetworkController::tcpHandler(const IO::Event& e) {

            // Find this connections target
            auto target = tcpTarget.find(e.fd);

            // Sometimes, if a tcp event is queued, it can come in before the unbind
            // Squash it here to prevent errors
            if (target == tcpTarget.end()) {
                return;
            }

            bool badPacket = false;

            // If we have data to read
            if(e.events & IO::READ) {

                // Allocate data for our header
                std::vector<char> data(sizeof(network::PacketHeader));

                // Read our header and check it is valid
                if(recv_all(e.fd, data.data(), data.size()) == sizeof(network::PacketHeader)) {
                    const network::PacketHeader& header = *reinterpret_cast<network::PacketHeader*>(data.data());
                    uint32_t length = header.length;

                    // Add enough space for our remaining packet
                    data.resize(data.size() + length);

                    // Read our remaining packet and check it's valid
                    if(recv_all(e.fd, data.data() + sizeof(network::PacketHeader), length) == ssize_t(length)) {

                        const network::DataPacket& packet = *reinterpret_cast<network::DataPacket*>(data.data());

                        // Copy our data into a vector
                        std::vector<char> payload(&packet.data, &packet.data + packet.length - sizeof(network::DataPacket) + sizeof(network::PacketHeader) + 1);

                        // Construct our NetworkSource information
                        dsl::word::NetworkSource src;
                        src.name = target->second->name;
                        src.address = target->second->address;
                        src.port = target->second->udpPort;
                        src.reliable = true;
                        src.multicast = packet.multicast;

                        // Store in our thread local cache
                        dsl::store::ThreadStore<std::vector<char>>::value = &payload;
                        dsl::store::ThreadStore<dsl::word::NetworkSource>::value = &src;

                        /* Mutex Scope */ {
                            // Lock our reaction mutex
                            std::lock_guard<std::mutex> lock(reactionMutex);

                            // Find interested reactions
                            auto rs = reactions.equal_range(packet.hash);

                            // Execute on our interested reactions
                            for(auto it = rs.first; it != rs.second; ++it) {
                                auto task = it->second->getTask();
                                if(task) {
                                    powerplant.submit(std::move(task));
                                }
                            }
                        }

                        // Clear our cache
                        dsl::store::ThreadStore<std::vector<char>>::value = nullptr;
                        dsl::store::ThreadStore<dsl::word::NetworkSource>::value = nullptr;
                    }
                    else {
                        // Packet is invalid
                        badPacket = true;
                    }
                }
                else {
                    // Packet is invalid
                    badPacket = true;
                }
            }

            // If the connection closed or errored (or reached an end of file)
            // And we have not already closed this connection
            if((e.events & IO::CLOSE) || (e.events & IO::ERROR) || badPacket) {

                // emit a message that says who disconnected
                auto l = std::make_unique<message::NetworkLeave>();
                l->name = target->second->name;
                l->address = target->second->address;
                l->tcpPort = target->second->tcpPort;
                l->udpPort = target->second->udpPort;
                emit(l);

                // Unbind our connection
                target->second->handle.unbind();

                // Close our half of the connection
                close(e.fd);

                // Remove our UDP target
                udpTarget.erase(udpTarget.find(std::make_pair(target->second->address, target->second->udpPort)));

                // Remove our name target
                auto range = nameTarget.equal_range(target->second->name);
                for (auto it = range.first; it != range.second; ++it) {
                    if(it->second->address == target->second->address && it->second->udpPort == target->second->udpPort) {
                        nameTarget.erase(it);
                        break;
                    };
                }

                // Remove our element
                targets.erase(target->second);

                // Remove our TCP target
                tcpTarget.erase(target);
            }
        }
    }
}
