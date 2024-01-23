#include "scene.hpp"
#include "sprites.h"
#include "logging.h"
#include "sceneModes.hpp"
#include <cstring>

static int lastPlayerCount = -1;
static bool sentReady = false;
#define PLAYER_READY 2
#define GAME_START 3

void PlayOptionScene::init(){
    loadSpriteTiles(620,&sprite49tiles_bin,1,1);
    for(int i = 0; i < 5; i++){
        proSprites[i] = &obj_buffer[10 + i];
        sprite_hide(proSprites[i]);
    }

    OptionListScene::init();

    if(elementList.empty()){
        onStart = true;
        if(getBoard(subMode, goal) != nullptr)
            onScore = true;
    }

    startSprite.setText("START");
}

bool PlayOptionScene::control(){
    MenuKeys k = savefile->settings.menuKeys;

    if(onScore && key_is_down(k.special1) && key_is_down(k.special2) && key_is_down(k.special3)){
        resetScoreboard();
        refreshText = true;
        return false;
    }

    if(key_hit(k.cancel)){
        sfx(SFX_MENUCANCEL);
        if(!(onScore || onStart) || options == 0){
            previousElement = name();
            previousSelection = selection;

            if(path.size())
                path.pop_back();

            changeScene(previousScene(),Transitions::FADE);
            return true;
        }else if(onStart){
            onStart = false;
        }

        if(onScore){
            onScore = false;
            refreshText = true;
            movingDirection = -1;
            listStart = 0;

        }
    }

    if(!(onScore || onStart)){
        if(key_hit(k.left) && !elementList.empty()){
            auto it = elementList.begin();
            std::advance(it,selection);

            (*it)->action(-1);

            refreshOption = true;
        }

        if(key_hit(k.right) && !elementList.empty()){
            auto it = elementList.begin();
            std::advance(it,selection);

            (*it)->action(1);

            refreshOption = true;
        }
    }

    int prev = listStart;

    if (key_hit(k.up)) {
        movingDirection = -1;

        bool sound = true;

        if(onScore && !elementList.empty()) {
            onScore = false;
            onStart = false;
            refreshText = true;
        }else if(onStart && !elementList.empty()){
            onStart = false;
        }else{
            if (selection > 0)
                selection--;

            if(elementList.empty())
                sound = false;
        }

        if(selection == options -1)
            listStart = max(0,selection-elementMax);
        else if(selection < listStart + 1){
            listStart--;
            if(listStart < 0)
                listStart = 0;
        }

        if(sound)
            sfx(SFX_MENUMOVE);

        refreshText = true;
    }

    if (key_hit(k.down)) {
        if(!onScore){
            movingDirection = 1;

            if (selection == options - 1){
                if(getBoard(subMode, goal) != nullptr)
                    movingScore = true;
                else
                    onStart = true;
            } else
                selection++;

            if(selection == 0)
                listStart = 0;
            else if(selection > listStart + elementMax)
                listStart++;
            refreshText = true;
            sfx(SFX_MENUMOVE);
        }
    }

    if (!onScore && (key_is_down(k.up) || key_is_down(k.down))) {
        if (dasVer < maxDas) {
            dasVer++;
        } else {
            int dir = (key_is_down(k.up)? -1 : 1);

            if (selection + dir >= 0 && selection + dir <= options-1 && arr++ > maxArr) {
                arr = 0;

                movingDirection = dir;

                selection += dir;

                if(dir < 0){
                    if(selection == options -1)
                        listStart = max(0,selection-elementMax);
                    else if(selection < listStart + 1){
                        listStart--;
                        if(listStart < 0)
                            listStart = 0;
                    }
                }else{
                    if(selection == 0)
                        listStart = 0;
                    else if(selection > listStart + elementMax)
                        listStart++;

                }

                refreshText = true;
                sfx(SFX_MENUMOVE);
            }
        }
    } else {
        dasVer = 0;
    }

    if (!(onScore || onStart) && (key_is_down(k.left) || key_is_down(k.right))) {
        if (dasHor < maxDas) {
            dasHor++;
        } else{
            int dir = (key_is_down(k.left)? -1 : 1);
            if (arr++ > maxArr) {
                arr = 0;

                auto it = elementList.begin();
                std::advance(it,selection);

                (*it)->action(dir);

                refreshOption = true;
            }
        }
    } else {
        dasHor = 0;
    }

    if(prev != listStart){
        moving = true;
    }

    if(key_hit(k.pause) || key_hit(k.confirm)){
        if(!onScore && getBoard(subMode, goal) != nullptr){
            movingScore = true;
            sfx(SFX_MENUCONFIRM);
        } else if(!onStart){
            onStart = true;
            sfx(SFX_MENUCONFIRM);
        } else{
            start();
        }
    }

    return false;
}

void PlayOptionScene::start(){
    bool training = false;
    int subMode = 0;
    int goal = 0;
    int bTypeHeight = 0;

    for(auto const& element : elementList){
        std::string s = element->getLabel();
        int value = element->getValue();
        log(s);
        log(value);
        if(s == "Level"){
            level = value;
        }else if(s == "Type" || s == "Rules"){
            subMode = value;
        }else if(s == "Lines"){
            goal = value;
            mode = element->value;
        }else if(s == "Height"){
            bTypeHeight = value;
        }else if(s == "Finesse Training"){
            training = value;
        }else if(s == "Difficulty"){
            if(name() == "CPU Battle"){
                botThinkingSpeed = value*3;
                botSleepDuration = 5-value*2;
                botStepMax = 10;
            }else{
                goal = value;
                mode = element->value;
            }
        }else if(s == "Minutes"){
            goal = value;
            mode = element->value;
        }
    }

    BlockEngine::Options newOptions;

    newOptions.mode = getMode();

    if(newOptions.mode == BlockEngine::BLITZ)
        goal = 2 * 3600;

    if(newOptions.mode == BlockEngine::CLASSIC){
        newOptions.rotationSystem = BlockEngine::NRS;
        newOptions.randomizer = BlockEngine::RANDOM;
    } else{
        newOptions.rotationSystem = savefile->settings.rotationSystem;

        if(savefile->settings.randomizer >= 0 && savefile->settings.randomizer <= 2){
            newOptions.randomizer = savefile->settings.randomizer;
        }else{
            newOptions.randomizer = BlockEngine::BAG_7;
        }
    }

    newOptions.goal = goal;
    newOptions.level = level;
    newOptions.tuning = getTuning();
    newOptions.bigMode = savefile->settings.big;
    newOptions.subMode = subMode;
    newOptions.trainingMode = training;
    newOptions.bTypeHeight = bTypeHeight;

    MenuKeys k = savefile->settings.menuKeys;

    proMode = (key_is_down(k.special1) || key_is_down(k.special2)) ^ (savefile->settings.pro);

    startGame(newOptions,qran());

    gameLoop();
}

void PlayOptionScene::draw(){
    fallingBlocks();

    const int space = 1;
    const int startX = 3 * 8;
    const int endX = 30-4;

    int offset = (sinLut(cursorFloat) * 2) >> 12;
    FIXED scale = float2fx((1.0 - ((float) 0.1 * offset)));

    int verticalOffset = 0;
    if(moving){
        int temp = lerp((movingDirection) * (space + 1) * 8,0,64*movingTimer);
        verticalOffset = lerp(temp,0,64*movingTimer);
        setLayerScroll(2, 0, -verticalOffset);
    }else{
        setLayerScroll(2, 0, 0);
    }

    EntryBoard *board = getBoard(subMode, goal);

    if(!onScore){
        int i = 0;//drawing index, starts at listStart
        int counter = 0;//element index
        for(auto & option : elementList){
            if(counter < listStart){
                counter++;
                continue;
            }

            if(i > elementMax + 1){ // break if elements drawn reached max
                continue;
            }

            std::string opt = option->getCurrentOption();

            const int index = (i + listStart) % (elementMax + 2);

            if(refreshOption || refreshText){
                if(counter == selection)
                    wordSprites[index]->setText(option->getCursor(opt)); //draw option with text cursor
                else
                    wordSprites[index]->setText(" " + opt); // draw option without text cursor
            }

            if(refreshText)
                labels[index]->setText(option->getLabel());

            const int x = (endX) * 8 - getVariableWidth(opt);
            const int y = startY + 8 * (space + 1) * i + verticalOffset;

            wordSprites[index]->setPriority(0);
            wordSprites[index]->show(x, y, 15 - (counter != selection) - (i == elementMax + 1 || (listStart && i == 0)));

            labels[index]->show(startX, y, 15 - ((counter != selection) + (i == elementMax + 1 || (listStart && i == 0))));

            if(counter == selection){ //draw sprite cursor
                currentOption = opt;
                currentElement = option->getLabel();
                for(int i = 0; i < 2; i++){
                    sprite_unhide(cursorSprites[i],0);
                    sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 1);
                    sprite_enable_affine(cursorSprites[i], i, true);
                    sprite_set_size(cursorSprites[i], scale , i);

                    int x = startX - ((8) + offset + 4) - 4;

                    sprite_set_pos(cursorSprites[i], x, y - 5);
                }
            }

            i++;
            counter++;
        }

        for(int j = i; j < elementMax + 2; j++){
            int index = (j + listStart) % (elementMax + 2);

            wordSprites[index]->hide();
            labels[index]->hide();
        }

        for(int i = elementMax + 2; i < 9; i++)
            wordSprites[i]->hide();

        for(int i = 0; i < 5; i++)
            sprite_hide(proSprites[i]);
    }else{
        if(board != nullptr){
            for(int i = 0; i < 5; i++){
                const int y = startY + 8 * (1) * i + verticalOffset;

                char buff[12];
                posprintf(buff, "%d. %s",i+1,board->entries[i].name);

                labels[i]->setText(buff);
                labels[i]->show(startX,y,15);

                std::string str = "";
                if(getIfGrade() && board->entries[i].value > 0){
                    str += GameInfo::masterGrades[board->entries[i].grade];
                    str += " ";
                }

                if(!getIfTime()){
                    str += std::to_string(board->entries[i].value);
                } else
                    str += timeToString(board->entries[i].value,false);

                const int x = (endX) * 8 - getVariableWidth(str);
                wordSprites[i]->setText(str);
                wordSprites[i]->show(x,y,15);

                if(board->entries[i].pro){
                    sprite_unhide(proSprites[i],0);

                    sprite_set_attr(proSprites[i], ShapeSquare, 0, 620, 15, 0);
                    sprite_set_pos(proSprites[i],endX * 8+ 4,y);
                }else{
                    sprite_hide(proSprites[i]);
                }
            }
        }

        for(int j = 5; j < elementMax + 2; j++)
            labels[j]->hide();

        for(int j = 6; j < elementMax + 2; j++)
            wordSprites[j]->hide();

        int i = 0;
        for(auto const& option : elementList){
            if(option->getLabel() == "Level" && !(name() == "Classic" && subMode))
                continue;

            wordSprites[6 + i]->setText(option->getCurrentOption());
            wordSprites[6 + i]->show(30 * 8 - 4 - wordSprites[6+i]->width,4 + i * 8,14);

            i++;
        }
    }

    if(onScore || board == nullptr){
        startSprite.show((240 - 43) /2, 132,15);

        if(onScore || onStart){
            for( int i = 0; i < 2; i++){
                sprite_unhide(cursorSprites[i],0);
                sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 1);
                sprite_enable_affine(cursorSprites[i], i, true);
                sprite_set_size(cursorSprites[i], scale , i);

                int x = 240/2 - ((43 + 8) / 2 + offset + 4) * ((i)?-1:1) - 10;

                sprite_set_pos(cursorSprites[i], x, 132 - 5);
            }
        }

        for(int i = 0; i < 2; i++){
            sprite_hide(scrollSideSprites[i]);
        }
    }else{
        startSprite.hide();
    }

    if(listStart > 0){
        sprite_unhide(arrowSprites[0], 0);
        sprite_set_attr(arrowSprites[0], ShapeWide, 0, 1, 14, 0);
        sprite_set_pos(arrowSprites[0], 240 / 2 - 8, startY - 11 - 16);
    }else{
        sprite_hide(arrowSprites[0]);
    }

    if(!onScore && getBoard(subMode, goal) != nullptr){
        sprite_unhide(arrowSprites[1], 0);
        sprite_set_attr(arrowSprites[1], ShapeWide, 0, 1, 14, 0);
        sprite_enable_flip(arrowSprites[1], false, true);
        sprite_set_pos(arrowSprites[1], 240 / 2 - 8, (elementMax + 2) * (1 + space) * 8 + startY - 48);
    }else{
        sprite_hide(arrowSprites[1]);
    }

    if(refreshText || refreshOption){
        setOptions();
        std::string next = getDescription(name(),currentElement,currentOption);

        if(next == "" || next != scrollText){
            scrollText = next;
            clearSpriteTiles(672, 144, 1);
            if(scrollText != ""){
                scrollTextLength = getVariableWidth(scrollText);

                const int width = (savefile->settings.aspectRatio == 0)? 240 : 214;

                endDelay = 0;
                scrollOffset = - max((width - scrollTextLength),0) / 2;
                scrollDelay = 0;
                scrollingText[0]->setTextSlow(scrollText);

                #ifdef TE
                for(int i = 0; i < 2; i++){
                    sprite_unhide(scrollSideSprites[i], ATTR0_AFF);
                    sprite_set_pos(scrollSideSprites[i],240/2 + ((i*2)-1)*(min(scrollTextLength,width)/2) - 16 * (i == 0), 152 - 4);
                }
                #endif
            }else{
                scrollingText[0]->setTextSlow(scrollText);

                for(int i = 0; i < 2; i++){
                    sprite_hide(scrollSideSprites[i]);
                }
            }
        }
    }

    showSprites(128);

    refreshText = false;
    refreshOption = false;
}

void PlayOptionScene::update(){
    if(movingScore > 0){
        if(++movingScoreTimer >= movingScoreTimerMax){
            if(++listStart >= options - 1){
                movingScore = 0;
                onScore = true;
                onStart = true;
                clearSpriteTiles(672, 144, 1);
                scrollingText[0]->setText("");
                scrollText = "";
                selection = elementList.size() - 1;
            }
        }
    }

    OptionListScene::update();
}

void PlayOptionScene::setOptions(){
    for(auto const& element : elementList){
        std::string s = element->getLabel();
        int value = element->value;
        if(s == "Level"){
            level = value;
        }else if(s == "Type" || s == "Rules"){
            subMode = value;
        }else if(s == "Lines"){
            goal = value;
        }else if(s == "Minutes"){
            goal = value;
        }else if(s == "Difficulty"){
            goal = value;
        }
    }
}

void ClassicScene::update(){
    if(!subMode){
        if(elementList.back()->getLabel() == "Height"){

            Element *e = elementList.back();
            elementList.pop_back();

            delete e;
            options--;
        }
    }else{
        if(elementList.back()->getLabel() != "Height"){
            elementList.push_back(new IntSelectorElement("Height",{0,1,2,3,4,5}));
            refreshText = true;
            options++;
        }
    }

    PlayOptionScene::update();
}

void MultBattleScene::init(){
    PlayOptionScene::init();

    #ifdef GBA

    linkConnection->activate();

    #endif

    clearSpriteTiles(672, 144, 1);
}

void MultBattleScene::draw(){
    fallingBlocks();

    if (connected < 1) {
        aprint("Waiting for", 7, 7);
        aprint("Link Adapter", 9, 9);
        aprint("connection...", 11, 11);
    } else {
        clearText();
        aprint("Connected!", 10, 6);
    }

    showSprites(128);
}

bool MultBattleScene::control(){
    MenuKeys k = savefile->settings.menuKeys;

    if(key_hit(k.cancel)){
        sfx(SFX_MENUCANCEL);
        if(!onScore || options == 0){
            previousElement = name();
            previousSelection = selection;

            if(path.size())
                path.pop_back();

            changeScene(previousScene(),Transitions::FADE);

            #ifdef GBA
            linkConnection->deactivate();
            #endif

            return true;
        }else{
            onScore = false;
            refreshText = true;
            movingDirection = -1;
            listStart = 0;
        }

    }

    return false;
}

void MultBattleScene::update(){
    PlayOptionScene::update();

    #ifdef GBA
    linkConnection->sync();
    // Store state of all player IDs
    u8 currentPlayerId = linkConnection->currentPlayerId();
    u8 currentPlayerCount = linkConnection->playerCount();
    if (lastPlayerCount < 0 || currentPlayerCount == 0)
        lastPlayerCount = currentPlayerCount;

    if (linkConnection->isConnected()) {
        // For player 1 to inform other players to start game
        if(multiplayerStart){
            multiplayerStart = false;
            linkConnection->send(GAME_START);
            startMultiplayerGame(nextSeed);
            gameLoop();
            return;
        }

        MenuKeys k = savefile->settings.menuKeys;

        // Poll all remotes for data
        for (int remotePlayerId = 0; remotePlayerId < currentPlayerCount; remotePlayerId++)
            while(remotePlayerId != currentPlayerId && linkConnection->canRead(remotePlayerId)) {
                u16 msg = linkConnection->read(remotePlayerId);
                // Any ready event means we're ready too
                if (msg == PLAYER_READY) {
                    if (connected < 1) {
                        refreshText = true;
                        clearText();
                    }
                    connected = 1;
                // Messages from player 0 (1st) can start game and set seed
                } else if (remotePlayerId == 0) {
                    if (msg > 100)
                        nextSeed = msg - 100;
                    else if (msg == GAME_START) {
                        // For clients (e.g. not player 1) to start games
                        startMultiplayerGame(nextSeed);
                        gameLoop();
                        return;
                    }
                }
            }

        // At this point we're set up and processed incoming messages, so we wait for the players to be ready
        if (currentPlayerId != 0) {
            aprint("Waiting", 12, 13);
            aprint("for host...", 12, 14);
        } else {
            // Inform new players we're all ready and what the seed is
            if (lastPlayerCount != currentPlayerCount) {
                lastPlayerCount = currentPlayerCount;
                linkConnection->send(PLAYER_READY);
            }
            aprint("Press Start", 10, 13);
        }

        // Await start signal and otherwise re-transmit ready signal
        if (key_hit(k.pause) && currentPlayerId == 0) {
            nextSeed = (u16)qran() & 0x1fff;
            multiplayerStart = true;
            linkConnection->send(nextSeed + 100);
        } else if (!sentReady) {
            linkConnection->send(PLAYER_READY);
            sentReady = true;
        }

        aprint("             ", 0, 19);
    } else {
        if (connected > -1) {
            refreshText = true;
            clearText();
        }
        connected = -1;
        sentReady = false;
    }

    #endif
}

void MultBattleScene::deinit(){
    PlayOptionScene::deinit();
}

void PlayOptionScene::resetScoreboard(){
    vsync();

    resetSmallText();
    clearText();

    clearSprites(128);
    showSprites(128);

    int timer = 0;

    int s = 1;

    naprint("Are you sure you", 6 * 8, 6 * 8);
    naprint("want to reset the", 6 * 8, 8 * 8);
    naprint("current scoreboard?", 6 * 8, 10 * 8);

    bool reset = false;

    MenuKeys k = savefile->settings.menuKeys;

    while (closed()) {
        vsync();
        key_poll();

        fallingBlocks();

        if(timer < 200){
            timer++;
        }else{
            aprint(" YES      NO ", 8, 15);

            int cursorX = 0;
            int length = 0;
            if(s == 0){
                cursorX = 11 * 8 - 2;
                length = 3 * 8;
            }else{
                cursorX = 19 * 8 + 2;
                length = 2 * 8;
            }

            int offset = (sinLut(cursorFloat) * 2) >> 12;
            FIXED scale = float2fx((1.0 - ((float) 0.1 * offset)));

            for( int i = 0; i < 2; i++){
                sprite_unhide(cursorSprites[i],0);
                sprite_set_attr(cursorSprites[i], ShapeSquare, 0, 7 * 16, 5, 1);
                sprite_enable_affine(cursorSprites[i], i, true);
                sprite_set_size(cursorSprites[i], scale , i);

                int x = cursorX - ((length + 8) / 2 + offset + 4) * ((i)?-1:1) - 10;

                sprite_set_pos(cursorSprites[i], x, 15 * 8 - 5);
            }

            if (key_hit(k.confirm)) {
                clearText();
                clearSprites(128);
                sfx(SFX_MENUCONFIRM);
                if(s == 0){
                    reset = true;
                    break;
                } else{
                    break;
                }
            }

            if (key_hit(k.cancel)) {
                clearText();
                sfx(SFX_MENUCANCEL);
                break;
            }

            if(key_hit(k.left)){
                if(s > 0){
                    s--;
                    sfx(SFX_MENUMOVE);
                }
            }

            if(key_hit(k.right)){
                if(s < 1){
                    s++;
                    sfx(SFX_MENUMOVE);
                }
            }
        }

        showSprites(128);

        cursorFloat += 6;
        if(cursorFloat >= 512)
            cursorFloat = 0;
    }

    if(reset){
        if(getMode() != BlockEngine::ZEN){
            EntryBoard * board = getBoard(subMode, goal);

            memset32(board, 0, sizeof(EntryBoard) / 4);
        }else{
            savefile->boards.zen = 0;
        }

        saveSavefile();
    }

    showPath();
}
