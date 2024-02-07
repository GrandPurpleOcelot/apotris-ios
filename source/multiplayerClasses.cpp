#include "multiplayerClasses.h"
RemotePlayerId::RemotePlayerId(u8 playerId) {
    id = playerId;
}

u8 getEnemyIndex(u8 id) {
    if(id > currentPlayerId)
        id -= 1;
    return id;
}

RemotePlayerIndex RemotePlayerId::getIndex() const {
    return {getEnemyIndex(id), id};
}

RemotePlayerIndex::RemotePlayerIndex(u8 index, u8 originalId) {
    this->index = index;
    this->originalId = originalId;
}

RemotePlayerId RemotePlayerIndex::getOriginalId() const {
    return RemotePlayerId(originalId);
}
