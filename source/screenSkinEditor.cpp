#include "def.h"
#include "logging.h"
#include "menu.h"
#include "platform.hpp"
#include "text.h"
#include "posprintf.h"

#include "sprite29tiles_bin.h"
#include "sprite30tiles_bin.h"
#include "sprite31tiles_bin.h"
#include "sprite32tiles_bin.h"
#include "sprite33tiles_bin.h"
#include "sprite34tiles_bin.h"
#include "sprite35tiles_bin.h"

#include <string>
#include <tuple>
#include <algorithm>

// #include "soundbank.h"
#include "tetrisEngine.h"
#include "scene.hpp"

const int xoffset = 15-8;
const int yoffset = 10-8;

class Cursor{
public:
    int x = 0;
    int y = 0;
    OBJ_ATTR* sprite;

    void show(bool mini){
        int sx = (x+mini) * 16 + xoffset * 8;
        int sy = (y+mini) * 16 + yoffset * 8;

        sprite_set_pos(sprite,sx,sy);
        sprite_unhide(sprite,0);
    }

    bool move(int dx, int dy,bool mini){
        bool sound = false;
        bool moved = false;

        if(dx){
            if(x+dx >= 0 && x+dx <= (7-(mini*2))){
                x += dx;
                sfx(SFX_SHIFT2);
                moved = true;
                sound = true;
            }
        }
        if(dy){
            if(y+dy >= 0 && y+dy <= 7-(mini*2)){
                y += dy;
                moved = true;
                if(!sound)
                    sfx(SFX_SHIFT2);
            }
        }

        return moved;
    }

    Cursor(){
        sprite = &obj_buffer[0];
        sprite_set_attr(sprite, ShapeSquare, 1, 712, 15, 0);

        // obj_set_attr(sprite,ATTR0_SQUARE,ATTR1_SIZE(1),ATTR2_BUILD(712, 15, 0));
    }
};

class Move{
    public:
        int x = 0;
        int y = 0;

        int previousValue = 0;
        int currentValue = 0;
        int tool = 0;

        Move(int _x, int _y, int previous, int current, int _tool){
            x = _x;
            y = _y;

            previousValue = previous;
            currentValue = current;

            tool = _tool;
        }
};

class Board{
    public:
        int board[8][8];
        Cursor cursor;

        std::list<Move> history;

        std::list<std::tuple<int,int>> queue;

        void set(int value, bool record){
            if(record)
                history.push_back(Move(cursor.x,cursor.y,board[cursor.y][cursor.x],value,1));

            if(history.size() > 10)
                history.pop_front();

            board[cursor.y][cursor.x] = value;
            int s = (cursor.x) * 4;
            customSkin->data[cursor.y] = (customSkin->data[cursor.y] & ~(0xf << (s))) | ((value + (value == 5)) << (s));
        }

        void fill(int value, int record){
            int previous = board[cursor.y][cursor.x];
            if(value == previous)
                return;
            if(record)
                history.push_back(Move(cursor.x,cursor.y,previous,value,2));

            int x,y;

            queue.push_back(std::make_tuple(cursor.x,cursor.y));

            do{
                std::tuple<int,int> coords = queue.front();
                x = std::get<0>(coords);
                y = std::get<1>(coords);

                board[y][x] = value;
                int s = (x) * 4;
                customSkin->data[y] = (customSkin->data[y] & ~(0xf << (s))) | ((value + (value == 5)) << (s));

                if(y - 1 >= 0 && board[y-1][x] == previous){
                    auto n = std::make_tuple(x,y-1);
                    if(std::find(queue.begin(),queue.end(),n) == queue.end())
                        queue.push_back(n);
                }
                if(y + 1 <= 7 && board[y+1][x] == previous){
                    auto n = std::make_tuple(x,y+1);
                    if(std::find(queue.begin(),queue.end(),n) == queue.end())
                        queue.push_back(n);
                }
                if(x - 1 >= 0 && board[y][x-1] == previous){
                    auto n = std::make_tuple(x-1,y);
                    if(std::find(queue.begin(),queue.end(),n) == queue.end())
                        queue.push_back(n);
                }
                if(x + 1 <= 7 && board[y][x+1] == previous){
                    auto n = std::make_tuple(x+1,y);
                    if(std::find(queue.begin(),queue.end(),n) == queue.end())
                        queue.push_back(n);
                }

                queue.pop_front();
            }while(!queue.empty());

        }

        void clear(){
            for(int i = 0; i < 8; i++){
                for(int j = 0; j < 8; j++)
                    board[i][j] = 0;

                customSkin->data[i] = 0;
            }
        }

        void show(bool mini){
            // u16* dest = (u16*) &se_mem[25];

            for(int i = 0; i < 8; i++){
                int y = ((i * 2)+yoffset);
                for(int j = 0; j < 8; j++){
                    int x = j * 2 + xoffset;
                    if(!mini){
                        if(board[i][j] != 0){
                            int n = board[i][j] + 100 - (board[i][j] == 6);
                            setTile(25, x, y, tileBuild(n, false, false, 0));
                            setTile(25, x+1, y, tileBuild(n, false, false, 0));
                            setTile(25, x, y+1, tileBuild(n, false, false, 0));
                            setTile(25, x+1, y+1, tileBuild(n, false, false, 0));
                            // dest[y+x] = n;
                            // dest[y+x+1] = n;
                            // dest[y+32+x] = n;
                            // dest[y+32+x+1] = n;
                        }else{
                            setTile(25, x, y, tileBuild(0x6a, false, false, 15));
                            setTile(25, x+1, y, tileBuild(0x6b, false, false, 15));
                            setTile(25, x, y+1, tileBuild(0x6c, false, false, 15));
                            setTile(25, x+1, y+1, tileBuild(0x6c, false, false, 15));
                            // dest[y+x] = 0xf06a;
                            // dest[y+x+1] = 0xf06b;
                            // dest[y+32+x] = 0xf06c;
                            // dest[y+32+x+1] = 0xf06d;
                        }
                    }else{
                        if(i == 0 || j == 0 || i > 6 || j > 6){
                            setTile(25, x, y, 0);
                            setTile(25, x+1, y, 0);
                            setTile(25, x, y+1, 0);
                            setTile(25, x+1, y+1, 0);
                            // dest[y+x] = 0;
                            // dest[y+x+1] = 0;
                            // dest[y+32+x] = 0;
                            // dest[y+32+x+1] = 0;
                        }else if(board[i-1][j-1] != 0){
                            int n = board[i-1][j-1] + 100 - (board[i-1][j-1] == 6);
                            setTile(25, x, y, tileBuild(n, false, false, 0));
                            setTile(25, x+1, y, tileBuild(n, false, false, 0));
                            setTile(25, x, y+1, tileBuild(n, false, false, 0));
                            setTile(25, x+1, y+1, tileBuild(n, false, false, 0));
                            // dest[y+x] = n;
                            // dest[y+x+1] = n;
                            // dest[y+32+x] = n;
                            // dest[y+32+x+1] = n;
                        }else{
                            setTile(25, x, y, tileBuild(0x6a, false, false, 15));
                            setTile(25, x+1, y, tileBuild(0x6b, false, false, 15));
                            setTile(25, x, y+1, tileBuild(0x6c, false, false, 15));
                            setTile(25, x+1, y+1, tileBuild(0x6c, false, false, 15));
                            // dest[y+x] = 0xf06a;
                            // dest[y+x+1] = 0xf06b;
                            // dest[y+32+x] = 0xf06c;
                            // dest[y+32+x+1] = 0xf06d;
                        }
                    }
                }
            }

            cursor.show(mini);
        }

        bool undo(){
            if(history.empty())
               return false;

            Move previous = history.back();

            cursor.x = previous.x;
            cursor.y = previous.y;


            switch(previous.tool){
            case 1: set(previous.previousValue,false); break;
            case 2: fill(previous.previousValue,false); break;
            }

            history.pop_back();

            return true;
        }

        void setBoard(TILE * t,int type){
            if(type == 0){
                for(int i = 0; i < 8; i++){
                    for(int j = 0; j < 8; j++){
                        board[i][j] = (t->data[i] >> (j * 4)) & 0xf;
                    }
                }
            }else{
                for(int i = 0; i < 8; i++){
                    for(int j = 0; j < 8; j++){
                        if(i < 6 && j < 6)
                            board[i][j] = (t->data[i] >> ((j) * 4)) & 0xf;
                        else
                            board[i][j] = 0;
                    }
                }
            }
        }

        Board(){
            for(int i = 0; i < 8; i++)
                for(int j = 0; j < 8; j++)
                    board[i][j] = 0;
        }
};

void generateTiles();
void showColorPalette(int);
void showMinos();
void refreshSkin();
void showTools(int);
void showCursor();
void showMini();
int selector();
void helpScreen();
int optionsMenu();

TILE * customSkin;
Board board;

static int cursorTimer = 0;
static int toolTimer = 0;

static int skinIndex = 0;
static bool onMini = false;
static bool refresh = false;

void EditorScene::init(){
    onMini = false;
    gradient(false);
    setPaletteColor(0, 0, 0, 1);
    clearTilemap(27);
    buildBG(3, 0, 27, 0, 2, 0);

    setTiles(26, 0, 32*32, 2 * !(savefile->settings.lightMode));
    // for(int i = 0; i < 32; i++)
    //     for(int j = 0; j < 20; j++)
    //         setTile(26,i,j,);

    if(game != nullptr && game->gameMode == Tetris::CLASSIC){
        game->gameMode = Tetris::SPRINT;
        setSkin();
    }
}

void EditorScene::update(){
    loadSpriteTiles(512 + 216, sprite35tiles_bin, 2, 2);
    setPalette();

    if(selector()){
        previousSelection = 5;
        changeScene(new MainMenuScene,Transitions::FADE);
        return;
    }

    onMini = false;

    toggleBG(3, true);
    clearText();

    loadTiles(0, 106, sprite34tiles_bin, 4);

    loadSpriteTiles(512 + 200, sprite29tiles_bin, 2,2);
    loadSpriteTiles(512 + 204, sprite31tiles_bin, 2,2);
    loadSpriteTiles(512 + 208, sprite32tiles_bin, 2,2);
    loadSpriteTiles(512 + 212, sprite33tiles_bin, 2,2);

    generateTiles();

    for(int i = 0; i < 8; i++)
        customSkin->data[i] = savefile->customSkins[skinIndex].board.data[i];

    board.setBoard(customSkin,0);
    refreshSkin();

    setSmallTextArea(110, 0, 10, 10, 20);
    clearText();
    aprints("Press SELECT",2,64,2);
    aprints("for help",2,72,2);

    while(closed()){
        canDraw = true;
        vsync();

        if(control()){
            break;
        }

        board.show(onMini);
        showMinos();
        showCursor();
        // showMini();

        showColorPalette(currentColor);
        showTools(currentTool);

        showSprites(32);
    }

    refreshSkin();

    clearSpriteTiles(712, 16, 1);
    clearSprites(128);
    showSprites(128);
    clearTilemap(25);

    clearText();
    toggleBG(3, false);

    delete customSkin;
}

void EditorScene::deinit(){
    //clean up
    clearSpriteTiles(712, 16, 1);
    clearSprites(128);
    showSprites(128);
    clearTilemap(25);
    clearTilemap(26);
    clearTilemap(27);

    gradient(true);
    drawUIFrame(0, 0, 30, 20);
    buildBG(3, 0, 27, 0, 3, 0);
    setLayerScroll(3, 0, 0);

    clearTiles(2,110,400);
}

bool EditorScene::control(){
    key_poll();

    int dx = 0;
    int dy = 0;

    if(key_is_down(KEY_R)){
        if(key_hit(KEY_LEFT)){
            currentColor--;
            if(currentColor < 0)
                currentColor = 5;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_RIGHT)){
            currentColor++;
            if(currentColor > 5)
                currentColor = 0;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_UP)){
            currentTool--;
            if(currentTool < 0)
                currentTool = 2;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_DOWN)){
            currentTool++;
            if(currentTool > 2)
                currentTool = 0;
            sfx(SFX_MENUMOVE);
        }
    }else if(key_is_down(KEY_L)){
        if(key_hit(KEY_UP)){
            savefile->settings.colors--;
            if(savefile->settings.colors < 0)
                savefile->settings.colors = MAX_COLORS-1;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_DOWN)){
            savefile->settings.colors++;
            if(savefile->settings.colors > MAX_COLORS-1)
                savefile->settings.colors = 0;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_UP) || key_hit(KEY_DOWN))
            setPalette();

        if(key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)){
            refreshSkin();
            onMini = !onMini;

            if(onMini){
                for(int i = 0; i < 8; i++)
                    customSkin->data[i] = savefile->customSkins[skinIndex].smallBoard.data[i];

                if(board.cursor.x > 0)
                    board.cursor.x--;
                if(board.cursor.x > 5)
                    board.cursor.x = 5;

                if(board.cursor.y > 0)
                    board.cursor.y--;
                if(board.cursor.y > 5)
                    board.cursor.y = 5;
            }else{
                for(int i = 0; i < 8; i++)
                    customSkin->data[i] = savefile->customSkins[skinIndex].board.data[i];
                board.cursor.x++;
                board.cursor.y++;
            }

            board.setBoard(customSkin,onMini);
        }

    }else{
        if(key_hit(KEY_LEFT | KEY_RIGHT)){
            dx = ((key_hit(KEY_RIGHT) != 0) - (key_hit(KEY_LEFT) != 0));
        }

        if(key_hit(KEY_UP | KEY_DOWN)){
            dy = ((key_hit(KEY_DOWN) != 0) - (key_hit(KEY_UP) != 0));
        }

        if(key_is_down(KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN))
            das++;
        else
            das = 0;

        if(das >= dasMax){

            if(key_is_down(KEY_LEFT))
                dx = -1;
            else if(key_is_down(KEY_RIGHT))
                dx = 1;

            if(key_is_down(KEY_UP))
                dy = -1;
            else if(key_is_down(KEY_DOWN))
                dy = 1;

            das -= arr;
        }

        bool moved = board.cursor.move(dx,dy,onMini);
        if((dx || dy) && key_is_down(KEY_A)){
            switch(currentTool){
            case 0: board.set(0,true); break;
            case 1: board.set(currentColor,true); break;
            }
            if(moved)
                sfx(SFX_PLACE);
            refreshSkin();
        }
    }

    if(key_hit(KEY_A)){
        switch(currentTool){
        case 0: board.set(0,true); break;
        case 1: board.set(currentColor,true); break;
        case 2: board.fill(currentColor,true); break;
        }

        sfx(SFX_PLACE);
        refreshSkin();
    }

    if(key_hit(KEY_B)){
        if(board.undo())
            sfx(SFX_ROTATE);
        else
            sfx(SFX_MENUCANCEL);

        refreshSkin();
    }

    if(key_hit(KEY_START)){
        sfx(SFX_MENUCONFIRM);
        return true;
    }

    if(key_hit(KEY_SELECT)){
        helpScreen();
    }

    if(key_is_down(KEY_L) && key_is_down(KEY_R)){
        if(++timer > 50){
            board.clear();
            refreshSkin();
            timer = 0;
        }
    }else{
        timer = 0;
    }

    return false;
}

void skinEditor(){
    EditorScene * s = new EditorScene();
    changeScene(s);

    while(closed()){
    }

    // memset32(&tile_mem[2][110],0,8 * 400);
    TitleScene* newScene = new TitleScene();
    changeScene(newScene);
}

int selector(){
    int das = 0;
    const int arr = 4;
    const int dasMax = 20;

    int selection = 0;
    const int options = 2;

    const int slotHeight = 9;

    aprint("Select a slot to edit:",4,6);

    for(int i = 0; i < 5; i++){
        OBJ_ATTR * sprite = &obj_buffer[i+1];

        loadSpriteTiles(512 + 200 + i, &savefile->customSkins[i].board, 1, 1);
        // memcpy16(&tile_mem[5][200+i],&savefile->customSkins[i].board,16);

        sprite_set_attr(sprite, ShapeSquare, 0, 712+i, 5, 0);
        // obj_set_attr(sprite, ATTR0_SQUARE, ATTR1_SIZE(0), ATTR2_BUILD(712+i, 5, 0));
        sprite_set_pos(sprite,(4+i*5)*8 + 4,(slotHeight * 8) + 12);
        sprite_unhide(sprite,0);
    }

    showSprites(32);
    // oam_copy(oam_mem, obj_buffer, 32);

    while(closed()){
        vsync();
        // VBlankIntrWait();
        key_poll();

        aprintColor(" C1   C2   C3   C4   C5 ",3, slotHeight, 1);
        aprint("C" + std::to_string(skinIndex+1),4 + skinIndex*5,slotHeight);

        aprint(" BACK ",12,16);

        if(selection == 0){
            int x = 3 + skinIndex*5;
            aprint("[",x,slotHeight);
            aprint("]",x+3,slotHeight);

        }else if(selection == options-1){
            aprint("[",12,16);
            aprint("]",17,16);
        }


        if(key_hit(KEY_A) || key_hit(KEY_START)){
            if(selection == 0){
                sfx(SFX_MENUCONFIRM);
                break;
            }else if(selection == 1){
                sfx(SFX_MENUCANCEL);
                return 1;
            }
        }

        if(key_hit(KEY_UP)){
            selection--;
            if(selection < 0)
                selection = options-1;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_DOWN) || key_hit(KEY_SELECT)){
            selection++;
            if(selection > options-1)
                selection = 0;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_LEFT) && skinIndex > 0 && !selection){
            skinIndex--;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_RIGHT) && skinIndex < 4 && !selection){
            skinIndex++;
            sfx(SFX_MENUMOVE);
        }

        if(key_is_down(KEY_LEFT) || key_is_down(KEY_RIGHT)){
            das++;

            if(das == dasMax){
                das-=arr;

                if(key_is_down(KEY_LEFT) && skinIndex > 0 && !selection){
                    skinIndex--;
                    sfx(SFX_MENUMOVE);
                }else if(key_is_down(KEY_RIGHT) && skinIndex < 4 && !selection){
                    skinIndex++;
                    sfx(SFX_MENUMOVE);
                }
            }
        }else {
            das = 0;
        }

        if(key_hit(KEY_B)){
            sfx(SFX_MENUCANCEL);
            return 1;
        }
    }

    return 0;
}

void generateTiles(){

    TILE *t = new TILE();

    for(int i = 0; i < 6; i++){
        int n = 0;
        for(int j = 0; j < 8; j++)
            n += (i + (i == 5)) << (j*4);

        for(int j = 0; j < 8; j++)
            t->data[j] = n;

        loadTiles(0, 100+i, t, 1);
        // memcpy16(&tile_mem[0][100+i],t,16);
    }

    delete t;

    customSkin = new TILE();
    for (int j = 0; j < 8; j++) {
        customSkin->data[j] = 0;
    }
}

void showColorPalette(int c){
    // u16* dest = (u16*) &se_mem[27];

    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 8; j++){
            if(i == 1){
                if(j > 0 && j < 7)
                    setTile(27, j, i, tileBuild(100+j-1, false, false, 0));
                    // *dest++ = 100 + j - 1;
                else
                    setTile(27, j, i, tileBuild(27, (j==7), false, 6));
                    // *dest++ = 27 + 0x400 * (j == 7) + 0x6000;
            }else{
                if(j > 0 && j < 7)
                    setTile(27, j, i, tileBuild(28, false, (i==2), 6));
                    // *dest++ = 28 + 0x800 * (i == 2) + 0x6000;
                else
                    setTile(27, j, i, tileBuild(29, (j==7), (i==2), 6));
                    // *dest++ = 29 + 0x400 * (j == 7) + 0x800 * (i == 2) + 0x6000;

            }
        }

        // dest+=32-8;
    }

    aprint("      ",0,1);

    setLayerScroll(3, 4, 4);
    // REG_BG3VOFS = 4;
    // REG_BG3HOFS = 4;

    OBJ_ATTR * sprite = &obj_buffer[11];

    sprite_set_attr(sprite, ShapeSquare, 1, 512 + 216, 15, 0);
    // obj_set_attr(sprite,ATTR0_SQUARE,ATTR1_SIZE(1),ATTR2_BUILD(512 + 216, 15, 0));
    sprite_set_pos(sprite,3+c*8,3);
    sprite_unhide(sprite,0);
}

void showTools(int c){
    OBJ_ATTR * sprites[3];

    for(int i = 0; i < 3; i++){
        sprites[i] = &obj_buffer[i+8];
        sprite_set_attr(sprites[i], ShapeSquare, 1, 716 + i * 4, 15, 2);
        // obj_set_attr(sprites[i], ATTR0_SQUARE, ATTR1_SIZE(1), ATTR2_BUILD(716 + i * 4, 15, 2));

        int offset = 0;

        if(i == c){
            aprint("<",3,4+i*2);

            offset = ((sinLut(toolTimer)*2)>>12);

        } else
            aprint(" ",3,4+i*2);

        sprite_set_pos(sprites[i],4,32+(i*16) - 4 + offset);
        sprite_unhide(sprites[i],0);
    }

    toolTimer+=6;
    if(toolTimer >= 512)
        toolTimer = 0;

}

void showMinos(){

    for(int i = 0; i < 7; i++){
        OBJ_ATTR *sprite = &obj_buffer[i+1];

        if(!onMini){
            sprite_set_attr(sprite, ShapeSquare, 2, 16 * i, i, 2);
            // obj_set_attr(sprite, ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_BUILD(16 * i, i, 2));
            sprite_unhide(sprite,0);
        }else{
            sprite_unhide(sprite, 0);
            sprite_set_attr(sprite, ShapeWide, 2, 9 * 16 + 8 * i, i, 3);
            // obj_set_attr(sprite, ATTR0_WIDE, ATTR1_SIZE(2), ATTR2_BUILD(9 * 16 + 8 * i, i, 3));
            // obj_set_pos(sprite, 192, i * 24 + 4);
        }
        sprite_set_pos(sprite,208 - (4-onMini) * (i == 0 || i == 3),i * 24 - 4);
    }
}

void refreshSkin(){
    if(!onMini){
        savefile->settings.skin = -(skinIndex+1);
        for(int i = 0; i < 8; i++){
            savefile->customSkins[skinIndex].board.data[i] = customSkin->data[i];
        }
        setSkin();
    }else{
        for(int i = 0; i < 8; i++){
            savefile->customSkins[skinIndex].smallBoard.data[i] = customSkin->data[i];
        }

        refresh = true;
    }
}

void showCursor(){
    cursorTimer++;

    const int animationLength = 32;

    if(cursorTimer < animationLength/2){
        loadSpriteTiles(512+200, sprite29tiles_bin, 2, 2);
        // memcpy16(&tile_mem[5][200],sprite29tiles_bin,sprite29tiles_bin_size/2);
    }else{
        loadSpriteTiles(512+200, sprite30tiles_bin, 2, 2);
        // memcpy16(&tile_mem[5][200],sprite30tiles_bin,sprite30tiles_bin_size/2);
        if(cursorTimer >= animationLength-1)
            cursorTimer = 0 ;
    }
}

void showMini(){
    for(int i = 0; i < 7; i++){
        OBJ_ATTR * sprite = &obj_buffer[12+i];

        // int n = savefile->customSkins[skinIndex].previewStyle;

        sprite_unhide(sprite, 0);
        sprite_set_attr(sprite, ShapeWide, 2, 9*16 + 8*i, i, 3);
        // sprite_set_attr(sprite, ATTR0_WIDE, ATTR1_SIZE(2), ATTR2_BUILD(9 * 16 + 8 * i, i, 3));
        sprite_set_pos(sprite, 192, i * 24 + 4);
    }
}

void helpScreen(){
    sfx(SFX_MENUCONFIRM);
    setSmallTextArea(110, 8, 1, 24, 21);
    clearText();

    const int startX = 2;
    const int startY = 1;
    u8 count = 0;

    aprints("-Move using the D-Pad",startX, (startY+count)*4,2);
    count+= 3;

    aprints("-Use tools by pressing A",startX, (startY+count)*4,2);
    count+= 3;

    aprints("-Undo by pressing B",startX, (startY+count)*4,2);
    count+= 3;

    aprints("-Change tools by holding R",startX, (startY+count)*4,2);
    aprints(" and pressing Up/Down",startX, (startY+count+2)*4,2);
    count+= 5;

    aprints("-Change colors by holding R",startX, (startY+count)*4,2);
    aprints(" and pressing Left/Right",startX, (startY+count+2)*4,2);
    count+= 5;

    aprints("-Cycle palettes by holding L",startX, (startY+count)*4,2);
    aprints(" and pressing Up/Down",startX, (startY+count+2)*4,2);
    count+= 5;

    aprints("-Edit preview/hold by holding L",startX, (startY+count)*4,2);
    aprints(" and pressing Left/Right",startX, (startY+count+2)*4,2);
    count+= 5;

    aprints("-Clear the canvas by",startX, (startY+count)*4,2);
    aprints(" holding L and R",startX, (startY+count+2)*4,2);
    count+= 5;

    aprints("-Exit by pressing Start",startX, (startY+count)*4,2);
    count+= 3;

    clearSprites(128);
    showSprites(128);

    toggleBG(0, false);
    toggleBG(3, false);

    // oam_init(obj_mem, 128);

    // REG_DISPCNT &= ~DCNT_BG0;
    // REG_DISPCNT &= ~DCNT_BG3;

    while(true){
        vsync();
        key_poll();

        if(key_hit(KEY_FULL))
            break;
    }

    toggleBG(0, true);
    toggleBG(3, true);
    // REG_DISPCNT |= DCNT_BG0;
    // REG_DISPCNT |= DCNT_BG3;
    setSmallTextArea(110, 0, 10, 10, 20);
    clearText();
    aprints("Press SELECT",2,64,2);
    aprints("for help",2,72,2);
    sfx(SFX_MENUCANCEL);
}

void EditorScene::draw(){
    if(refresh){
        refresh = false;
        buildMini(customSkin);
    }
}
