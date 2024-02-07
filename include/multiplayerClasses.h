#ifndef APOTRIS_MULTIPLAYERCLASSES_H
#define APOTRIS_MULTIPLAYERCLASSES_H

#include "def.h"

class RemotePlayerId;
class RemotePlayerIndex;

class RemotePlayerId{
public:
    u8 id;
    explicit RemotePlayerId(u8 playerId);
    [[nodiscard]] RemotePlayerIndex getIndex() const;
};

class RemotePlayerIndex{
public:
    u8 index;
    u8 originalId;
    RemotePlayerIndex(u8 index, u8 originalId);
    [[nodiscard]] RemotePlayerId getOriginalId() const;
};


#endif //APOTRIS_MULTIPLAYERCLASSES_H
