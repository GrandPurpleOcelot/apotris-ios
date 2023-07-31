#include "scene.hpp"
#include "sprites.h"
#include "logging.h"
#include "sceneModes.hpp"
#include <cstring>

void PlayOptionScene::init(){
    OptionListScene::init();

    if(elementList.empty())
        onScore = true;
}

bool PlayOptionScene::control(){

    if(key_hit(KEY_B)){
        sfx(SFX_MENUCANCEL);
        if(!onScore){
            previousElement = name();
            previousSelection = selection;

            if(path.size())
                path.pop_back();

            changeScene(previousScene(),Transitions::FADE);
            return true;
        }else{
            onScore = false;
            refreshText = true;
            movingDirection = -1;
            listStart = 0;
        }

    }

    if(key_hit(KEY_LEFT) && !elementList.empty()){
        auto it = elementList.begin();
        std::advance(it,selection);

        (*it)->action(-1);

        refreshOption = true;
    }

    if(key_hit(KEY_RIGHT) && !elementList.empty()){

        auto it = elementList.begin();
        std::advance(it,selection);

        (*it)->action(1);

        refreshOption = true;
    }

    if(key_hit(KEY_A)){
        if(onScore){
            start();
        }
    }

    int prev = listStart;

    if (key_hit(KEY_UP)) {
        movingDirection = -1;

        if(onScore && !elementList.empty()) {
            onScore = false;
            refreshText = true;
        }else{
            if (selection > 0)
                selection--;
        }

        if(selection == options -1)
            listStart = max(0,selection-elementMax);
        else if(selection < listStart + 1){
            listStart--;
            if(listStart < 0)
                listStart = 0;
        }

        sfx(SFX_MENUMOVE);
        refreshText = true;
    }

    if (key_hit(KEY_DOWN)) {
        if(!onScore){
            movingDirection = 1;

            if (selection == options - 1){
                if(getBoard(subMode, goal) != nullptr)
                    movingScore = true;
            } else
                selection++;

            if(selection == 0)
                listStart = 0;
            else if(selection > listStart + elementMax)
                listStart++;
        }

        sfx(SFX_MENUMOVE);
        refreshText = true;
    }

    if (key_is_down(KEY_UP) || key_is_down(KEY_DOWN)) {
        if (dasVer < maxDas) {
            dasVer++;
        } else {
            int dir = (key_is_down(KEY_UP)? -1 : 1);

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

    if (key_is_down(KEY_LEFT) || key_is_down(KEY_RIGHT)) {
        if (dasHor < maxDas) {
            dasHor++;
        } else{
            int dir = (key_is_down(KEY_LEFT)? -1 : 1);
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

    if(key_hit(KEY_START)){
        if(!onScore && getBoard(subMode, goal) != nullptr){
            movingScore = true;
            sfx(SFX_MENUCONFIRM);
        } else{
            start();
        }
    }

    if(key_hit(KEY_SELECT)){
        savefile->quickStart = quick();
        saveSavefile();
        sfx(SFX_SECRET);
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
        }else if(s == "Height"){
            bTypeHeight = value;
        }else if(s == "Finesse Training"){
            training = value;
        }else if(s == "Difficulty"){
            if(name() == "CPU Battle"){
                botThinkingSpeed = value*3;
                botSleepDuration = 5-value*2;
            }else{
                goal = value;
            }
        }else if(s == "Minutes"){
            goal = value;
        }
    }

    BlockEngine::Options newOptions;

    newOptions.mode = getMode();
    newOptions.goal = goal;
    newOptions.level = level;
    newOptions.tuning = getTuning();
    newOptions.bigMode = bigMode;
    newOptions.subMode = subMode;
    newOptions.trainingMode = training;
    newOptions.bTypeHeight = bTypeHeight;
    savefile->stats.gamesStarted++;

    proMode = (key_is_down(KEY_L) || key_is_down(KEY_R));

    if(newOptions.mode != BlockEngine::BATTLE){
        startGame(newOptions,qran());
    }else{
        int seed = qran();

        startGame(newOptions,seed);
        startBotGame(seed);
    }

    gameLoop();
}

QuickStart PlayOptionScene::quick(){
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
        }else if(s == "Minutes"){
            goal = value;
        }
    }

    QuickStart q;

    q.mode = getMode();
    q.submode = subMode;
    q.goal = goal;
    q.level = level;
    q.big = bigMode;

    return q;
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
                currentOption = option->getCurrentOption();
                currentElement = option->getLabel();
                for(int i = 0; i < 2; i++){
                    sprite_unhide(cursorSprites[i],0);
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
    }else{
        EntryBoard *board = getBoard(subMode, goal);

        if(board != nullptr){
            for(int i = 0; i < 5; i++){
                const int y = startY + 8 * (1) * i + verticalOffset;

                char buff[12];
                posprintf(buff, "%d. %s",i+1,board->entries[i].name);

                labels[i]->setText(buff);
                labels[i]->show(startX,y,15);

                std::string str = "";
                if(getIfGrade()){
                    str += GameInfo::masterGrades[board->entries[i].grade];
                    str += " ";
                }

                if(!getIfTime()){
                    posprintf(buff, "%d",board->entries[i].value);
                    str += buff;
                } else
                    str += timeToString(board->entries[i].value);

                const int x = (endX) * 8 - getVariableWidth(str);
                wordSprites[i]->setText(str);
                wordSprites[i]->show(x,y,15);
            }
        }

        wordSprites[5]->setText("START");
        wordSprites[5]->show((240 - 43) /2, 132,15);

        for(int i = 0; i < 2; i++){
            sprite_unhide(cursorSprites[i],0);
            sprite_enable_affine(cursorSprites[i], i, true);
            sprite_set_size(cursorSprites[i], scale , i);

            int x = 240/2 - ((43 + 8) / 2 + offset + 4) * ((i)?-1:1) - 10;

            sprite_set_pos(cursorSprites[i], x, 132 - 5);
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

    if(listStart > 0){
        sprite_unhide(arrowSprites[0], 0);
        sprite_set_pos(arrowSprites[0], 240 / 2 - 8, startY - 11 - 16);
    }else{
        sprite_hide(arrowSprites[0]);
    }

    if(!onScore && getBoard(subMode, goal) != nullptr){
        sprite_unhide(arrowSprites[1], 0);
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

                endDelay = 0;
                scrollOffset = - max((240 - scrollTextLength),0) / 2;
                scrollDelay = 0;
                scrollingText[0]->setTextSlow(scrollText);
            }else{
                scrollingText[0]->setTextSlow(scrollText);
            }
        }
    }

    showSprites(128);

    refreshText = false;
    refreshOption = false;
}

void PlayOptionScene::update(){
    OptionListScene::update();

    if(movingScore > 0){
        if(++movingScoreTimer >= movingScoreTimerMax){
            if(++listStart >= options - 1){
                movingScore = 0;
                onScore = true;
                clearSpriteTiles(672, 144, 1);
                scrollingText[0]->setText("");
                scrollText = "";
                selection = elementList.size() - 1;
            }
        }
    }
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
