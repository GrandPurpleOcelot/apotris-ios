#pragma once
#include "def.h"
#include "rtc/rtc.hpp"
#include <algorithm>
#include <future>
#include <nlohmann/json.hpp>
#include <queue>
#include <sstream>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

constexpr char LINK_UNIVERSAL_MAX_PLAYERS = 5;
constexpr char LINK_WIRELESS_QUEUE_SIZE = 30;
#ifndef GBA
extern void removePlayer(u8 pId);
extern void clearPlayers();
extern void initPlayer(u8 pId);
#endif

using nlohmann::json;
using std::shared_ptr;
using std::weak_ptr;
template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

std::string stringifyMultiplayerPacket(u16 packet);

struct BoardUpdatePacket {
    u8 playerId;
    u16 seq;
    std::array<u16, 20> boardRows;
};

class LinkUniversal {
    static constexpr char STUN_SERVER[] = "stun:stun.l.google.com:19302";
    static constexpr char TYPE[] = "type";
    static constexpr char ID[] = "id";
    static constexpr char NAME[] = "name";
    static constexpr char DESCRIPTION[] = "description";
    static constexpr char OFFER[] = "offer";
    static constexpr char ANSWER[] = "answer";
    static constexpr char MATCH[] = "match";
    static constexpr char CLOSE[] = "close";
    static constexpr char SEED[] = "seed";
    static constexpr char FULL[] = "full";
    static constexpr char CANDIDATE[] = "candidate";
    static constexpr char PLAYER_INFO[] = "playerInfo";
    static constexpr char MID[] = "mid";
    static constexpr char ITER[] = "iter";
    static constexpr char FROM[] = "from";

    rtc::Configuration config;
    std::map<std::string, std::queue<Packet>> readQueues;
    std::queue<BoardUpdatePacket> incomingBoardUpdates;
    bool active = false;
    shared_ptr<rtc::WebSocket> ws;
    int playerId = -1;
    std::unordered_map<std::string, u8> playerStringToId;
    std::unordered_map<u8, std::string> playerIdToString;
    std::unordered_map<std::string, shared_ptr<rtc::PeerConnection>>
        peerConnectionMap;
    std::unordered_map<std::string, shared_ptr<rtc::DataChannel>>
        dataChannelMap;
    std::unordered_map<u8, std::string> playerIdToName;

    std::mutex readQueueLock;

public:
    std::string localId;
    std::string localPlayerName;
    [[nodiscard]] u8 currentPlayerId() const { return playerId; }
    static std::string randomId(size_t length) {
        using std::chrono::high_resolution_clock;
        static const std::string characters("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        std::string id(length, '0');
        std::uniform_int_distribution<int> uniform(0,
                                                   int(characters.size() - 1));
        std::generate(id.begin(), id.end(), [&]() {
            return characters.at((int)randNext() % (characters.length() - 1));
        });
        return id;
    }
    shared_ptr<rtc::PeerConnection>
    getOrCreatePeerConnection(const std::string& peerId,
                              const std::string& type) {
        if (auto jt = peerConnectionMap.find(peerId);
            jt != peerConnectionMap.end())
            return jt->second;
        else if (type == OFFER || type == MATCH)
            return createPeerConnection(make_weak_ptr(ws), peerId);
        else
            return nullptr;
    }
    void initWebSocketConfig() {
        localId = randomId(10);
        localPlayerName = savefile->latestName;

#ifndef WEB
        rtc::InitLogger(rtc::LogLevel::Debug);
#endif
        config.iceServers.emplace_back(STUN_SERVER);
        ws = std::make_shared<rtc::WebSocket>();

        ws->onClosed([]() { std::cout << "WebSocket closed" << std::endl; });
        ws->onMessage([this, wws = make_weak_ptr(ws)](auto data) {
            if (!std::holds_alternative<std::string>(data))
                return;
            std::cout << "Got websocket message: "
                      << std::get<std::string>(data) << std::endl;
            json message = json::parse(std::get<std::string>(data));
            if (message.find(ID) == message.end() ||
                message.find(TYPE) == message.end())
                return;

            std::string type = message[TYPE];
            std::string id =
                message[ID]; // The value from the "id" field of the JSON
            std::string peerId = id; // This will be the peer ID for RTC
                                     // operations. Default to
                                     // id_field_in_message.

            // For OFFER, ANSWER, CANDIDATE, the message[ID] is localId
            // (recipient). The actual remote peer's ID is in the "from" field
            // added by the server.
            if (type == OFFER || type == ANSWER || type == CANDIDATE) {
                if (!message.contains(FROM)) {
                    std::cerr << "WebSocket " << type
                              << " message missing 'from' field. Discarding."
                              << std::endl;
                    return;
                }
                peerId = message[FROM].get<std::string>();
                if (peerId ==
                    localId) { // Should be caught by server or is an error
                    std::cerr << "WebSocket " << type
                              << " message 'from' self (" << peerId
                              << "). Discarding." << std::endl;
                    return;
                }
            }

            if (type == PLAYER_INFO) {
                playerIdToName[playerStringToId[message[FROM]]] = message[NAME];
            }
            if (type == SEED) {
                nextSeed = stoi(id) & ((0x1000) - 1);
                lastSeed = nextSeed;
                return;
            }
            if (type == MATCH) {
                int iter = message[ITER];
                int remotePlayerId = iter;
                if (peerId == localId) {
                    if (playerId != -1 && remotePlayerId == 0) {
                        playerIdToString.clear();
                        playerStringToId.clear();
                        playerIdToName.clear();
                    }
                    playerId = remotePlayerId;

                    playerIdToName[playerId] = localPlayerName;
                    std::cout << "We think we are player ID " << playerId
                              << std::endl;
                }
                playerIdToString[remotePlayerId] = peerId;
                playerStringToId[peerId] = remotePlayerId;
                clearPlayers();
                for (auto& i : playerIdToString) {
                    initPlayer(i.first);
                }
                if (peerId == localId)
                    return;
            }

            auto pc = getOrCreatePeerConnection(peerId, type);

            if (type == CLOSE) {
                if (dataChannelMap.count(peerId)) {
                    dataChannelMap[peerId]->close();
                    dataChannelMap.erase(peerId);
                }
                peerConnectionMap.erase(peerId);
#ifndef WEB
                if (pc)
                    pc->close();
#endif
                if (playerStringToId.count(peerId)) {
                    u8 pId_to_remove = playerStringToId[peerId];
                    removePlayer(pId_to_remove);
                    playerIdToString.erase(pId_to_remove);
                    playerStringToId.erase(peerId);
                }

            } else if (type == MATCH && peerId != localId) {
                if (!pc || dataChannelMap.count(peerId))
                    return;
                makeOffer(pc, peerId);
            } else if (type == OFFER || type == ANSWER)
                pc->setRemoteDescription(
                    rtc::Description(message[DESCRIPTION], type));
            else if (type == CANDIDATE)
                pc->addRemoteCandidate(
                    rtc::Candidate(message[CANDIDATE], message[MID]));
            else if (type == FULL)
                exceptionReason = "Room is full";
        });
    }
    void activate() {
        if (active)
            return;
        active = true;
        initWebSocketConfig();

#ifndef WEB
        std::promise<void> wsPromise;
        auto wsFuture = wsPromise.get_future();

        ws->onOpen([this, wws = make_weak_ptr(ws), &wsPromise]() {
            std::cout << "WebSocket connected, signaling ready" << std::endl;
            wsPromise.set_value();
            if (auto shared_ws = wws.lock()) {
                json playerInfo = {{ID, "broadcast"},
                                   {TYPE, PLAYER_INFO},
                                   {NAME, localPlayerName}};
                shared_ws->send(playerInfo.dump());
                std::cout << "Sent player info to signaling server: "
                          << playerInfo.dump() << std::endl;
            }
        });

        ws->onError([&wsPromise](const std::string& s) {
            std::cout << "WebSocket error" << std::endl;
            wsPromise.set_exception(
                std::make_exception_ptr(std::runtime_error(s)));
        });
#else
        ws->onOpen([wws = make_weak_ptr(ws)]() {
            std::cout << "WebSocket connected, signaling ready" << std::endl;
            if (auto shared_ws = wws.lock()) {
                json playerInfo = {{ID, "broadcast"},
                                   {TYPE, PLAYER_INFO},
                                   {NAME, localPlayerName}};
                shared_ws->send(playerInfo.dump());
                std::cout << "Sent player info to signaling server: "
                          << playerInfo.dump() << std::endl;
            }
        });

        ws->onError([](const std::string& s) {
            std::cout << "WebSocket error" << std::endl;
        });
#endif

        std::string url = "wss://api.apotris.com/ws/" + localId +
                          (!roomId.empty() ? "/" + roomId : "/match");
        url.erase(remove(url.begin(), url.end(), ' '), url.end());

        std::cout << "WebSocket URL is " << url << std::endl;
        ws->open(url);

#ifndef WEB
        std::cout << "Waiting for signaling to be connected..." << std::endl;
        wsFuture.get();
#endif
    }

    void makeOffer(std::shared_ptr<rtc::PeerConnection> pc,
                   const std::string& remoteId) {
        // We are the offerer if we have a lower ID
        if (localId < remoteId) {
            return;
        }
        std::stringstream ss;
        ss << "apotris:" << localId << std::to_string(playerId) << "<->"
           << std::to_string(playerStringToId[remoteId]);
        std::cout << "Creating DataChannel with label \"" << ss.str() << "\""
                  << std::endl;
        auto dc = pc->createDataChannel(ss.str());
        registerDataChannelHandlers(dc, remoteId);
    }

    void parseMessage(rtc::message_variant data, const std::string& peerId) {
        // data holds either std::string or rtc::binary
        if (std::holds_alternative<std::string>(data)) {
            return;
        } else {
            auto packet = std::get<rtc::binary>(data);
            if (packet.size() == 2) {
#ifdef TRACY_ENABLE
                ZoneScopedN("recv_packet");
                ZoneTextF("from=%s", peerId.c_str());
#endif
                std::byte foo = packet[0];
                std::byte bar = packet[1];
                u16 real = (u8)foo + ((u16)bar << 8);
#ifdef TRACY_ENABLE
                ZoneTextF("0x%04x", real);
                ZoneTextF("%s", stringifyMultiplayerPacket(real).c_str());
#endif
                int realRemoteId = playerStringToId[peerId];
                std::lock_guard<std::mutex> guard{readQueueLock};
                readQueues[peerId].emplace((u8)(realRemoteId), real);
            } else if (packet.size() == 44 &&
                       ((u16)packet[0] | ((u16)packet[1] << 8)) == 0x1c00) {
                // 0x1c00 == ENCODE(SEND_ROW) + (0x1f << 10)
#ifdef TRACY_ENABLE
                ZoneScopedN("recv_packet(board)");
                ZoneTextF("from=%s", peerId.c_str());
#endif
                int realRemoteId = playerStringToId[peerId];
                const u16 seq = (u16)packet[2] | ((u16)packet[3] << 8);

#ifdef TRACY_ENABLE
                ZoneTextF("seq=%u", seq);
#endif
                std::lock_guard<std::mutex> guard{readQueueLock};
                auto& boardUpdate =
                    incomingBoardUpdates.emplace(BoardUpdatePacket{
                        .playerId = (u8)realRemoteId,
                        .seq = seq,
                    });
                for (size_t i = 0; i < 20; i++) {
                    const u16 row_bitfield = (u16)packet[4 + i * 2] |
                                             ((u16)packet[4 + i * 2 + 1] << 8);
                    boardUpdate.boardRows[i] = row_bitfield;
                }
            } else {
                fprintf(stderr, "Received unknown message of len=%d",
                        packet.size());
                for (size_t i = 0; i < packet.size(); i++) {
                    fprintf(stderr, "%02X ",
                            *((unsigned char*)packet.data() + i));
                    if (i % 16 == 15 && i + 1 != packet.size()) {
                        fputs("\n", stderr);
                    }
                }
                fputs("\n", stderr);
            }
        }
    }

    void
    registerDataChannelHandlers(const shared_ptr<rtc::DataChannel>& dataChannel,
                                const std::string& peerId) {
        dataChannel->onOpen([this, peerId]() {
            std::cout << "DataChannel from " << peerId << " opened"
                      << std::endl;
            initPlayer(playerStringToId[peerId]);
        });

        dataChannel->onClosed([this, peerId]() {
            std::cout << "DataChannel from " << peerId << " closed"
                      << std::endl;
        });

        dataChannel->onMessage(
            [this, peerId](auto data) { parseMessage(data, peerId); });

        dataChannel->onError([peerId](auto error) {
            std::cout << "Error on DataChannel to peer " << peerId
                      << ", was error:\n"
                      << error << std::endl;
        });

        dataChannelMap.emplace(peerId, dataChannel);
    }

    shared_ptr<rtc::PeerConnection>
    createPeerConnection(const weak_ptr<rtc::WebSocket>& wws,
                         std::string peerId) {
        auto pc = std::make_shared<rtc::PeerConnection>(config);

#ifndef WEB
        pc->onStateChange([](rtc::PeerConnection::State state) {
            std::cout << "State: " << state << std::endl;
        });

        pc->onGatheringStateChange(
            [](rtc::PeerConnection::GatheringState state) {
                std::cout << "Gathering State: " << state << std::endl;
            });
#endif

        pc->onLocalDescription(
            [wws, peerId](const rtc::Description& description) {
                json message = {{ID, peerId},
                                {TYPE, description.typeString()},
                                {DESCRIPTION, std::string(description)}};
                std::cout << "Websocket local description "
                          << to_string(message) << std::endl;
                if (auto wwss = wws.lock())
                    wwss->send(message.dump());
            });

        pc->onLocalCandidate([wws, peerId](const rtc::Candidate& candidate) {
            json message = {{ID, peerId},
                            {TYPE, "candidate"},
                            {CANDIDATE, std::string(candidate)},
                            {MID, candidate.mid()}};
            std::cout << "Websocket local candidate " << to_string(message)
                      << std::endl;
            if (auto wwss = wws.lock())
                wwss->send(message.dump());
        });

        pc->onDataChannel(
            [this, peerId](const shared_ptr<rtc::DataChannel>& dataChannel) {
                std::cout << "DataChannel from " << peerId
                          << " received with label \"" << dataChannel->label()
                          << "\"" << std::endl;
                registerDataChannelHandlers(dataChannel, peerId);
            });

        peerConnectionMap.emplace(peerId, pc);
        return pc;
    };
    [[nodiscard]] bool isActive() const { return active; }

    void deactivate() {
        if (!active)
            return;
        active = false;
        send(2 << 13); // BYE
        dataChannelMap.clear();
#ifndef WEB
        for (auto& it : peerConnectionMap)
            it.second->close();
#endif
        readQueues.clear();
        peerConnectionMap.clear();
        playerStringToId.clear();
        playerIdToString.clear();
        playerIdToName.clear();
        ws->close();
        localId = randomId(10);
        playerId = -1;
    }

    [[nodiscard]] bool sync() const { return isConnected(); }

    bool canRead(u8 pId) {
        if (pId == playerId)
            return false;
        std::string peerId = playerIdToString[pId];
        std::lock_guard<std::mutex> guard{readQueueLock};
        return !readQueues[peerId].empty();
    }

    u16 read(u8 pId) {
        std::string realRemoteId = playerIdToString[pId];
        std::lock_guard<std::mutex> guard{readQueueLock};
        auto front = readQueues[realRemoteId].front();
        readQueues[realRemoteId].pop();
        return front.packet;
    }

    std::optional<BoardUpdatePacket> readBoardUpdate() {
        std::lock_guard<std::mutex> guard{readQueueLock};
        if (incomingBoardUpdates.empty()) {
            return std::nullopt;
        }
        auto front = incomingBoardUpdates.front();
        incomingBoardUpdates.pop();
        return front;
    }

    bool send(u16 packet) {
#ifdef TRACY_ENABLE
        ZoneScopedN("link->send(u16)");
        ZoneTextF("0x%04x", packet);
        ZoneTextF("%s", stringifyMultiplayerPacket(packet).c_str());
#endif
        return send(reinterpret_cast<const std::byte*>(&packet), sizeof(u16));
    }
    template <size_t N> bool send(const std::array<std::byte, N>& packet) {
#ifdef TRACY_ENABLE
        ZoneScopedN("link->send(array)");
        ZoneTextF("len=%zu", packet.size());
#endif
        return send(packet.data(), packet.size());
    }
    bool send(const std::byte* data, size_t len) {
#ifdef TRACY_ENABLE
        ZoneScopedN("link->send(data, len)");
        ZoneTextF("len=%zu", len);
#endif
        try {
            for (auto& it : dataChannelMap) {
#ifdef TRACY_ENABLE
                ZoneScopedN("send_to_datachannel");
                ZoneTextF("peer=%s", it.first.c_str());
#endif
                if (!it.second->isOpen()) {
                    std::cout << "DataChannel to " << it.first
                              << " appears closed!" << std::endl;
                    if (it.second->isClosed()) {
#ifdef TRACY_ENABLE
                        ZoneTextF("DataChannel closed");
                        ZoneColor(tracy::Color::Red);
#endif
                    } else {
#ifdef TRACY_ENABLE
                        ZoneTextF("DataChannel not open");
                        ZoneColor(tracy::Color::Orange);
#endif
                    }
                    return false;
                }
                if (!it.second->send(data, len)) {
#ifdef TRACY_ENABLE
                    ZoneTextF("send returned false");
                    ZoneColor(tracy::Color::Orange);
#endif
                    return false;
                }
            }
            return true;
        } catch (std::exception& e) {
#ifdef TRACY_ENABLE
            ZoneTextF("send err: %s", e.what());
            ZoneColor(tracy::Color::Red);
#endif
            printf("%s", e.what());
            return true;
        }
    }
    bool send(std::string msg) {
#ifdef TRACY_ENABLE
        ZoneScopedN("link->send(string)");
        ZoneTextF("%s", msg.c_str());
#endif
        try {
            for (auto& it : dataChannelMap) {
#ifdef TRACY_ENABLE
                ZoneScopedN("send_to_datachannel");
                ZoneTextF("peer=%s", it.first.c_str());
#endif
                if (!it.second->isOpen()) {
                    std::cout << "DataChannel to " << it.first
                              << " appears closed!" << std::endl;
                    if (it.second->isClosed()) {
#ifdef TRACY_ENABLE
                        ZoneTextF("DataChannel closed");
                        ZoneColor(tracy::Color::Red);
#endif
                    } else {
#ifdef TRACY_ENABLE
                        ZoneTextF("DataChannel not open");
                        ZoneColor(tracy::Color::Orange);
#endif
                    }
                    return false;
                }
                if (!it.second->send(msg)) {
#ifdef TRACY_ENABLE
                    ZoneTextF("send returned false");
                    ZoneColor(tracy::Color::Orange);
#endif
                    return false;
                }
            }
            return true;
        } catch (std::exception& e) {
#ifdef TRACY_ENABLE
            ZoneTextF("send err: %s", e.what());
            ZoneColor(tracy::Color::Red);
#endif
            printf("%s", e.what());
            return true;
        }
    }

    [[nodiscard]] bool isConnected() const { return playerCount() > 1; }

    [[nodiscard]] u8 playerCount() const { return dataChannelMap.size() + 1; }

    [[nodiscard]] std::unordered_map<u8, std::string> playerNames() const {
        return playerIdToName;
    }
};

extern LinkUniversal* linkUniversal;
