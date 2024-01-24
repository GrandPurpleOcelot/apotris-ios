#include "platform.hpp"

#ifdef GBA
#include "logging.h"
#include "LinkUniversal.hpp"
#include "maxmod.h"
#include "rumble.h"
#include "gbp_logo.hpp"
#include "text.h"
#include "flashSaves.h"
#include "scene.hpp"
#include "gbfs.h"

// #include <string>
// #include "scene.hpp"
// #include "main.hpp"
bool unlock_gbp();
void loadAudio();
void onKeyInterrupt();

int spriteVOffset = 0;

u8 * soundbank = nullptr;

bool sortSprites = false;

Songs songs;

void onHBlank() {
    if(REG_VCOUNT < 160)
        pal_bg_mem[0] = gradientTable[REG_VCOUNT];
}

#define WIRELESS_TIMEOUT_FRAMES 15

LinkUniversal* linkUniversal =
        new LinkUniversal(LinkUniversal::Protocol::AUTODETECT,
                          "Apotris",
                          (LinkUniversal::CableOptions){
                                  .baudRate = LinkCable::BAUD_RATE_1,
                                  .timeout = LINK_WIRELESS_DEFAULT_TIMEOUT,
                                  .remoteTimeout = LINK_WIRELESS_DEFAULT_REMOTE_TIMEOUT,
                                  .interval = 250,
                                  .sendTimerId = LINK_CABLE_DEFAULT_SEND_TIMER_ID},
                          (LinkUniversal::WirelessOptions){
                                  .retransmission = true,
                                  .maxPlayers = LINK_UNIVERSAL_MAX_PLAYERS,
                                  .timeout = WIRELESS_TIMEOUT_FRAMES,
                                  .remoteTimeout = WIRELESS_TIMEOUT_FRAMES,
                                  .interval = LINK_WIRELESS_DEFAULT_INTERVAL * 2,
                                  .sendTimerId = LINK_WIRELESS_DEFAULT_SEND_TIMER_ID,
                                  .asyncACKTimerId = 2});
LinkUniversal* linkConnection = linkUniversal;

OBJ_AFFINE* obj_aff_buffer = (OBJ_AFFINE*)obj_buffer;

mm_word myEventHandler(mm_word msg, mm_word param){
    if(msg == MMCB_SONGMESSAGE)
        return 0;

    playNextSong();

    return 0;
}

void toggleBG(int layer, bool state){
    if(state){
        REG_DISPCNT |= ((1 << layer) << 8);
    }else{
        REG_DISPCNT &= ~((1 << layer) << 8);
    }
}

void buildBG(int layer, int cbb, int sbb, int size, int prio, int mos){

    *(vu16*) (REG_BASE + 0x0008 + 0x0002 * layer) = BG_CBB(cbb) | BG_SBB(sbb) | BG_SIZE(size) | BG_PRIO(prio) | BG_MOSAIC * mos;
}

void toggleSprites(bool state){
    if(state){
        REG_DISPCNT |= DCNT_OBJ;
    }else{
        REG_DISPCNT &= ~DCNT_OBJ;
    }
}

void vsync(){
    VBlankIntrWait();
}

void showSprites(int count){
    if(sortSprites){
        int counters[4] = {0,0,0,0};

        int positions[count];

        for(int i = 0; i < count; i++){
            const int priority = (obj_buffer[i].attr2 >> 0xa) & 0b11;
            positions[i] = counters[priority]++ + (priority << 8);
        }

        int offsets[4] = {0,0,0,0};

        for(int i = 0; i < 4; i++){
            for(int j = 0; j < i; j++){
                offsets[i] += counters[j];
            }
        }

        for(int i = 0; i < count; i++){
            const int priority = positions[i] >> 8;
            const int position = positions[i] & 0xff;
            const int offset = offsets[priority];

            oam_mem[offset + position] = obj_buffer[i];
        }
    }else{
        oam_copy(oam_mem,obj_buffer,count);
    }

    obj_aff_copy(obj_aff_mem, obj_aff_buffer, 32);
}

void platformInit(){

    #ifndef NO_DIAGNOSE
    logInitMgba();
    #endif

    if(ENABLE_FLASH_SAVE)
        flash_init();

    interrupt_init();
    REG_DISPSTAT |= (1 << 3);
    interrupt_enable(INTR_VBLANK);
    interrupt_enable(INTR_SERIAL);
    interrupt_enable(INTR_TIMER3);
    interrupt_enable(INTR_TIMER2);
    REG_KEYCNT = KEY_A | KEY_B | KEY_SELECT | KEY_START | BIT(14) | BIT(15);
    interrupt_set_handler(INTR_KEYPAD, onKeyInterrupt);
    interrupt_enable(INTR_KEYPAD);
    interrupt_set_handler(INTR_VBLANK, onVBlank);

    interrupt_set_handler(INTR_SERIAL, LINK_UNIVERSAL_ISR_SERIAL);
    interrupt_set_handler(INTR_TIMER3, LINK_UNIVERSAL_ISR_TIMER);
    interrupt_set_handler(INTR_TIMER2, LINK_UNIVERSAL_ISR_ACK_TIMER);

    // interrupt_add(INTR_HBLANK, onHBlank);
    interrupt_disable(INTR_HBLANK);

    loadAudio();

    mmInitDefault((mm_addr)soundbank, 12);
    mmSetEventHandler((mm_callback)myEventHandler);

    REG_DISPCNT = DCNT_OBJ_1D | DCNT_MODE0;

    REG_BLDALPHA = BLD_EVA(31) | BLD_EVB(5);

    REG_WAITCNT = 0x4317;
}

void deinitialize(){
}

bool closed(){
    return true;
}

void loadPalette(int palette, int index, const void *src, int count){
    memcpy16(&pal_bg_mem[16*palette + index], src, count);
}

void loadTiles(int tileset, int index, const void *src, int count){
    memcpy32(&tile_mem[tileset][index], src,count*8);
}

void loadTilemap(int tilemap, int index, const void *src, int count){
    u16 * dst = (u16 * ) &se_mem[tilemap];
    memcpy16(&dst[index], src, count);
}

void clearTilemap(int tilemap){
    memset16(&se_mem[tilemap], 0,32*32);
}

void clearSpriteTiles(int index, int lengthX, int lengthY){
    memset32(&tile_mem[4][index],0,lengthX*lengthY*8);
}

void clearSpriteTile(int index, int tx, int ty, int width){
    memset32(&tile_mem[4][index + ty * width + tx],0,8);
}

void loadSpriteTiles(int index, const void *src, int lengthX, int lengthY){
    memcpy32(&tile_mem[4][index], src,lengthX * lengthY * 8);
}

void loadSpriteTilesPartial(int index, const void *src, int tx, int ty, int lengthX, int lengthY, int rowLength){
    memcpy32(&tile_mem[4][index + ty * rowLength + tx], src,lengthX * lengthY * 8);
}

void setPaletteColor(int palette, int index, u16 color, int count){
    memset16(&pal_bg_mem[palette * 16 + index],color,count);
}

void sprite_hide(OBJ_ATTR * sprite){
    obj_hide(sprite);
}

void sprite_unhide(OBJ_ATTR * sprite, int mode){
    obj_unhide(sprite,mode);
}

void sprite_set_pos(OBJ_ATTR * sprite, int x, int y){
    obj_set_pos(sprite,x,y);
}

void sprite_set_attr(OBJ_ATTR* sprite, int shape, int size, int tile_start, int palette, int priority){
    obj_set_attr(sprite, ATTR0_SHAPE(shape), ATTR1_SIZE(size), ATTR2_BUILD(tile_start, palette, priority));
}

void setLayerScroll(int layer, int x, int y){
    switch(layer){
    case 0:
        REG_BG0HOFS = x;
        REG_BG0VOFS = y;
        break;
    case 1:
        REG_BG1HOFS = x;
        REG_BG1VOFS = y;
        break;
    case 2:
        REG_BG2HOFS = x;
        REG_BG2VOFS = y;
        break;
    case 3:
        REG_BG3HOFS = x;
        REG_BG3VOFS = y;
        break;

    }
}

void enableLayerBlend(int layer){
    REG_BLDCNT = (1 << 6) + (1 << 0xb) + (1<<layer);
    REG_BLDALPHA = BLD_EVA(31) | BLD_EVB(15);

    // TLN_SetLayerBlendMode(2,BLEND_ADD,0);
}

void color_fade_palette(int palette, int index, const COLOR *src, COLOR color, int count, u32 alpha){
    clr_fade(src,color,&pal_bg_mem[palette*16+index],count,alpha);
}

void color_fade(COLOR * dst, const COLOR * src, COLOR color, int count, u32 alpha){
    clr_fade(src,color,dst,count,alpha);
}

void color_adj_brightness(int palette, int index, const COLOR * src, u32 count, FIXED alpha){
    clr_adj_brightness(&pal_bg_mem[palette*16+index], src, count, alpha);
}

void color_adj_MEM(COLOR * dst, const COLOR * src, u32 count, u32 alpha){
    clr_adj_brightness(dst, src, count, alpha);
}

void color_blend(COLOR *dst, const COLOR *srca, const COLOR *srcb, int nclrs, u32 alpha) {
    clr_blend(srca,srcb,dst,nclrs,alpha);
}

void addColorToPalette(int palette, int index, COLOR color, int count){
    for(int i = 0; i < count; i++){
        pal_bg_mem[palette * 16 + index + i] += color;
    }
}

void initRumble(){
    if(!savefile->settings.rumble)
        return;

    rumbleInitialized = true;
    RegisterRamReset(RESET_VRAM);
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
    *((volatile u16*)0x4000008) = 0x0088;

    if (unlock_gbp()) {

        RumbleGBPConfig conf{[](void (*rumble_isr)(void)) {
            interrupt_enable(INTR_SERIAL);
            interrupt_set_handler(INTR_SERIAL, rumble_isr);
        }};

        rumble_init(&conf);
    } else {
        rumble_init(nullptr);
    }
}

bool unlock_gbp(){
    bool gbp_detected = false;

    memcpy16((u16*)0x6008000, gbp_logo_pixels, (sizeof gbp_logo_pixels)/2);
    memcpy16((u16*)0x6000000, gbp_logo_tiles, (sizeof gbp_logo_tiles)/2);
    memcpy16(pal_bg_mem, gbp_logo_palette, (sizeof gbp_logo_palette)/2);

    static volatile u32* keys = (volatile u32*)0x04000130;

    for(int i = 0; i < 120; i++){
        // if(key_is_down(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)){
        if(*keys==0x030f){
            gbp_detected = true;
        }
        VBlankIntrWait();
    }

    RegisterRamReset(RESET_VRAM);

    REG_DISPCNT = DCNT_OBJ_1D | DCNT_MODE0;
    REG_BLDALPHA = BLD_EVA(31) | BLD_EVB(5);
    REG_WAITCNT = 0x4317;

    return gbp_detected;
}

void sleep() {
    int display_value = REG_DISPCNT;

    bool prevGradient = gradientEnabled;
    interrupt_disable(INTR_VBLANK);
    gradient(false);

    int stat_value = REG_SNDSTAT;
    int dsc_value = REG_SNDDSCNT;
    int dmg_value = REG_SNDDMGCNT;

    REG_DISPCNT |= 0x0080;
    REG_SNDSTAT = 0;
    REG_SNDDSCNT = 0;
    REG_SNDDMGCNT = 0;

    bool isLinked = linkConnection->isActive();

    if(isLinked)
        linkConnection->deactivate();

    interrupt_disable(INTR_TIMER3);
    interrupt_disable(INTR_TIMER2);
    interrupt_disable(INTR_SERIAL);

    REG_P1CNT = 0b1100001100000100;

    Stop();

    REG_DISPCNT = display_value;
    REG_SNDSTAT = stat_value;
    REG_SNDDSCNT = dsc_value;
    REG_SNDDMGCNT = dmg_value;

    if(isLinked)
        linkConnection->activate();

    interrupt_enable(INTR_TIMER3);
    interrupt_enable(INTR_TIMER2);
    interrupt_enable(INTR_SERIAL);
    interrupt_enable(INTR_VBLANK);

    if(prevGradient)
        gradient(true);
}

void sfx(int s){

    int id = SoundEffectIds[s];

    if(id == -1)
        return;

    mm_sfxhand h = mmEffect(id);
    mmEffectVolume(h, 255 * (float)savefile->settings.sfxVolume / 10);

    if((s == SFX_PLACE) && game->zoneTimer){
        mmEffectRate(h,512);
    }
}

void sfxRate(int sound, float rate){
    int id = SoundEffectIds[sound];

    if(id == -1)
        return;

    mm_sound_effect s = {
        {(mm_word) id},
        (mm_hword)((rate) * (1 << (10 - (game->zoneTimer != 0)))),
        0,
        (u8)(255 * (float) savefile->settings.sfxVolume / 10),
        128,
    };

    mmEffectEx(&s);
}

void stopDMA(){
    REG_DMA0CNT &= ~(1 << 0x1f);
}

void setMosaic(int sx, int sy){
    REG_MOSAIC = MOS_BUILD(sx,sy,sx,sy);
}

void clearSprites(int count){
    oam_init(obj_buffer, count);
}

void clearTiles(int tileset, int index, int count){
    memset32(&tile_mem[tileset][index],0,8 * count);
}

IWRAM_CODE void onVBlank() {

    mmVBlank();
    LINK_UNIVERSAL_ISR_VBLANK();

    if(gradientEnabled)
        DMA_TRANSFER(&pal_bg_mem[0], gradientTable, 1, 0, DMA_HDMA);


    if (canDraw) {
        canDraw = false;

        scene->draw();
    }

    frameCounter++;
    mmFrame();
}

void sprite_enable_affine(OBJ_ATTR * sprite,int affineId, bool doubleSize){
    sprite->attr0 |= ATTR0_AFF + doubleSize * ATTR0_AFF_DBL_BIT;
    sprite->attr1 |= ATTR1_AFF_ID(affineId);
}

void sprite_enable_mosaic(OBJ_ATTR * sprite){
    sprite->attr0 |= ATTR0_MOSAIC;
}

void sprite_set_id(OBJ_ATTR * sprite, int id){
    sprite = &obj_buffer[id];
}

void sprite_set_size(OBJ_ATTR * sprite, FIXED size, int aff_id){
    obj_aff_identity(&obj_aff_buffer[aff_id]);
    obj_aff_scale(&obj_aff_buffer[aff_id], size, size);
}

void sprite_rotscale(OBJ_ATTR * sprite, FIXED sizex, FIXED sizey, int angle , int aff_id){
    obj_aff_identity(&obj_aff_buffer[aff_id]);
    obj_aff_rotscale(&obj_aff_buffer[aff_id], sizex, sizey, angle);
}

void setMusicTempo(int tempo){
    mmSetModuleTempo(tempo);
}

void setMusicVolume(int volume){
    mmSetModuleVolume(volume);
}

void startSong(int song, bool loop){
    int n = song;

    if(n < 0)
        return;

    mmStart(n, (!loop)? MM_PLAY_ONCE:MM_PLAY_LOOP);
}

void stopSong(){
    mmStop();
}

void setTiles(int tilemap, int index, int count, u32 tile){
    u16 * dst = (u16 * ) &se_mem[tilemap];
    memset16(&dst[index], tile, count);
}

void pauseSong(){
    mmPause();
}

void resumeSong(){
    mmResume();
}

void clearTilemapEntries(int tilemap, int index, int count){
    u16 * dst = (u16*) &se_mem[tilemap];

    memset16(&dst[index],0,count);
}

void loadSavefile(){
    if(savefile != nullptr)
        delete savefile;

    savefile = new Save();

    volatile u8* sf = (volatile u8*)sram_mem;

    u8* arr = (u8*)savefile;

    for (int i = 0; i < (int)(sizeof(Save)); i++)
        arr[i] = (u8)sf[i];
}

void saveSavefile(){
    volatile u8* sf = (volatile u8*)sram_mem;

    u8* arr = (u8*)savefile;

    for (int i = 0; i < (int)sizeof(Save); i++)
        sf[i] = (u8)arr[i];

    if(ENABLE_FLASH_SAVE)
        save_sram_flash();
}

void mirrorPalettes(int index, int count){
    memcpy16(&pal_bg_mem[index],&pal_obj_mem[index],(8 * 16));
}

void toggleHBlank(bool state){
    if(state){
        interrupt_set_handler(INTR_HBLANK, onHBlank);
        interrupt_enable(INTR_HBLANK);
    }else{
        interrupt_disable(INTR_HBLANK);
    }
}

void savePalette(COLOR * dst){
    memcpy32(dst, pal_bg_mem, 256);
}

void enableBlend(int info){
    REG_BLDCNT = info;
}

void sprite_enable_blend(OBJ_ATTR * sprite){
    sprite->attr0 |= ATTR0_BLEND;
}

int prevDispCnt = 0;

void toggleRendering(bool state){
    if(state){
        REG_DISPCNT = prevDispCnt;
    } else {
        prevDispCnt = REG_DISPCNT;
        REG_DISPCNT = 0;
    }
}

void sprite_enable_flip(OBJ_ATTR *sprite, bool flipX, bool flipY){
    sprite->attr1 |= flipX << 0xc;
    sprite->attr1 |= flipY << 0xd;
}

void quit(){}

void findSoundbank(){
    const GBFS_FILE * gbfsFile = find_first_gbfs_file(0);
    const void * myFile = gbfs_get_obj(gbfsFile, "soundbank.bin", NULL);

    if(myFile == NULL){
        log("couldn't find soundbank.bin");
        return;
    }

    log("found soundbank.bin");

    soundbank = (u8*) myFile;
}

void buildEffectLocations(){

    const GBFS_FILE * gbfsFile = find_first_gbfs_file(0);
    const void * myFile = gbfs_get_obj(gbfsFile, "effect_locations.bin", NULL);

    u32 * file = (u32*) myFile;

    if(file[0] != 0x51){
        log("couldn't find effect location file");
        return;
    }

    log("found location file");

    file++;

    int count = *file++;
    for(int i = 0; i < count; i++){
        SoundEffectIds[i] = *file++;
    }

    count = *file++;
    log("found " + std::to_string(count) + " menu songs");
    for(int i = 0; i < count; i++){
        songs.menu.push_back(*file++);
    }

    count = *file++;
    log("found " + std::to_string(count) + " game songs");
    for(int i = 0; i < count; i++){
        songs.game.push_back(*file++);
    }
}

void loadAudio(){
    log("Trying to load from appended files...");

    findSoundbank();
    buildEffectLocations();
}

void enableLayerWindow(int layer, int x1, int y1, int x2, int y2, bool invert){}

void disableLayerWindow(int layer){}

std::map<int,std::string> keyToString = {
    {KEY_LEFT,"Left"},
    {KEY_RIGHT,"Right"},
    {KEY_UP,"Up"},
    {KEY_DOWN,"Down"},
    {KEY_A,"A"},
    {KEY_B,"B"},
    {KEY_L,"L"},
    {KEY_R,"R"},
    {KEY_START,"Start"},
    {KEY_SELECT,"Select"},
};

void refreshWindowSize(){}

void toggleSpriteSorting(bool state){
    sortSprites = state;
}

IWRAM_CODE void onKeyInterrupt(){
    mmStop();
    REG_IME = 0;
    REG_DMA0CNT_H &= ~DMA_ENABLE;
    REG_DMA1CNT_H &= ~DMA_ENABLE;
    REG_DMA2CNT_H &= ~DMA_ENABLE;
    REG_DMA3CNT_H &= ~DMA_ENABLE;
    REG_TM0D = 0;
    REG_TM1D = 0;
    REG_TM2D = 0;
    REG_TM3D = 0;
    SoftReset();
}
#endif
