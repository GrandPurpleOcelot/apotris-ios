#include <iostream>
#include <algorithm>
#include <string>
#include <tuple>
#include <math.h>

#include "tetromino.hpp"
#include "blockEngine.h"
#include "logging.h"
#include "posprintf.h"

using namespace BlockEngine;
using namespace GameInfo;

int Game::checkRotation(int dx, int dy, int r) {
    const int x = dx + pawn.x;
    const int y = dy + pawn.y;

    int total = 0;
    for (int j = 0; j < 4; j++) {
        for (int i = pawn.boardLowest[r][j]; i >= 0; i--) {
            if (pawn.board[r][i][j] == 0)
                continue;
            
            if (x + j < 0 || x + j > lengthX - 1 || y + i < 0 || y + i > lengthY - 1 || board[i+y][j+x] != 0)
                return 0;

            if(++total >= 4)
                return 1;
        }
    }

    return 1;
}

int Game::checkSpecialRotation(int dx, int r){
    int x = dx + pawn.x;

    int total = 0;
    for (int i = 0; i < 4; i++) {
        if(total >= 4)
            break;


        for (int j = 0; j < 4; j++) {
            if (pawn.board[r][i][j] == 0)
                continue;

            total++;

            if (x + j < 0 || x + j > lengthX - 1 || pawn.y + i < 0 || pawn.y + i > lengthY - 1 || board[i+pawn.y][j+x] != 0)
                return i * 4 + j + 1;

            if(total >= 4)
                break;
        }
    }

    return 0;
}

void Pawn::setBlock(int system) {
    memset32(boardLowest,-1,16);
    memset32(heighest,-1,4);
    memset32(lowestBlock,0,4);

    for (int r = 0; r < 4; r++){
        int * shape = getShape(current,r,system);

        for(int iy = 0; iy < 4; iy++){
            for(int ix = 0; ix < 4; ix++){
                board[r][iy][ix] = shape[iy * 4 + ix];

                if(shape[iy * 4 + ix]){
                    if(boardLowest[r][ix] < iy)
                        boardLowest[r][ix] = iy;

                    if(lowestBlock[r] < iy)
                        lowestBlock[r] = iy;

                    if(heighest[r] == -1)
                        heighest[r] = iy;
                }
            }
        }

        delete[] shape;
    }
}

int Game::getNbr(int x, int y, int ex, int sy , int ey){
    int count = 0;

    if(y - 1 >= sy && board[y-1][x])
        count++;
    if(y + 1 <= ey-1 && board[y+1][x])
        count++;
    if(x - 1 >= 0 && board[y][x-1])
        count++;
    if(x + 1 <= ex-1 && board[y][x+1])
        count++;

    if(y - 1 >= sy && board[y-1][x] && x - 1 >= 0 && board[y][x-1] && board[y-1][x-1])
        count++;
    if(y - 1 >= sy && board[y-1][x] && x + 1 <= ex && board[y][x+1] && board[y-1][x+1])
        count++;
    if(y + 1 <= ey && board[y+1][x] && x - 1 >= 0 && board[y][x-1] && board[y+1][x-1])
        count++;
    if(y + 1 <= ey && board[y+1][x] && x + 1 <= ex && board[y][x+1] && board[y+1][x+1])
        count++;

    return count;
}

void Game::rotateCW(int dir) {
    rotates[0] = dir;

    replay.push_back(Timestamp(timer,2,dir));

    rotate(1,dir);
}

void Game::rotateCCW(int dir) {
    rotates[1] = dir;

    replay.push_back(Timestamp(timer,3,dir));

    rotate(-1,dir);
}

void Game::rotateTwice(int dir) {
    if(rotationSystem == NRS)
        return;

    rotates[2] = dir;

    replay.push_back(Timestamp(timer,4,dir));

    rotate(2,dir);
}

void Game::rotate(int dir, int on){

    if(!on)
        return;

    if ((clearLock || entryDelay)){
        fromLockRotate = true;
        return;
    }else if(eventLock){
        return;
    }

    if(rotationSystem == ARS && dir == 2)
        dir = -1;

    int dx = 0;
    int dy = 0;
    int r = pawn.rotation + dir;
    if(r > 3)
        r -= 4;
    else if(r < 0)
        r += 4;

    moveCounter++;

    if(rotationSystem == SRS){
        for (int i = 0; i < 5 + (dir > 1); i++) {
            int dx;
            int dy;
            if(dir <= 1){
                dx = kicks[(pawn.current == 0)][(dir < 0)][pawn.rotation][i][0];
                dy = kicks[(pawn.current == 0)][(dir < 0)][pawn.rotation][i][1];
            }else{
                dx = kickTwice[pawn.rotation][i][0];
                dy = kickTwice[pawn.rotation][i][1];
            }

            if (checkRotation(dx, dy, r)) {
                specialTspin = (pawn.current == 5 && (abs(dx) + abs(dy) >= 2));
                lastMoveRotation = dir;

                if(dy < 0)
                    verticalKick++;

                if(verticalKick >= verticalKickMax)
                    hardDrop();

                rotatePlace(dir,dx,dy,r);
                return;

            }else{
                sounds.invalid = 1;
            }
        }

    }else if(rotationSystem == NRS){
        if(checkRotation(0, 0, r)){
            rotatePlace(dir,dx,dy,r);
            return;
        }else
            sounds.invalid = 1;

    }else if(rotationSystem == ARS){

        if((pawn.current == 1 || pawn.current == 2 || pawn.current == 5) && (pawn.rotation % 2 == 0)){
            int n = checkSpecialRotation(0, r);
            if((n-1) % 4 == 1)
                return;
        }else if(pawn.current == 0){
            if(checkRotation(0, 0, r)){
                rotatePlace(dir,dx,dy,r);
                return;
            }else{
                int touching = checkITouching(r, (pawn.rotation % 2 == 0));
                if(pawn.rotation % 2 == 1){
                    if(touching == 1)
                        dx = 2;
                    else
                        dx = -1;
                }else{
                    if(touching)
                        dy = -2;
                }

                if(checkRotation(dx, dy, r)){
                    rotatePlace(dir,dx,dy,r);
                    return;
                }else
                    sounds.invalid = 1;
            }
            return;
        }

        for(int i = 0; i < 3; i++){
            dx = 0;
            dy = 0;

            if(i == 1 && pawn.current == 5 && r == 2){
                dy = -1;
            }else if(i == 1)
                dx = 1;
            else if(i == 2)
                dx = -1;

            if(checkRotation(dx, dy, r)){
                rotatePlace(dir,dx,dy,r);

                if((pawn.current == 0 || pawn.current == 5) && dy)
                    stopLockReset = true;

                return;
            }else
                sounds.invalid = 1;

            if(dy)
                break;
        }
    }
}

void Game::rotatePlace(int dir, int dx, int dy, int r){
    pawn.rotation = r;

    lockCheck();
    pawn.x += dx;
    pawn.y += dy;

    sounds.rotate = 1;

    switch(dir){
        case -1: moveHistory.push_front(5); break;
        case  1: moveHistory.push_front(4); break;
        case  2: moveHistory.push_front(6); break;
    }

    lastMoveDx = dx;
    lastMoveDy = dy;
}

bool Game::moveLeft() {
    if(das != maxDas)
        moveHistory.push_back(0);
    else if(!(moveHistory.size() > 0 && moveHistory.back() == 2)){
        if(moveHistory.size() > 0 && moveHistory.back() == 0)
            moveHistory.pop_back();

        moveHistory.push_back(2);
    }

    if (checkRotation(-1, 0, pawn.rotation)) {
        pawn.x--;
        sounds.shift = 1;
        lastMoveRotation = 0;
        lockCheck();
        return true;
    }else{
        pushDir = -1;
        if(gameMode == CLASSIC)
            das = maxDas;
        return false;
    }
}

bool Game::moveRight() {
    if(das != maxDas)
        moveHistory.push_back(1);
    else if(!(moveHistory.size() > 0 && moveHistory.back() == 3)){
        if(moveHistory.size() > 0 && moveHistory.back() == 1)
            moveHistory.pop_back();

        moveHistory.push_back(3);
    }

    if (checkRotation(1, 0, pawn.rotation)) {
        pawn.x++;
        sounds.shift = 1;
        lastMoveRotation = 0;
        lockCheck();
        return true;
    }else{
        pushDir = 1;
        if(gameMode == CLASSIC)
            das = maxDas;
        return false;
    }
}

void Game::moveDown() {
    if (checkRotation(0, 1, pawn.rotation)){
        pawn.y++;

        if(rotationSystem == ARS && !stopLockReset)
            lockTimer = maxLockTimer;

        if(gameMode != CLASSIC)
            sounds.shift = 1 ;
        lastMoveRotation = 0;
    }else if(rotationSystem == NRS || rotationSystem == ARS){
        place();
        down = 0;
    }
}

void Game::hardDrop() {
    if (clearLock || eventLock || dropLockTimer || entryDelay)
        return;

    int diff = pawn.lowest - pawn.y;

    if(diff)
        lastMoveRotation = 0;

    int offset = 0;
    for(int i = 0; i < 4; i++){
        if(pawn.boardLowest[pawn.rotation][i] != -1){
            offset = i;
            break;
        }
    }

    std::list<int> bestFinesse = getBestFinesse(pawn.current,pawn.x+offset,pawn.rotation);

    previousBest.clear();
    for(auto item : bestFinesse){
        previousBest.push_back(item);
    }

    int difference = 0;
    bool correct = false;
    if(moveHistory.size() > (int) bestFinesse.size()-1){
        for(const auto& move : bestFinesse){
            if(move == 6 && moveHistory.size() == (int) bestFinesse.size()){
                correct = true;
                break;
            }
        }
        if(!correct){
            difference = moveHistory.size() - (bestFinesse.size() - 1);
        }
    }else{
        correct = true;
    }

    if(!correct && !softDrop){
        finesseFaults += difference;
        finesseStreak = 0;

        if(trainingMode){
            lockTimer = maxLockTimer;
            lockMoveCounter = 15;
            verticalKick = 0;
            sounds.finesse = 1;
            refresh = 1;
            dropLockTimer = dropLockMax;
            softDrop = false;

            pushDir = 0;

            moveHistory.clear();
            previousKey = 0;

            pawn.y = (int)lengthY / 2;
            pawn.x = (int)lengthX / 2 - 2;
            pawn.rotation = 0;

            return;
        }
    }else{
        finesseStreak++;
    }

    pawn.y = pawn.lowest;

    // profile_start();
    if(rotationSystem != ARS)
        place();
    // log(std::to_string(profile_stop()));

    score += diff * 2;

    return;
}

void Game::update() {
    if(!disappearing)
        timer++;
    else
        eventTimer++;

    if(dropLockTimer)
        dropLockTimer--;

    if(zoneTimer > 1){
        zoneTimer--;
    }

    if (clearLock || lost)
        return;

    if(!disappearing)
        inGameTimer++;

    if (entryDelay){
        entryDelay--;
    }

    if(pawn.current == -1 && !entryDelay){
        next();
    }

    updateDas();

    if(entryDelay)
        return;


    if (das == maxDas && !(left && right)) {
        if (arr >= 0 && --arrCounter <= 0 && !(gameMode == CLASSIC && down)) {
            for(int i = 0; i < 1 + (arr == 0); i++){//move piece twice if arr is 0
                if (left)
                    moveLeft();
                else if(right)
                    moveRight();

                if(speed == 20){
                    int counter = 0;
                    while(counter++ < 20 && checkRotation(0,1,pawn.rotation))
                        pawn.y++;
                }
            }
            arrCounter = arr;
        }else if(arr < 0){
            bool c = true;
            while(c){
                if(left)
                    c = moveLeft();
                else if(right)
                    c = moveRight();

                if(speed == 20){
                    int counter = 0;
                    while(counter++ < 20 && checkRotation(0,1,pawn.rotation))
                        pawn.y++;
                }
            }
        }
    }

    if(gameMode == BATTLE){
        auto index = garbageQueue.begin();

        while(index != garbageQueue.end()){
            if(index->timer){
                index->timer--;
            }
            ++index;
        }
    }else if(gameMode == SURVIVAL){
        int rate = 60 * (4-goal);
        if(timer % rate == 0){
            generateGarbage(1,0);
            refresh = 1;

            while(!checkRotation(0,0,pawn.rotation)){
                if(!checkRotation(0,-1,pawn.rotation))
                    lost = 1;
                else
                    pawn.y--;
            }
        }
    }else if(gameMode == MASTER && !disappearing){
        if(level < 500 && timer >= 25200)
            won = 1;

        if(decayTimer && comboCounter < 2){
            decayTimer--;
        }else{
            decayTimer = gradeTable[internalGrade][1];
            if(gradePoints)
                gradePoints--;
        }
    }

    if((linesCleared >= goal && gameMode == SPRINT && goal && subMode == 0) ||
       (linesSent >= goal && gameMode == SPRINT && goal && subMode == 1) ||
       (goal && linesCleared >= goal && gameMode == MARATHON) ||
       (garbageCleared >= goal && gameMode == DIG) ||
       (timer > goal && (gameMode == ULTRA || gameMode == BLITZ)) ||
       (linesCleared >= goal && gameMode == CLASSIC && goal)){
       won = 1;
       if(gameMode == CLASSIC && goal){
           score+= (level + bTypeHeight) * 1000;
       }
    }else if(gameMode == MASTER && disappearing && (eventTimer) >= CREDITS_LENGTH){
        won = 1;

        creditGrade += creditPoints[4][disappearing-1];
    }

    if(gracePeriod){
        gracePeriod--;
    }else if(!zoneTimer)
        speedCounter += speed;

    int n = (int)speedCounter;
    for (int i = 0; i < n; i++) {
        if (checkRotation(0, 1, pawn.rotation)){
            pawn.y++;
            if(rotationSystem == ARS)
                lockTimer = maxLockTimer;
        }else if(rotationSystem == NRS){
            place();
        }
    }

    speedCounter -= n;

    pawn.lowest = lowest();

    bool placed = false;

    if(dropping){
        hardDrop();
        dropping = false;
        return;
    }else{
        if (pawn.lowest <= pawn.y && !zoneTimer)
            lockTimer--;

        if (lockTimer == 0 && rotationSystem != NRS){
            place();
            placed = true;
        }
    }

    if (down) {
        if(rotationSystem == NRS || gameMode == MASTER)
            softDropCounter = maxDas;
        else if (softDropCounter < maxDas)
            softDropCounter++;
    }else
        softDropCounter = 0;

    if (((!delaySoftDrop && down) || softDropCounter == maxDas) && !placed) {
        if(softDropSpeed >= 0 && ++softDropRepeatTimer >= softDropSpeed){
            int n = 1 + (softDropSpeed == 0); // move piece twice if softDropSpeed is 0

            for(int i = 0; i < n; i++){
                moveDown();
                if (pawn.y != pawn.lowest)
                    score++;
            }
            softDropRepeatTimer = 0;
        }else if (softDropSpeed < 0){
            int n = pawn.lowest - pawn.y;

            pawn.y = pawn.lowest;
            score+=n;
        }
    }

    if(disappearing)
        updateDisappear();

    while((int)moveHistory.size() > 10)
        moveHistory.pop_front();

    if(enableBot || replay.size() > MAX_REPLAY_SIZE){
        replay.clear();

        replayElligible = false;
    }
}

int Game::lowest() {
    int max = 0;

    if(stackHeight-4 >= pawn.y){
        for(int x = 0; x < 4; x++){
            if(pawn.boardLowest[pawn.rotation][x] == -1)
                continue;

            int n = pawn.boardLowest[pawn.rotation][x] + columnHeights[pawn.x+x];
            if(n > max)
                max = n;
        }

        return lengthY-(max)-1;
    }else{
        int start = pawn.y;
        // int start = stackHeight-4;
        // if(stackHeight-4 < pawn.y)
        //     start = pawn.y;

        for(int i = start; i < lengthY; i++){
            for(int j = 0; j < 4; j++){
                if(pawn.boardLowest[pawn.rotation][j] == -1)
                    continue;
                int x = pawn.x+j;
                int y = i+pawn.boardLowest[pawn.rotation][j];
                if(y >= lengthY || board[y][x])
                    return i-1;
            }
        }
    }


    return pawn.y;
}

void Game::place() {
    if(pawn.current == -1)
        return;

    lastDrop = calculateDrop();

    for (int j = 0; j < 4; j++) {
        for (int i = pawn.boardLowest[pawn.rotation][j]; i >= 0; i--) {
            if (pawn.board[pawn.rotation][i][j] == 0)
                continue;

            int x = pawn.x + j;
            int y = pawn.y + i;

            if (y > lengthY - 1 || x > lengthX)
                continue;

            board[y][x] = pawn.current + pawn.board[pawn.rotation][i][j];

            bitboard[y] |= 1 << x;
            if((lengthY-y) > columnHeights[x])
                columnHeights[x] = lengthY-y;

            if(disappearing == 1)
                disappearTimers[y][x] = MAX_DISAPPEAR;
            else if(disappearing == 2)
                disappearTimers[y][x] = 1;
        }
    }

    toEndZone = false;
    if ((pawn.y+pawn.lowestBlock[pawn.rotation]) <= (lengthY / 2 - 1)) {
        if(zoneTimer)
            toEndZone = true;
        else if(gameMode == TRAINING)
            clearBoard();
        else{
            lost = 1;
            return;
        }
    }

    int prevLevel = level;

    if (clear(lastDrop)){
        if(rotationSystem != NRS)
            comboCounter++;
        else
            comboCounter = 1;

        if(gameMode == MASTER)
            entryDelay = lineAre;

    } else {
        if(gameMode == COMBO && comboCounter)
            lost = 1;

        comboCounter = 0;

        int targetHeight = 9;
        if(pawn.big)
            targetHeight = 4;

        if(gameMode == DIG && garbageHeight < targetHeight){
            int toAdd = targetHeight-garbageHeight;
            if(garbageCleared+toAdd+garbageHeight <= goal)
                generateGarbage(toAdd,0);
            else if(garbageCleared+toAdd+garbageHeight > goal && garbageCleared+garbageHeight < goal)
                generateGarbage(goal-(garbageCleared+garbageHeight),0);
        }else if(gameMode == BATTLE){
            int sum = 0;
            std::list<Garbage>::iterator index = garbageQueue.begin();
            while(index != garbageQueue.end()){
                if(index->timer == 0){
                    sum+=index->amount;
                    garbageQueue.erase(index++);
                }else{
                    ++index;
                }
            }
            generateGarbage(sum,1);
        }

        if(gameMode == MASTER)
            entryDelay = areMax;
    }

    if(comboCounter > statTracker.maxCombo)
        statTracker.maxCombo = comboCounter;

    if(rotationSystem == NRS){
        int add = 0;
        while(pawn.y + add < lengthY-3)
            add+=4;

        entryDelay = 10 + add;

        down = 0;
    }else if (gameMode == MASTER){

        if(level % 100 < 99 && level < 998)
            level++;

        if (level % 100 >= 70 && prevLevel % 100 < 70){
            if(sectionTimeGoal[(level / 100)][0] > timer-sectionStart && (timer-sectionStart < previousSectionTime+120)){
                cool = true;
                previousSectionTime = timer-sectionStart;
            }else{
                previousSectionTime = 1000000;
            }
        }else if(cool && level % 100 >= 82 && prevLevel % 100 < 82){
            sounds.section = 1;
        }else if((level % 100 == 99 && prevLevel % 100 < 99 && level != 999) || (level == 998 && prevLevel != 998)){
            sounds.section = 2;
        }
    }

    if(entryDelay){
        pawn.current = -1;
        activePiece = -1;
    }

    canHold = true;

    // if (!clearLock && !entryDelay && !(zoneTimer == 1 && comboCounter > 0))
    if (!clearLock && !entryDelay)
        next();

    lockTimer = maxLockTimer;
    lockMoveCounter = 15;
    verticalKick = 0;
    dropLockTimer = dropLockMax;
    stopLockReset = false;

    sounds.place = 1;
    refresh = 1;

    pushDir = 0;
    previousKey = 0;
    pieceCounter++;

    moveHistory.clear();

    if(zoneTimer == 1 || toEndZone)
        endZone();

    if((maxClearDelay == 1 || gameMode == SURVIVAL) && clearLock){
        removeClearLock();
    }
}

int Game::clear(Drop drop) {
    int clearCount = 0;
    int attack = 0;
    int isTSpin = 0;
    int isPerfectClear = 1;
    int isBackToBack = 0;
    int isDifficult = 0;

    //check for t-spin
    if (pawn.current == 5 && lastMoveRotation && gameMode != CLASSIC) {
        int frontCount = 0;
        int backCount = 0;
        int x = pawn.x;
        int y = pawn.y;
        int offset = 2;

        switch (pawn.rotation) {
        case 0:
            frontCount += (board[y][x] != 0) + (board[y][x + offset] != 0);
            if (y + offset > lengthY - 1)
                backCount = 2;
            else
                backCount += (board[y + offset][x] != 0) + (board[y + offset][x + offset] != 0);
            break;
        case 1:
            frontCount += (board[y][x + offset] != 0) + (board[y + offset][x + offset] != 0);
            if (x < 0)
                backCount = 2;
            else
                backCount += (board[y][x] != 0) + (board[y + offset][x] != 0);
            break;
        case 2:
            frontCount += (board[y + offset][x] != 0) + (board[y + offset][x + offset] != 0);
            backCount += (board[y][x] != 0) + (board[y][x + offset]);
            break;
        case 3:
            frontCount += (board[y][x] != 0) + (board[y + offset][x] != 0);
            if (x > lengthX - 1)
                backCount = 2;
            else
                backCount += (board[y][x + offset] != 0) + (board[y + offset][x + offset] != 0);
            break;
        }
        if ((frontCount == 2 && backCount > 0) || (frontCount > 0 && backCount == 2 && specialTspin))
            isTSpin = 2;
        else if (frontCount > 0 && backCount == 2)
            isTSpin = 1;
    }

    //check for lines to clear
    int garbageToRemove = 0;
    int zonedBefore = zonedLines;

    std::list<int> linesToMove;

    int start = lastDrop.startY + boardOffset;
    int end = lastDrop.rawEndY + 1;

    if(zonedLines && !zoneTimer){
        start = 0;
        end = lengthY;
    }

    for (int i = start ; i < end; i++) {

        bool toClear = (bitboard[i] == fullLine);

        if (toClear) {
            if(!zoneTimer){
                linesToClear.push_back(i);
                lineClearArray[i] = true;
                clearCount++;
                if(gameMode == DIG && i >= lengthY-(garbageHeight))
                    garbageToRemove++;
            }else{
                linesToMove.push_back(i);
            }
        }
    }

    if(!linesToMove.empty()) {

        fixConnected(linesToMove);

        int offset = 0;
        for(auto const& line : linesToMove){
            int i = line - offset;
            if(i >= lengthY-zonedLines)
                break;

            for(int ii = i; ii < lengthY-1-zonedLines; ii++){
                for (int j = 0; j < lengthX; j++){
                    board[ii][j] = board[ii+1][j];
                }
                bitboard[ii] = bitboard[ii+1];
            }

            zonedLines++;

            for (int j = 0; j < lengthX; j++)
                board[lengthY-zonedLines][j] = 9;

            bitboard[lengthY-zonedLines] = fullLine;

            offset++;
        }

        if (zonedLines > zonedBefore) {
            sounds.clear = 1;
            clearCount = zonedLines - zonedBefore;
        }

        if (!inversion && zonedLines >= 8 && sounds.zone != -1) {
            inversion = true;
            sounds.zone = 2;
        }

        for(int x = 0; x < lengthX; x++){
            for(int y = lengthY-stackHeight; y < lengthY; y++){
                if(board[y][x]){
                    columnHeights[x] = lengthY-y;
                    break;
                }
            }
        }

        linesToMove.clear();
    }

    garbageHeight-=garbageToRemove;
    garbageCleared+=garbageToRemove;

    //FIXME: perfect clears should be counted during zone

    if (clearCount == 0){
        if(isTSpin > 0){
            // previousClear = Score(clearCount, scoring[1 + isTSpin * 3][0], comboCounter, isTSpin, isPerfectClear, isBackToBack, isDifficult, drop);
            score += scoring[1 + isTSpin * 3][0];
            sounds.tspin = 1;
            statTracker.tspins++;
        }

        return 0;
    }

    // if(pawn.big)
    //     clearCount/=2;

    if(!zoneTimer)
        linesCleared+=clearCount;

    auto index = linesToClear.begin();
    for (int j = 0; j < lengthY; j++) {
        if(j == *index){
            ++index;
            continue;
        }

        if(bitboard[j]){
            isPerfectClear = false;
            break;
        }
    }

    fixConnected(linesToClear);

    //check for level up
    int prevLevel = level;
    switch(gameMode){
    case MARATHON:
        level = ((int)linesCleared / 10) + 1;
        if(subMode && !zonedLines){
            if(zoneCharge < 32){
                if((zoneCharge + clearCount) >= 32){
                    sounds.meter = 1;
                    zoneCharge = 32;
                } else
                    zoneCharge += clearCount;
            }
        }
        break;
    case BLITZ:
        for(int i = 0; i < 15; i++){
            if(linesCleared < blitzLevels[i]){
                level = i+1;
                break;
            }
        }
        break;
    case CLASSIC:
        if(goal)
            break;

        if(initialLevel){
            int init = min(initialLevel*10+10,max(100,initialLevel*10-50));
            int requirement = linesCleared-init;
            if(requirement >= 0)
                level = (requirement/10) + initialLevel + 1;
        }else{
            level = ((int)linesCleared / 10);
        }
        break;
    case MASTER:
        if(level == 999){
            if(clearCount && disappearing)
                creditGrade += creditPoints[clearCount-1][disappearing-1];

            break;
        }

        level += clearCount + (clearCount > 2) + (clearCount > 3);

        if(level > 999)
            level = 999;

        if(level == 999 && !disappearing){

            disappearing = 1 + (coolCount == 9);

            sectionStart = timer;

            eventLock = true;

            for(int i = 0; i < lengthY; i++){
                for(int j = 0; j < lengthX; j++)
                    board[i][j] = 0;
                bitboard[i] = 0;
            }
        }

        int currentSectionLevel = level / 100;

        if(currentSectionLevel > prevLevel / 100){
            int currentSession = timer-sectionStart;

            if(cool){
                coolCount++;
                cool = false;
            }

            if(currentSession > sectionTimeGoal[currentSectionLevel-1][1]){//regret
                if(grade){
                    int newInternal = 31;
                    while(newInternal != 0 && gradeTable[newInternal][0] >= grade)
                        newInternal--;

                    internalGrade = newInternal;
                    grade--;
                    gradePoints = 0;
                    regretCount++;
                }
                sounds.section = -1;
            }

            sounds.levelUp = 1;
            sectionStart = timer;
            setMasterTuning();
        }

        break;
    }

    if(level < prevLevel)
        level = prevLevel;

    if (prevLevel < level && (gameMode == MARATHON || gameMode == BLITZ || gameMode == CLASSIC)){
        sounds.levelUp = 1;
    }

    if(gameMode == ZEN)
        prevLevel = 1;

    //calculate score
    int add = 0;
    if(clearCount <= 4){
        if(isTSpin == 0){
            //if no t-spin
            if(gameMode != CLASSIC){
                add += scoring[clearCount - 1][0] * prevLevel;
                isDifficult = scoring[clearCount - 1][1];
                attack = scoring[clearCount - 1][2];
            } else
                add += classicScoring[clearCount-1] * (level+1);

            statTracker.clears[clearCount-1]++;
        }else if(isTSpin == 1){
            //if t-spin mini
            add += scoring[clearCount + 4][0] * prevLevel;
            isDifficult = scoring[clearCount + 4][1];
            attack = scoring[clearCount + 4][2];
            statTracker.tspins++;
        }else if(isTSpin == 2){
            //if t-spin
            add += scoring[clearCount  + 7][0] * prevLevel;
            attack = scoring[clearCount + 7][2];
            isDifficult = true;
            statTracker.tspins++;
        }
    }

    stackHeight += clearCount;

    if(gameMode == MASTER && !disappearing){
        gradePoints += (int) ((float) gradeTable[internalGrade][clearCount+1] * masterComboMultiplayer[(comboCounter < 10)?comboCounter:9][clearCount-1] + 1) * ((int)(level/250)+1);

        if(gradePoints >= 100){
            gradePoints = 0;
            internalGrade++;

            if(internalGrade > 31)
                internalGrade = 31;
        }

        grade = gradeTable[internalGrade][0];
    }

    isBackToBack = previousClear.isDifficult && (clearCount == 4 || isTSpin == 2) && gameMode != CLASSIC;

    if (isBackToBack){
        add = (int)add * 3 / 2;
        b2bCounter++;
        attack+= 1;
    }else if (!isDifficult){
        b2bCounter = 0;
    }

    if(b2bCounter > statTracker.maxStreak)
        statTracker.maxStreak = b2bCounter;

    //add combo bonus
    add += scoring[16][0] * level * comboCounter;
    attack += comboTable[comboCounter];

    if (isPerfectClear && gameMode != CLASSIC && clearCount <= 4){
        if(gameMode != BLITZ)
            add += scoring[clearCount - 1 + 11][0] * level;
        else
            add += 3500 * level;
        attack += scoring[clearCount - 1 + 11][2];

        statTracker.perfectClears++;
    }

    if(zoneTimer){
        zoneScore += add;
    } else if (zonedLines){
        int n = (1 + fullZone + (zonedLines >= 8));
        zoneScore = zoneScore * n;
        score += zoneScore;
        zoneScore = 0;
    } else
        score += add;

    previousClear = Score(clearCount, add, comboCounter, isTSpin, isPerfectClear, isBackToBack, isDifficult, drop);

    sounds.clear = 1;

    if(!zoneTimer)
        clearLock = true;
    specialTspin = false;

    if(garbageQueue.size()){
        auto index = garbageQueue.begin();
        while(attack > 0 && index != garbageQueue.end()){
            bool cleared = false;
            if(attack > index->amount){
                attack -= index->amount;
                cleared = true;

                garbageQueue.erase(index++);
            }else{
                index->amount -= attack;
                attack = 0;
            }
            if(!cleared)
                ++index;
        }
    }

    if(attack){
        attackQueue.push_back(Garbage(timer & 0xff,attack));
        linesSent+=attack;
    }

    setSpeed();

    return 1;
}

Color Game::color(int n) {
    Color result;
    result.r = colors[n][0];
    result.g = colors[n][1];
    result.b = colors[n][2];

    return result;
}

void Game::fillBag() {
    if(rotationSystem == SRS){
        for (int i = 0; i < 7; i++)
            bag.push_back(i);
        bagCounter++;
    }else if(rotationSystem == ARS){
        bag.clear();

        //fill bag
        for(int i = 0; i < 35; i++)
            bigBag[i] = i / 7;

        //fill drought history
        for(int i = 0; i < 7; i++)
            pieceDrought[i] = 5;

        //fill piece history
        historyList.push_back(PIECE_Z);
        historyList.push_back(PIECE_Z);
        historyList.push_back(PIECE_S);
        historyList.push_back(PIECE_S);
    }
}

void Game::next() {//~21-29k cycles
    pawn.y = (int)lengthY / 2;
    pawn.x = (int)lengthX / 2 - 2;

    pawn.current = queue.front();
    activePiece = pawn.current;
    queue.pop_front();
    fillQueue(1);

    //for CLASSIC 2 in a row protection
    pieceHistory = pawn.current;

    pawn.setBlock(rotationSystem); //~15k cycles

    if(gameMode == CLASSIC){
        down = 0;
        if(pawn.current != 0 && pawn.current != 3)
            pawn.x++;
        if(pawn.current == 0)
            pawn.y--;
    }

    pawn.rotation = 0;
    if((irs && (((fromLockRotate) && maxClearDelay > 1) || !initialType)) || rotationSystem == ARS){
        if(rotationSystem == ARS){
            if(rotates[0])
                rotate(1,1);
            else if(rotates[1]){
                rotate(-1,1);
                if(rotates[2] && rotationSystem == ARS)
                    rotate(-1,1);
            } else if(rotates[2] && rotationSystem != ARS)
                rotate(2,1);
        }else{
            int sum = rotates[0] + (rotates[1] * -1) + (rotates[2] * ((rotationSystem == ARS)? -1 : 2));

            pawn.rotation = sum + 4 * (sum < 0);
        }

        sounds.initial = 1;
    }

    //check if stack has reached top 3 lines
    const int line = 24;
    bool check = bitboard[line] != 0;

    if(pawn.rotation && rotationSystem == SRS){
        int lowestPart = pawn.lowestBlock[pawn.rotation];

        int n = (40 - 19) - (lowestPart + pawn.y);

        pawn.y += std::min(n,0);
    }

    if (check || !checkRotation(0, 0, pawn.rotation) || gameMode == CLASSIC){
        pawn.y-=1;
        if (!checkRotation(0, 0, pawn.rotation)){
            if(zoneTimer && zonedLines) {
                if(!toEndZone)
                    endZone();
            } else if(gameMode == TRAINING || gameMode == NO_MODE || gameMode == ZEN)
                clearBoard();
            else{
                lost = 1;
            }
        }
    }

    lastMoveDx = 0;
    lastMoveDy = 0;

    arrCounter = 0;

    speedCounter = 0;

    softDrop = false;

    if(holding && canHold && ((ihs && ((fromLockHold && maxClearDelay > 1) || !initialType)) || rotationSystem == ARS)){
        hold(1);

        sounds.initial = 1;
    }
    fromLockRotate = false;
}

void Game::fillQueue(int count) {
    if(seed != 0){
        sqran(seed);
        seed+= qran();
    }

    for (int i = 0; i < count; i++) {
        if(rotationSystem == SRS){
            int n = qran() % bag.size();

            // std::list<int>::iterator index = bag.begin();
            // std::advance(index, n);

            queue.push_back(bag.array[n]);
            bag.erase(n);

            if (bag.size() == 0)
                fillBag();
        }else if (rotationSystem == NRS){
            int n = qran() % 8;

            if(n == 7 || n == pieceHistory)
                n = qran() % 7;

            queue.push_back(n);
        }else if (rotationSystem == ARS){
            int n = -1;
            int record = 0;
            int droughted = 0;
            if(queue.empty()){
                while(n == -1 || n == PIECE_O || n == PIECE_Z || n == PIECE_S){
                    n = qran() % 7;
                }

                queue.push_back(n);
                continue;
            }

            int r = 0;
            for(int i = 0; i < 6; i++){
                r = qran() % 35;

                n = bigBag[r];

                if(historyList.find(n) < 0)
                    break;

                //find most droughted
                for(int i = 0; i < 7; i++){
                    if(record < pieceDrought[i]){
                        record = pieceDrought[i];
                        droughted = i;
                    }
                }

                //insert droughted into bag
                bigBag[r] = droughted;

                //in case all previous rolls failed
                if(i == 5){
                    r = qran() % 35;
                    n = bigBag[r];
                }
            }

            //update droughted list
            for(int i = 0; i < 7; i++)
                pieceDrought[i]++;
            pieceDrought[n] = 0;

            //find most droughted
            //record is intentionally not reset; this bug was in the original game
            for(int i = 0; i < 7; i++){
                if(record < pieceDrought[i]){
                    record = pieceDrought[i];
                    droughted = i;
                }
            }

            //insert droughted into bag
            bigBag[r] = droughted;

            historyList.push_front(n);
            historyList.pop_back();

            queue.push_back(n);
        }
    }
}

void Game::hold(int dir) {
    holding = dir;

    replay.push_back(Timestamp(timer,5,dir));

    if((clearLock || entryDelay) && gameMode != CLASSIC){
        // holding = dir;
        fromLockHold = true;
        return;
    }else if(dir == 0){
        fromLockHold = false;
        return;
    }

    // if (!canHold || clearLock || entryDelay || gameMode == CLASSIC || pawn.current == -1)
    if (!canHold || gameMode == CLASSIC || pawn.current == -1)
        return;

    moveCounter++;
    canHold = false;

    if (held == -1) {
        int temp = fromLockRotate;
        held = pawn.current;
        next();
        fromLockRotate = temp;
    } else {
        int temp = held;
        held = pawn.current;
        pawn.current = temp;
        activePiece = temp;
        pawn.setBlock(rotationSystem);
		pawn.y = (int)lengthY / 2;
        pawn.x = (int)lengthX / 2 - 2;

        // if(pawn.big){
        //     pawn.x/=2;
        //     pawn.y/=2;
        // }
    }

    pawn.rotation = 0;
    if(rotationSystem == ARS || (irs && ((fromLockRotate && maxClearDelay > 1) || !initialType))){
        if(rotationSystem == ARS){
            if(rotates[0])
                rotate(1,1);
            else if(rotates[1]){
                rotate(-1,1);
                if(rotates[2] && rotationSystem == ARS)
                    rotate(-1,1);
            } else if(rotates[2] && rotationSystem != ARS)
                rotate(2,1);
        }else{
            int sum = rotates[0] + (rotates[1] * -1) + (rotates[2] * ((rotationSystem == ARS)? -1 : 2));

            pawn.rotation = sum + 4 * (sum < 0);
        }

        sounds.initial = 1;
    }

    //check if stack has reached top 3 lines
    bool check = (bitboard[24] != 0);

    if(pawn.rotation && rotationSystem == SRS){
        int lowestPart = 0;
        for(int i = 3; i >= 0; i--){//find height of lowest block in column
            for(int j = 0; j < 4; j++){
                if(pawn.board[pawn.rotation][i][j]){
                    lowestPart = i;
                    break;
                }
            }

            if(lowestPart == i)
                break;
        }

        int n = (40 - 19) - (lowestPart + pawn.y);

        pawn.y += std::min(n,0);
    }

    if (check || !checkRotation(0, 0, pawn.rotation) || gameMode == CLASSIC){
        pawn.y-=1;
        if (!checkRotation(0, 0, pawn.rotation)){
            if(zoneTimer && zonedLines)
                endZone();
            else if(gameMode == TRAINING || gameMode == NO_MODE || gameMode == ZEN)
                clearBoard();
            else{
                lost = 1;
            }
        }
    }

    sounds.hold = 1;
    holdCounter++;

    lockTimer = maxLockTimer;
    lockMoveCounter = 15;
    verticalKick = 0;
    arrCounter = 0;

    dropLockTimer = 0;
    fromLockHold = false;
    moveHistory.clear();

    statTracker.holds++;
}

int* BlockEngine::getShape(int n,int r, int rotationSystem) {
    int* result;
    result = new int[4 * 4];

    int * source;

    switch(rotationSystem){
    case SRS:
        source = (int * )tetrominos[n][r];
        break;
    case NRS:
        source = (int * )classic[n][r];
        break;
    case ARS:
        source = (int * )ars[n][r];
        break;
    default:
        source = (int * )tetrominos[n][r];
        break;
    }

    memcpy32(result,source,16);
    return result;
}

void Game::lockCheck() {
    if (pawn.lowest == pawn.y && lockMoveCounter > 0 && rotationSystem == SRS) {
        lockTimer = maxLockTimer;
        lockMoveCounter--;
    }
}

void Game::keyLeft(int dir) {
    moveCounter++;

    replay.push_back(Timestamp(timer,0,dir));

    if (clearLock || eventLock || entryDelay || (gameMode == CLASSIC && down)) {
        left = dir;
        return;
    }

    if (left == 0 && dir)
        moveLeft();
    left = dir;
    
    previousKey = -1;
    dropLockTimer = 0;

    if(gameMode == CLASSIC && dir){
        das = 0;
        arrCounter = 0;
    }

    if(!dir){
        pushDir = 0;

        if(directionCancel && gameMode != CLASSIC){
            das = 0;
            arrCounter = 0;
        }
    }
}

void Game::keyRight(int dir) {
    moveCounter++;

    replay.push_back(Timestamp(timer,1,dir));

    if (clearLock || eventLock || entryDelay || (gameMode == CLASSIC && down)) {
        right = dir;
        return;
    }

    if (right == 0 && dir)
        moveRight();
    right = dir;
    
    previousKey = 1;
    dropLockTimer = 0;

    if(gameMode == CLASSIC && dir){
        das = 0;
        arrCounter = 0;
    }

    if(!dir){
        pushDir = 0;
        if(directionCancel && gameMode != CLASSIC){
            das = 0;
            arrCounter = 0;
        }
    }
}

void Game::keyDown(int dir) {
    moveCounter++;
    replay.push_back(Timestamp(timer,6,dir));
    if (clearLock || eventLock || entryDelay) {
        if(!(entryDelay && rotationSystem == NRS))
            down = dir;
        return;
    }

    if (down == 0 && dir){
        moveDown();
        if (pawn.y != pawn.lowest)
            score++;
    }

    down = dir;
    softDrop = true;
    dropLockTimer = 0;
    gracePeriod = 0;

}

void Game::removeClearLock() {
    if(!clearLock)
        return;
    clearLock = false;

    auto index = linesToClear.rbegin();

    const int len = (int)linesToClear.size();

    int pos = linesToClear.back();
    for(int i = pos; i >= stackHeight-len; i--){
        if(*index == i){
            if(index != linesToClear.rend())
                index++;
        }else{
            memcpy32(board[pos],board[i],10);
            bitboard[pos] = bitboard[i];
            if(disappearing)
                memcpy16(disappearTimers[pos],disappearTimers[i],10);
            pos--;
        }
    }

    for(int i = stackHeight-len-1; i < stackHeight; i++){
        memset32(board[i],0,10);
        bitboard[i] = 0;
        if(disappearing)
            memset16(disappearTimers[i],0,10);
    }

    // for (int i = 0; i < len; i++) {
    //     for (int j = *index; j >= stackHeight-len; j--){
    //         log(std::to_string(j));
    //         memcpy32(board[j],board[j-1],10);
    //         bitboard[j] = bitboard[j-1];
    //         if(disappearing)
    //             memcpy16(disappearTimers[j],disappearTimers[j-1],10);
    //     }
    //     ++index;
    // }

    for(int x = 0; x < lengthX; x++){
        int found = -1;
        for(int y = lengthY-(columnHeights[x]-len)-1; y < lengthY; y++){
            if(board[y][x]){
                found = y;
                break;
            }
        }

        if(found >= 0){
            columnHeights[x] = lengthY-found;
        }else{
            columnHeights[x] = 0;
        }
    }

    if(gameMode == COMBO){
        for(int i = lengthY/2-4; i < lengthY; i++){
            if(bitboard[i] != 0){
                break;
            }
            for(int j = 0; j < 10; j++){
                if(j > 2 && j < 7)
                    continue;
                board[i][j] = (i+comboCounter) % 7 + 1;
                bitboard[i] |= 1 << j;
            }
            connectBoard(i, i);
        }
        stackHeight = lengthY/2 - 4;
    }

    linesToClear.clear();
    memset32(lineClearArray, 0, 40 / 4);

    if(!entryDelay && !zoneExit){
        next();
    }else if(zoneExit){
        zoneExit = false;
    }

    refresh = 2;
    zonedLines = 0;
}

void Game::resetSounds(){
    sounds = SoundFlags();
}

void Game::resetRefresh() {
    refresh = 0;
}

void Game::setLevel(int newLevel){
    initialLevel = level = newLevel;
    setSpeed();
}

void Game::setGoal(int newGoal){
    goal = newGoal;
}

void Game::generateGarbage(int height,int mode){
    int hole = qran() % lengthX;
    // shift up
    for(int i = 0; i < lengthY; i++){
        if(i < lengthY-height){
            for(int j = 0; j < lengthX; j++)
                board[i][j] = board[i+height][j];
            bitboard[i] = bitboard[i+height];
        } else{
            memset32(board[i],0,10);
            bitboard[i] = 0;
        }
    }

    for(int i = lengthY-height; i < lengthY; i++){
        int prevHole = hole;
        int count = 0;
        if(!mode || qran() % 10 < 3){
            do{
                hole = qran() % lengthX;
            }while(++count < 11 && ((!board[i-1][hole] && height < garbageHeight) || hole == prevHole));
        }
        for(int j = 0; j < lengthX; j++){
            if(j == hole)
                continue;
            board[i][j]=8;
        }

        bitboard[i] = 0x3ff & ~(1 << hole);
    }

    garbageHeight+=height;

    stackHeight -= height;

    updateColumns(height);

    if(pawn.big)
        return;

    for(int i = lengthY-height; i < lengthY; i++){
        connectBoard(i, i);
    }
}

void Game::keyDrop(int dir){
    replay.push_back(Timestamp(timer, 7,dir));

    if(dir == 0 && gameMode == MASTER)
        dropping = false;

    if(!dir)
        return;

    if(gameMode == CLASSIC)
        return;

    dropping = true;

    moveCounter++;
}

Drop Game::getDrop(){
    Drop result = lastDrop;
    lastDrop = Drop();
    return result;
}

Drop Game::calculateDrop(){
    Drop result;

    result.on = true;

    result.x = pawn.x;
    result.y = pawn.y;

    result.dx = lastMoveDx;
    result.dy = lastMoveDy;

    result.piece = pawn.current;
    result.rotation = pawn.rotation;
    result.rotating = lastMoveRotation;

    result.startX = pawn.x;

    int start = 0;
    int end = 3;
    while(pawn.boardLowest[pawn.rotation][start] == -1)
        start++;
    while(pawn.boardLowest[pawn.rotation][end] == -1)
        end--;

    result.startX += start;
    result.endX = pawn.x + end + 1;

    result.startY = pawn.y + pawn.heighest[pawn.rotation];
    
    int add = 0;
    for(int i = 0; i < 4; i++)
        if(pawn.boardLowest[pawn.rotation][i] > add)
            add = pawn.boardLowest[pawn.rotation][i];

    result.rawEndY = result.endY = pawn.y + add;

    if(rotationSystem != NRS){
        if(((pawn.current == 2 && pawn.rotation == 3) || (pawn.current == 1 && pawn.rotation == 1)))
            result.endY--;
    }else{
        if(((pawn.current == 2 && pawn.rotation == 1) || (pawn.current == 1 && pawn.rotation == 3)))
            result.endY--;
    }

    if(result.startY < stackHeight)
        stackHeight = result.startY;

    // if(pawn.big){
    //     result.startX *=2;
    //     result.endX *=2;
    //     result.startY *=2;
    //     result.endY *=2;
    //     result.rawEndY *=2;
    // }

    result.startY -= boardOffset;
    result.endY -= boardOffset;
    result.y -= boardOffset;

    return result;
}

void Game::setTuning(Tuning newTune){
    if(gameMode == CLASSIC)
        return;

    if(rotationSystem != ARS){
        ihs = newTune.ihs;
        irs = newTune.irs;
        initialType = newTune.initialType;
    }

    if(gameMode == MASTER)
        return;

    maxDas = newTune.das;
    arr = newTune.arr;
    softDropSpeed = newTune.sfr;
    dropLockMax = newTune.dropProtection;
    directionCancel = newTune.directionalDas;
    delaySoftDrop = newTune.delaySoftDrop;
    maxClearDelay = 20;
}

void Game::setMasterTuning(){

    int n = 0;
    int speedLevel = level + coolCount * 100;

    if(speedLevel >= 500){
        n = (speedLevel - 400)/100;
    }

    if(n > 8)
        n = 8;

    arr = 1;
    areMax = masterDelays[n][0];
    lineAre = masterDelays[n][1];
    maxDas = masterDelays[n][2];
    maxLockTimer = masterDelays[n][3];
    maxClearDelay = masterDelays[n][4];
    softDropSpeed = 1;
}

void Game::clearAttack(int id){
    std::list<Garbage>::iterator index = attackQueue.begin();

    while(index != attackQueue.end()){
        if(index->id == id){
            attackQueue.erase(index++);
        }else{
            ++index;
        }
    }
}

void Game::setWin(){
    won = true;
}

void Game::addToGarbageQueue(int id, int amount){
    if(amount <= 0)
        return;

    for(const auto& attack : garbageQueue)
        if(attack.id == id)
            return;

    garbageQueue.push_back(Garbage(id,amount));
}

int Game::getIncomingGarbage(){
    int result = 0;

    for(const auto& atck : garbageQueue){
        result+= atck.amount;
    }

    return result;
}

std::list<int> Game::getBestFinesse(int piece,int dx, int rotation){
    std::list<int> result;

    int r = 0;
    switch(piece){
    case 0:
        r = rotation % 2;
        break;
    case 1:
        r = rotation;
        break;
    case 2:
        r = rotation;
        break;
    case 3:
        r = 0;
        break;
    case 4:
        r = rotation % 2;
        break;
    case 5:
        r = rotation;
        break;
    case 6:
        r = rotation % 2;
        break;
    }

    for(int i = 0; i < 4; i++){
        result.push_back(finesse[piece][r][dx][i]);
        if(finesse[piece][r][dx][i] == 7)
            break;
    }

    return result;
}

void Game::setTrainingMode(bool mode){
    if(pawn.big)
        return;
    trainingMode = mode;
}

void Game::demoClear(){
    clearLock = true;

    for(int i = lengthY-2; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            board[i][j] = 8;
        }
    }

    clear(Drop());
}

void Game::demoFill(){
    clearLock = false;
    for(int i = lengthY-2; i < lengthY; i++){
        int n = qran() % lengthX;
        board[i][n] = 0;
    }
}

void Game::bType(int height){
    int n = 0;

    if(height)
        n = height * 2 + (height + 1)/2 - (height == 5);

    for(int i = lengthY-n; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            if(qran() % 2 && !(i == lengthY-n && j == 0)){
                board[i][j] = (qran() % 7);

                if(columnHeights[j] < lengthY-i)
                    columnHeights[j] = lengthY-i;

                bitboard[i] |= 1 << j;
            }
        }
    }

    // for(int i = 0; i < 10; i++){
    //     log(std::to_string(columnHeights[i]));
    // }

    // log(std::to_string(stackHeight));

    stackHeight -= n;

    // log(std::to_string(stackHeight));

    // updateColumns(height);
}

void Game::setSubMode(int sm){

    if(gameMode == MASTER && sm){
        setRotationSystem(ARS);

        return;
    }

    subMode = sm;

    if(sm && gameMode == CLASSIC){
        bType(bTypeHeight);
        goal = 25;
    }
}

void Game::setSpeed(){
    if ((gameMode == MARATHON || gameMode == TRAINING) && !(subMode && zoneTimer)){
        if(level)
            speed = gravity[(level < 19) ? level - 1 : 18];
        else
            speed = 0;
    } else if(gameMode == BLITZ)
        speed = blitzGravity[(level < 15) ? level - 1 : 14];
    else if(gameMode == CLASSIC)
        speed = classicGravity[(level < 30) ? level: 29];
    else if(gameMode == MASTER){
        int speedLevel = level + coolCount * 100;
        if(speedLevel >= 500)
            speed = masterGravity[29][1];
        else{
            for(int i = 28; i >= 0; i--){
                if(speedLevel >= masterGravity[i][0]){
                    speed = masterGravity[i][1];
                    break;
                }
            }
        }
        setMasterTuning();
    } else
        speed = gravity[0];

}

void Game::setRotationSystem(int rs){
    rotationSystem = rs;

    if(rs == ARS){
        queue.clear();

        fillBag();
        fillQueue(3);
    }
}

int Game::checkITouching(int r, int oriantation){
    int x = pawn.x;
    int y = pawn.y;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if(oriantation == 0){
                if((j == 1 || j == 3) && ((x + j < 0) || (x+j > lengthX - 1) || ((y + i >= 0 && y + i <= lengthY-1) && board[i+y][j+x])))
                    return j;
            }else{
                if(i == 2 && ((y + i > lengthY-1) || ((x + j >= 0 && x + j <= lengthX-1) && (board[y+i][x+j]))))
                    return i;
            }
        }
    }

    return 0;
}

void Game::updateDisappear(){
    if(!disappearing)
        return;

    bool found = false;

    for(int i = 0; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            if(disappearTimers[i][j] <= 1)
                continue;
            disappearTimers[i][j]--;

            if(disappearTimers[i][j] == 1){
                found = true;
                toDisappear.push_back(std::make_tuple(j,i));
            }
        }
    }

    if(found)
        sounds.disappear = 1;
}

void Game::removeEventLock(){
    eventLock = false;
    inversion = false;
}

void Game::activateZone(int dir){
    if(!dir)
        return;

    if(zoneCharge < 8 && !DIAGNOSE)
        return;

    if(DIAGNOSE)
        zoneCharge = 32;

    fullZone = (zoneCharge == 32);

    previousClear.isTSpin = 0;
    previousClear.isBackToBack = 0;
    previousClear.isPerfectClear = 0;

    zoneTimer = (float) 37.5 * zoneCharge;

    zoneStart = zoneTimer;

    zoneCharge = 0;

    sounds.zone = 1;
}

void Game::endZone(){
    if(zonedLines > statTracker.maxZonedLines)
        statTracker.maxZonedLines = zonedLines;

    zoneTimer = 0;
    sounds.zone = -1;

    clear(Drop());

    eventLock = true;
    zoneExit = true;

    previousClear.isPerfectClear = false;
}

void Game::clearBoard(){
    for(int i = 0; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            board[i][j] = 0;
        }
        bitboard[i] = 0;
    }

    memset32(columnHeights,0,10);

    garbageHeight = 0;
}

void Game::fixConnected(std::list<int> sourceList){
    std::list<std::tuple<int,int>> fixList;

    for(auto const& line : sourceList){
        bool found = false;
        if(!fixList.empty()){
            int n = std::get<0>(fixList.back());
            if(n == line){
                fixList.pop_back();
                found = true;
            }else if (n == line-1){
                std::get<1>(fixList.back())++;
                found = true;
            }
        }

        if(!found && line > 0){
            fixList.push_back(std::make_tuple(line-1,0));
        }

        if(line < lengthY-1){
            fixList.push_back(std::make_tuple(line+1,1));
        }
    }

    for(auto const & line : fixList){
        int iy = std::get<0>(line);
        int type = std::get<1>(line);
        for(int ix = 0; ix < lengthX; ix++){
            int n = board[iy][ix];
            board[iy][ix] = (n & 0xf) + (GameInfo::connectedFix[type][n >> 4] << 4);
        }
    }
}

void Game::connectBoard(int startY,int endY){
    int ex = lengthX;
    int sy = startY;
    int ey = endY;

    for(int iy = startY; iy < endY+1; iy++){

        for(int ix = 0; ix < lengthX; ix++){
            if(!board[iy][ix] || (board[iy][ix] >> 4) != 0)
                continue;

            int n = getNbr(ix, iy, ex, sy, ey);

            if(n == 1){
                if(iy > sy && board[iy-1][ix])
                    board[iy][ix] += 1 << 4;
                else if(iy < ey && board[iy+1][ix])
                    board[iy][ix] += 2 << 4;
                else if(ix > 0 && board[iy][ix-1])
                    board[iy][ix] += 3 << 4;
                else if(ix < ex && board[iy][ix+1])
                    board[iy][ix] += 4 << 4;
            }else if(n == 2){
                if(iy > sy && board[iy-1][ix] && iy < ey && board[iy+1][ix])
                    board[iy][ix] += 5 << 4;
                else if(ix > 0 && board[iy][ix-1] && ix < ex && board[iy][ix+1])
                    board[iy][ix] += 6 << 4;
                else if(iy < ey && board[iy+1][ix] && ix < ex && board[iy][ix+1])
                    board[iy][ix] += 7 << 4;
                else if(iy < ey && board[iy+1][ix] && ix > 0 && board[iy][ix-1])
                    board[iy][ix] += 8 << 4;
                else if(iy > sy && board[iy-1][ix] && ix < ex && board[iy][ix+1])
                    board[iy][ix] += 9 << 4;
                else if(iy > sy && board[iy-1][ix] && ix > 0 && board[iy][ix-1])
                    board[iy][ix] += 10 << 4;
            }else if(n == 3){
                if(iy < ey && board[iy+1][ix] && ix < ex && board[iy][ix+1] && board[iy+1][ix+1])
                    board[iy][ix] += 11 << 4;
                else if(iy < ey && board[iy+1][ix] && ix > 0 && board[iy][ix-1] && board[iy+1][ix-1])
                    board[iy][ix] += 12 << 4;
                else if(iy > sy && board[iy-1][ix] && ix < ex && board[iy][ix+1] && board[iy-1][ix+1])
                    board[iy][ix] += 13 << 4;
                else if(iy > sy && board[iy-1][ix] && ix > 0 && board[iy][ix-1] && board[iy-1][ix-1])
                    board[iy][ix] += 14 << 4;
                else if(iy > sy && board[iy-1][ix] && iy < ey && board[iy+1][ix] && ix < ex && board[iy][ix+1])
                    board[iy][ix] += 15 << 4;
                else if(ix > 0 && board[iy][ix-1] && ix < ex && board[iy][ix+1] && iy < ey && board[iy+1][ix])
                    board[iy][ix] += 16 << 4;
                else if(iy > sy && board[iy-1][ix] && iy < ey && board[iy+1][ix] && ix > 0 && board[iy][ix-1])
                    board[iy][ix] += 17 << 4;
                else if(ix > 0 && board[iy][ix-1] && ix < ex && board[iy][ix+1] && iy > sy && board[iy-1][ix])
                    board[iy][ix] += 18 << 4;
            }else if(n == 5){
                if(iy < ey && ix < ex && ix > 0 && board[iy+1][ix-1] && board[iy+1][ix+1])
                    board[iy][ix] += 19 << 4;
                else if(iy > sy && ix < ex && ix > 0 && board[iy-1][ix-1] && board[iy-1][ix+1])
                    board[iy][ix] += 20 << 4;
                else if(ix < ex && iy < ey && iy > sy && board[iy-1][ix+1] && board[iy+1][ix+1])
                    board[iy][ix] += 21 << 4;
                else if(ix > 0 && iy < ey && iy > sy && board[iy-1][ix-1] && board[iy+1][ix-1])
                    board[iy][ix] += 22 << 4;
            }else if(n == 8){
                board[iy][ix] += 23 << 4;
            }
        }
    }
}

void Game::liftKeys(){
    for(int i = 0; i < 3; i++)
        rotates[0] = false;

    holding = 0;
    left = 0;
    right = 0;
}

void Game::setOptions(Options newOptions){
    goal = newOptions.goal;
    setLevel(newOptions.level);
    setTuning(newOptions.tuning);

    trainingMode = newOptions.trainingMode;

    bTypeHeight = newOptions.bTypeHeight;
    setRotationSystem(newOptions.rotationSystem);
    setSubMode(newOptions.subMode);
}

void Game::updateColumns(int height){

    for(int x = 0; x < lengthX; x++){
        int found = -1;
        for(int y = lengthY-(columnHeights[x]+height)-1; y < lengthY; y++){
            if(board[y][x]){
                found = y;
                break;
            }
        }

        if(found >= 0){
            columnHeights[x] = lengthY-found;
        }else{
            columnHeights[x] = 0;
        }
    }
}

void Game::updateDas(){
    if (!(left || right)){
        if(gameMode != CLASSIC){
            das = 0;
            arrCounter = 0;
        }
    }else if (das < maxDas){
        das++;
    }
}

void Game::chargeDas(){
    if(!(left || right))
        return;

    das = maxDas;
}

int poll(int max, std::string buffer){
    int result = 0;

    for(int i = 0; i < max; i++){
        std::string c = buffer.substr(i,1);

        int n = fumenFormat.at(c);

        result += n * pow(64,i);
    }

    return result;
}

void Game::loadFumen(){

    // for(int i = 0; i < 20; i++){
    //     for(int j = 0; j < 10; j++){
    //         if(!fumenBoard[i][j])
    //             continue;

    //         int y = i + 20;
    //         int x = j;

    //         board[y][x] = fumenBoard[i][j];

    //         bitboard[y] |= 1 << x;
    //         if((lengthY-y) > columnHeights[x])
    //             columnHeights[x] = lengthY-y;

    //         if(stackHeight > y)
    //             stackHeight = y;
    //     }
    // }

    if((int) fumenString.size() < 5 || fumenString.substr(0,5) != "v115@"){
        log("invalid fumen");
        return;
    }

    log("valid fumen!");

    std::string buffer = fumenString.substr(5);

    int blockCount = 0;
    while(blockCount < 240 && buffer.size() >= 2){

        if(buffer[0] == '?')
            buffer = buffer.substr(1);

        int n = poll(2,buffer);
        buffer = buffer.substr(2);

        int block = n / 240 - 8;
        int count = n % 240 + 1;

        if(block == 0){
            blockCount += count;
            continue;
        }

        for(int i = 0; i < count; i++){
            int x = (blockCount + i) % 10;
            int y = (blockCount + i) / 10 + 17;

            if(y > 39)
                continue;

            board[y][x] = fumenBlocks[block];

            bitboard[y] |= 1 << x;
            if((lengthY-y) > columnHeights[x])
                columnHeights[x] = lengthY-y;

            if(stackHeight > y)
                stackHeight = y;
        }

        blockCount += count;
    }
}

Game::~Game(){
}

Game::Game(int gm, int sd, bool big){
    gameMode = gm;
    seed = initSeed = sd;

    for (int i = 0; i < 40; i++) {
        bitboard[i] = 0;
        lineClearArray[i] = false;

        for (int j = 0; j < 10; j++)
            board[i][j] = 0;
    }

    for(int j = 0; j < lengthX; j++)
        columnHeights[j] = 0;

    fillBag();

    if(gameMode == CLASSIC){
        maxLockTimer = 1;
        fillQueue(1);

        maxDas = 16;
        arr = 6;
        softDropSpeed = 2;
        gracePeriod = 90;

        rotationSystem = NRS;
    }else if(enableFumen && enableFumenQueue){
        for(int i = 0; i < 5; i++)
            queue.push_back(fumenQueue[i]);
    } else
        fillQueue(5);

    pawn = Pawn(0,0);
    pawn.big = big;

    if(gameMode == SPRINT)
        goal = 40;
    else if(gameMode == MARATHON)
        goal = 150;
    else if(gameMode == DIG){
        goal = 100;
        if(!big)
            generateGarbage(9,0);
        else
            generateGarbage(4,0);
    }else if(gameMode == COMBO){
        if(!big){
            for(int i = lengthY/2-4; i < lengthY; i++){
                for(int j = 0; j < 10; j++){
                    if(j > 2 && j < 7 && !(i == lengthY-2 && j < 5) && !(i == lengthY-1 && j < 4))
                        continue;
                    board[i][j] = i % 7 + 1;
                    bitboard[i] |= 1 << j;

                    if((lengthY-i) > columnHeights[j])
                        columnHeights[j] = lengthY-i;
                }
                connectBoard(i, i);
            }
            stackHeight = lengthY/2 - 4;
        }else{
            for(int i = (lengthY/4-1); i < lengthY/2; i++){
                for(int j = 0; j < 5; j++){
                    if(j != 0 && !(i == (lengthY/2)-2 && j < 3) && !(i == (lengthY/2)-1 && j < 2))
                        continue;
                    board[i*2][j*2] = i % 7 + 1;
                    board[i*2][j*2+1] = i % 7 + 1;
                    board[i*2+1][j*2] = i % 7 + 1;
                    board[i*2+1][j*2+1] = i % 7 + 1;
                }
            }
        }
    }else if(gameMode == MASTER){
        level = 0;
        speed = GameInfo::masterGravity[0][1];
        setMasterTuning();
    }

    if(big){
        lengthY /= 2;
        lengthX /= 2;
        boardOffset = lengthY/2;
        fullLine >>= 5;
    }

    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 10; j++)
            disappearTimers[i][j] = 0;
    }

    if(enableFumen)
        loadFumen();
}

Game::Game(Game * oldGame) {
    oldGame->removeClearLock();

    for (int i = 0; i < 40; i++) {
        bitboard[i] = oldGame->bitboard[i];
        for (int j = 0; j < 10; j++){
            board[i][j] = oldGame->board[i][j];
            disappearTimers[i][j] = oldGame->disappearTimers[i][j];
        }

        lineClearArray[i] = oldGame->lineClearArray[i];
    }

    for (int j = 0; j < 10; j++)
        columnHeights[j] = oldGame->columnHeights[j];

    queue = oldGame->queue;
    canHold = oldGame->canHold;
    bag = oldGame->bag;

    pawn = Pawn(oldGame->pawn);
    statTracker = Stats(oldGame->statTracker);

    held = oldGame->held;
    level = oldGame->level;
    score = oldGame->score;
    comboCounter = oldGame->comboCounter;
    linesCleared = oldGame->linesCleared;
    clearLock = oldGame->clearLock;
    gameMode = oldGame->gameMode;
    previousClear = Score(oldGame->previousClear);
    timer = oldGame->timer;
    inGameTimer = oldGame->inGameTimer;
    refresh = oldGame->refresh;
    won = oldGame->won;
    lost = oldGame->lost;
    goal = oldGame->goal;
    finesseFaults = oldGame->finesseFaults;
    garbageCleared = oldGame->garbageCleared;
    garbageHeight = oldGame->garbageHeight;
    pushDir = oldGame->pushDir;
    b2bCounter = oldGame->b2bCounter;
    bagCounter = oldGame->bagCounter;
    seed = oldGame->seed;
    initSeed = oldGame->initSeed;
    subMode = oldGame->subMode;
    initialLevel = oldGame->initialLevel;
    trainingMode = oldGame->trainingMode;
    clearLock = oldGame->clearLock;
    linesCleared = oldGame->linesCleared;
    dropping = oldGame->dropping;
    stackHeight = oldGame->stackHeight;
    pieceCounter = oldGame->pieceCounter;
    speed = oldGame->speed;
    activePiece = oldGame->activePiece;
    linesToClear = oldGame->linesToClear;
}
