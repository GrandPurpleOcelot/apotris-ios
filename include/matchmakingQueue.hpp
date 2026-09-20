#pragma once

namespace MatchmakingQueue {

void start();
void cancel();
void acceptMatch();
void denyMatch();
void activateMatchFoundScene();
void matchStarted();
bool isQueued();
bool returnToWaitingAfterConnectionFailure();
void update();
void drawOverlay();

bool blocksSceneUpdate();
bool isTransitioningToBattle();

} // namespace MatchmakingQueue
