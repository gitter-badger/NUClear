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

#ifndef NUCLEAR_EXTENSION_NETWORKCONTROLLER_HPP
#define NUCLEAR_EXTENSION_NETWORKCONTROLLER_HPP

#include "nuclear"
#include "nuclear_bits/extension/network/WireProtocol.hpp"

namespace NUClear {
    namespace extension {

        class NetworkController : public Reactor {
        private:
            struct NetworkTarget {

                NetworkTarget(std::string name
                              , in_addr_t address
                              , in_port_t tcpPort
                              , in_port_t udpPort
                              , int tcpFD)
                : name(name)
                , address(address)
                , tcpPort(tcpPort)
                , udpPort(udpPort)
                , tcpFD(tcpFD)
                , handle()
                , buffer()
                , bufferMutex() {}

                std::string name;
                in_addr_t address;
                in_port_t tcpPort;
                in_port_t udpPort;
                fd_t tcpFD;
                ReactionHandle handle;
                std::map<in_port_t, std::pair<clock::time_point, std::vector<std::vector<char>>>> buffer;
                std::mutex bufferMutex;
            };

        public:
            explicit NetworkController(std::unique_ptr<NUClear::Environment> environment);

        private:
            void tcpConnection(const TCP::Connection& con);
            void tcpHandler(const IO::Event& event);
            void udpHandler(const UDP::Packet& packet);
            void tcpSend(const dsl::word::emit::NetworkEmit& emit);
            void udpSend(const dsl::word::emit::NetworkEmit& emit);
            void announce();

            // Our max UDP size is based of a 1500 MTU
            // Subtract the IP and UPD headers, and our protocol header size
            static constexpr const size_t MAX_UDP_PAYLOAD_LENGTH = 1500 /*MTU*/ - 20 /*IP header*/ - 8 /*UDP header*/ - sizeof(network::DataPacket) + 1 /*Last char*/;
            static constexpr const size_t MAX_NUM_UDP_ASSEMBLEY = 5;

            std::mutex writeMutex;

            ReactionHandle udpHandle;
            ReactionHandle tcpHandle;
            ReactionHandle multicastHandle;
            ReactionHandle multicastEmitHandle;
            ReactionHandle networkEmitHandle;

            std::string name;
            std::string multicastGroup;
            in_port_t multicastPort;
            in_port_t udpPort;
            in_port_t tcpPort;

            int udpServerFD;
            int tcpServerFD;

            std::atomic<uint16_t> packetIDSource;

            std::mutex reactionMutex;
            std::multimap<std::array<uint64_t, 2>, std::shared_ptr<threading::Reaction>> reactions;

            std::list<NetworkTarget> targets;
            std::multimap<std::string, std::list<NetworkTarget>::iterator> nameTarget;
            std::map<std::pair<int, int>, std::list<NetworkTarget>::iterator> udpTarget;
            std::map<int, std::list<NetworkTarget>::iterator> tcpTarget;
        };

    }  // namespace extension
}  // namespace NUClear

#endif  // NUCLEAR_EXTENSION_NETWORKCONTROLLER_HPP
