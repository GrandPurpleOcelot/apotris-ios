#pragma once
#include "def.h"
#include <queue>
#include <future>
#include <algorithm>
#include "rtc/rtc.hpp"
#include <nlohmann/json.hpp>



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

class LinkUniversal {
    static constexpr char STUN_SERVER[] = "stun:stun.l.google.com:19302";
    static constexpr char TYPE[] = "type";
    static constexpr char ID[] = "id";
    static constexpr char DESCRIPTION[] = "description";
    static constexpr char OFFER[] = "offer";
    static constexpr char ANSWER[] = "answer";
    static constexpr char MATCH[] = "match";
    static constexpr char CLOSE[] = "close";
    static constexpr char FULL[] = "full";
    static constexpr char CANDIDATE[] = "candidate";
    static constexpr char MID[] = "mid";
    static constexpr char ITER[] = "iter";

    rtc::Configuration config;
    std::map<std::string, std::queue<Packet>> readQueues;
    bool active = false;
    shared_ptr<rtc::WebSocket> ws;
    int playerId = -1;
    std::unordered_map<std::string, u8> playerStringToId;
    std::unordered_map<u8, std::string> playerIdToString;
    std::unordered_map<std::string, shared_ptr<rtc::PeerConnection>> peerConnectionMap;
    std::unordered_map<std::string, shared_ptr<rtc::DataChannel>> dataChannelMap;
public:
    std::string localId = randomId(10);
    [[nodiscard]] u8 currentPlayerId() const {
        return playerId;
    }
    static std::string randomId(size_t length) {
        using std::chrono::high_resolution_clock;
        static const std::string characters(
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        std::string id(length, '0');
        std::uniform_int_distribution<int> uniform(0, int(characters.size() - 1));
        std::generate(id.begin(), id.end(), [&]() { return characters.at(qran() % (characters.length() - 1)); });
        return id;
    }
    shared_ptr<rtc::PeerConnection> getOrCreatePeerConnection(const std::string& peerId, const std::string& type) {
        if (auto jt = peerConnectionMap.find(peerId); jt != peerConnectionMap.end())
            return jt->second;
        else if (type == OFFER || type == MATCH)
            return createPeerConnection(make_weak_ptr(ws), peerId);
        else
            return nullptr;
    }
    void initWebSocketConfig() {
#ifndef WEB
        rtc::InitLogger(rtc::LogLevel::Debug);
#endif
        config.iceServers.emplace_back(STUN_SERVER);
        ws = std::make_shared<rtc::WebSocket>();

        ws->onClosed([]() { std::cout << "WebSocket closed" << std::endl; });
        ws->onMessage([this, wws = make_weak_ptr(ws)](auto data) {
            if (!std::holds_alternative<std::string>(data))
                return;
            std::cout << "Got websocket message: " << std::get<std::string>(data) << std::endl;
            json message = json::parse(std::get<std::string>(data));
            if (message.find(ID) == message.end() || message.find(TYPE) == message.end())
                return;

            std::string id = message[ID];
            std::string type = message[TYPE];
            if (type == MATCH) {
                int iter = message[ITER];
                int remotePlayerId = iter;
                if (id == localId) {
                    if (playerId != -1 && remotePlayerId == 0) {
                        playerIdToString.clear();
                        playerStringToId.clear();
                    }
                    playerId = remotePlayerId;
                    std::cout << "We think we are player ID " << playerId << std::endl;
                }
                playerIdToString[remotePlayerId] = id;
                playerStringToId[id] = remotePlayerId;
                clearPlayers();
                for (auto & i : playerIdToString) {
                    initPlayer(i.first);
                }
                if (id == localId)
                    return;
            }
            auto pc = getOrCreatePeerConnection(id, type);

            if (type == CLOSE) {
                if (dataChannelMap.find(id) != dataChannelMap.end())
                    dataChannelMap[id]->close();
                readQueues.erase(id);
                dataChannelMap.erase(id);
                peerConnectionMap.erase(id);
#ifndef WEB
                if (pc)
                    pc->close();
#endif
                int pId = playerStringToId[id];
                removePlayer(pId);
                //readQueues.erase(id);
                playerIdToString.erase(pId);
                playerStringToId.erase(id);
            }
            else if (type == MATCH && id != localId) {
                if (!pc || dataChannelMap.find(id) != dataChannelMap.end())
                    return;
                makeOffer(pc, id);
            }
            else if (type == OFFER || type == ANSWER)
                pc->setRemoteDescription(rtc::Description(message[DESCRIPTION], type));
            else if (type == CANDIDATE)
                pc->addRemoteCandidate(rtc::Candidate(message[CANDIDATE], message[MID]));
            else if (type == FULL)
                exceptionReason = "Room is full";
        });
    }
    void activate() {
        if (active) return;
        active = true;
        initWebSocketConfig();

#ifndef WEB
        std::promise<void> wsPromise;
        auto wsFuture = wsPromise.get_future();

        ws->onOpen([&wsPromise]() {
            std::cout << "WebSocket connected, signaling ready" << std::endl;
            wsPromise.set_value();
        });

        ws->onError([&wsPromise](const std::string& s) {
            std::cout << "WebSocket error" << std::endl;
            wsPromise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
        });
#else
        ws->onOpen([]() {
            std::cout << "WebSocket connected, signaling ready" << std::endl;
        });

        ws->onError([](const std::string& s) {
            std::cout << "WebSocket error" << std::endl;
        });
#endif

        std::string url = "wss://signaling.pingas.org/" + localId + (!roomId.empty() ? "/" + roomId : "/match");
        url.erase(remove(url.begin(), url.end(), ' '), url.end());

        std::cout << "WebSocket URL is " << url << std::endl;
        ws->open(url);

#ifndef WEB
        std::cout << "Waiting for signaling to be connected..." << std::endl;
        wsFuture.get();
#endif
    }

    void makeOffer(std::shared_ptr<rtc::PeerConnection> pc, const std::string& remoteId) {
        // We are the offerer if we have a lower ID
        if (playerId < playerStringToId[remoteId] && playerId != -1) {
            return;
        }
        std::stringstream ss;
        ss << "apotris:" << localId << std::to_string(playerId) << "<->" << std::to_string(playerStringToId[remoteId]);
        std::cout << "Creating DataChannel with label \"" << ss.str() << "\"" << std::endl;
        auto dc = pc->createDataChannel(ss.str());
        registerDataChannelHandlers(dc, remoteId);
    }

    void parseMessage(rtc::message_variant data, const std::string& peerId) {
        // data holds either std::string or rtc::binary
        if (std::holds_alternative<std::string>(data)) {
            return;
        }
        else {
            auto packet = std::get<rtc::binary>(data);
            if (packet.size() == 2) {
                std::byte foo = packet[0];
                std::byte bar = packet[1];
                u16 real = (u8)foo + ((u16)bar << 8);
                int realRemoteId = playerStringToId[peerId];
                readQueues[peerId].emplace((u8)(realRemoteId), real);
            }
        }
    }

    void registerDataChannelHandlers(const shared_ptr<rtc::DataChannel>& dataChannel, const std::string& peerId) {
        dataChannel->onOpen([this, peerId]() {
            std::cout << "DataChannel from " << peerId << " opened" << std::endl;
            initPlayer(playerStringToId[peerId]);
        });

        dataChannel->onClosed([this, peerId]() {
            std::cout << "DataChannel from " << peerId << " closed" << std::endl;
            dataChannelMap.erase(peerId);
            int pId = playerStringToId[peerId];
            removePlayer(pId);
            playerIdToString.erase(pId);
            playerStringToId.erase(peerId);
        });

        dataChannel->onMessage([this, peerId](auto data) {
            parseMessage(data, peerId);
        });

        dataChannel->onError([peerId](auto error){
           std::cout << "Error on DataChannel to peer " << peerId << ", was error:\n" << error << std::endl;
        });

        dataChannelMap.emplace(peerId, dataChannel);
    }

    shared_ptr<rtc::PeerConnection> createPeerConnection(const weak_ptr<rtc::WebSocket>& wws, std::string peerId) {
        auto pc = std::make_shared<rtc::PeerConnection>(config);

#ifndef WEB
        pc->onStateChange([](rtc::PeerConnection::State state) {
            std::cout << "State: " << state << std::endl;
        });

        pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
            std::cout << "Gathering State: " << state << std::endl;
        });
#endif

        pc->onLocalDescription([wws, peerId](const rtc::Description& description) {
            json message = {{ID, peerId},
                            {TYPE, description.typeString()},
                            {DESCRIPTION, std::string(description)}};
            std::cout << "Websocket local description " << to_string(message) << std::endl;
            if (auto wwss = wws.lock())
                wwss->send(message.dump());
        });

        pc->onLocalCandidate([wws, peerId](const rtc::Candidate& candidate) {
            json message = {{ID, peerId},
                            {TYPE, "candidate"},
                            {CANDIDATE, std::string(candidate)},
                            {MID, candidate.mid()}};
            std::cout << "Websocket local candidate " << to_string(message) << std::endl;
            if (auto wwss = wws.lock())
                wwss->send(message.dump());
        });

        pc->onDataChannel([this, peerId](const shared_ptr<rtc::DataChannel>& dataChannel) {
            std::cout << "DataChannel from " << peerId << " received with label \"" << dataChannel->label() << "\""
                      << std::endl;
            registerDataChannelHandlers(dataChannel, peerId);
        });

        peerConnectionMap.emplace(peerId, pc);
        return pc;
    };
    [[nodiscard]] bool isActive() const { return active; }

    void deactivate() {
        if (!active) return;
        active = false;
        dataChannelMap.clear();
#ifndef WEB
        for (auto & it : peerConnectionMap)
            it.second->close();
#endif
        peerConnectionMap.clear();
        playerStringToId.clear();
        playerIdToString.clear();
        ws->close();
        localId = randomId(10);
    }

    bool sync() const {
        return isConnected();
    }

    bool canRead(u8 pId) {
        if (pId == playerId) return false;
        std::string peerId = playerIdToString[pId];
        return !readQueues[peerId].empty();
    }

    u16 read(u8 pId) {
        std::string realRemoteId = playerIdToString[pId];
        auto front = readQueues[realRemoteId].front();
        readQueues[realRemoteId].pop();
        return front.packet;
    }

    bool send(u16 packet) {
        for (auto & it : dataChannelMap) {
            if (!it.second->isOpen()) {
                std::cout << "DataChannel to " << it.first << " appears closed!";
                return false;
            }
            if (!it.second->send(reinterpret_cast<const std::byte *>(&packet), sizeof(u16)))
                return false;
        }
        return true;
    }
    bool send(std::string msg) {
        for (auto & it : dataChannelMap) {
            if (!it.second->isOpen()) {
                std::cout << "DataChannel to " << it.first << " appears closed!";
                return false;
            }
            if (!it.second->send(msg))
                return false;
        }
        return true;
    }

    [[nodiscard]] bool isConnected() const {
        return playerCount() > 1;
    }

    [[nodiscard]] u8 playerCount() const {
        return dataChannelMap.size() + 1;
    }

};

extern LinkUniversal* linkUniversal;
