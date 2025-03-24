#include "sceneColorEditor.hpp"

#include "sprite30tiles_bin.h"
#include "sprites.h"
#include "text.h"
#include <posprintf.h>
#include <string>

void ColorEditorScene::init() {
    setPaletteColor(0, 0, RGB15(41, 205, 41), 1);
    setPalette();
    setSkin(1);
    toggleBG(2, true);
    toggleBG(1, true);
    buildBG(2, 1, 25, 0, 2, 0);

    resetSmallText();

    setSmallTextArea(256, 0, 18, 30, 20);

    drawText();

    // background
    setTiles(26, 0, 32 * 32, 35 * !(savefile->settings.lightMode));

    // arrow
    loadTiles(2, 105, sprite41tiles_bin, sprite41tiles_bin_size / 32);

    // cursor
    loadSpriteTiles(512 + 200, sprite29tiles_bin, 2, 2);

    // color sprites
    loadSpriteTiles(254, &sprite48tiles_bin, 1, 1);

    // color sprite colors
    setPaletteColor(16 + 0, 15, RGB15(31, 0, 0), 1);
    setPaletteColor(16 + 1, 15, RGB15(0, 31, 0), 1);
    setPaletteColor(16 + 2, 15, RGB15(5, 5, 31), 1);

    // create solid color tiles for each color
    for (int i = 1; i < 4; i++) {
        TILE t;

        const u32 fill = 0x11111111 * i;

        memset32(&t, fill, 8);

        loadTiles(1, i, &t, 1);
    }

    showPalettes();

    for (int i = 0; i < 3; i++) {
        colorSprites[i] = &obj_buffer[15 + i];

        sprite_set_attr(colorSprites[i], ShapeSquare, 0, 254, i, 0);
        sprite_hide(colorSprites[i]);
    }
}

void ColorEditorScene::draw() {
    showCursor();

    if (!refresh) {
        showSprites(128);

        return;
    }
    refresh = false;

    showMinos();

    c = 0;

    const int space = 5;
    const int height = 13 + colorSelectorHeight;
    const int tile = 105;

    for (int i = 0; i < 3; i++) {
        int x = 14 + (i - 1) * space;

        char buff[8];

        if (hsvMode) {
            switch (i) {
            case 0:
                posprintf(buff, "H%03d", hsv.hue);
                break;
            case 1:
                posprintf(buff, "S%03d", (int)(hsv.saturation * 100));
                break;
            case 2:
                posprintf(buff, "V%03d", (int)(hsv.value * 100));
                break;
            }

        } else {
            posprintf(buff, "%02d", color1[i]);
        }

        aprintColor(buff, x - hsvMode, height, onColors);

        if (i == selection && !onColors) {
            setTile(29, x, height - 1, tileBuild(tile, false, false, 15));
            setTile(29, x + 1, height - 1,
                    tileBuild(tile + 1, false, false, 15));
            setTile(29, x, height + 1, tileBuild(tile, false, true, 15));
            setTile(29, x + 1, height + 1,
                    tileBuild(tile + 1, false, true, 15));
        } else {
            aprint("  ", x, height - 1);
            aprint("  ", x, height + 1);
        }

        if (!hsvMode) {
            sprite_unhide(colorSprites[i], 0);
            c += color1[i] << (5 * i);
        } else {
            sprite_hide(colorSprites[i]);
        }
        sprite_set_pos(colorSprites[i], x * 8 - 9, height * 8);
    }

    if (hsvMode) {
        c = hsv2rgb(hsv);
    }

    showSprites(128);
}

void ColorEditorScene::update() {
    if (control())
        return;

    canDraw = true;

    cursorTimer++;

    const int animationLength = 32;

    if (!onColors || cursorTimer < animationLength / 2) {
        loadSpriteTiles(512 + 200, sprite29tiles_bin, 2, 2);
    } else {
        loadSpriteTiles(512 + 200, sprite30tiles_bin, 2, 2);
        if (cursorTimer >= animationLength - 1)
            cursorTimer = 0;
    }
}

bool ColorEditorScene::control() {
    key_poll();

    MenuKeys k = savefile->settings.menuKeys;

    if (key_hit(k.pause)) {
        changeScene(previousScene(), Transitions::FADE);
        return true;
    }

    if (key_hit(k.special1)) {
        hsvMode = !hsvMode;

        if (hsvMode)
            setHSV();
        else
            setRGB();

        sfx(SFX_MENUCONFIRM);
        refresh = true;
        drawText();
        return false;
    }

    if (onColors) {
        if (key_hit(k.left)) {
            cursorX--;

            if (cursorX < 0)
                cursorX = 6;

            refresh = true;
            sfx(SFX_SHIFT2);
        }

        if (key_hit(k.right)) {
            cursorX++;

            if (cursorX > 6)
                cursorX = 0;

            refresh = true;
            sfx(SFX_SHIFT2);
        }

        if (key_hit(k.up)) {
            cursorY--;

            if (cursorY < 0)
                cursorY = 2;

            refresh = true;
            sfx(SFX_SHIFT2);
        }

        if (key_hit(k.down)) {
            cursorY++;

            if (cursorY > 2)
                cursorY = 0;

            refresh = true;
            sfx(SFX_SHIFT2);
        }

        if (key_hit(k.confirm)) {
            onColors = false;
            setHSV();
            setRGB();
            refresh = true;
            sfx(SFX_MENUCONFIRM);
        }
    } else {
        if (key_hit(k.confirm)) {
            calculate();
            refresh = true;
        }

        if (key_hit(k.cancel)) {
            onColors = true;
            refresh = true;
            sfx(SFX_MENUCANCEL);
        }

        if (key_hit(k.left)) {
            selection--;
            if (selection < 0)
                selection = maxSelection;
            refresh = true;
        }

        if (key_hit(k.right)) {
            selection++;
            if (selection > maxSelection)
                selection = 0;
            refresh = true;
        }

        if (!hsvMode) {
            if (key_hit(k.up)) {
                if (color1[selection] < 31)
                    color1[selection]++;
                refresh = true;
                calculate();
                sfx(SFX_SHIFT2);
            }

            if (key_hit(k.down)) {
                if (color1[selection] > 0)
                    color1[selection]--;
                refresh = true;
                calculate();
                sfx(SFX_SHIFT2);
            }

            if (key_is_down(k.up) || key_is_down(k.down)) {
                das++;

                if (das < dasMax) {
                    das++;
                } else {
                    if (key_is_down(k.up)) {
                        if (color1[selection] < 31) {
                            color1[selection]++;
                            refresh = true;
                            calculate();
                            sfx(SFX_SHIFT2);
                        }
                    } else if (key_is_down(k.down)) {
                        if (color1[selection] > 0) {
                            color1[selection]--;
                            refresh = true;
                            calculate();
                            sfx(SFX_SHIFT2);
                        }
                    }
                    das -= arrMax;
                }
            } else {
                das = 0;
            }

        } else {
            if (key_hit(k.up)) {
                refresh = true;
                calculate();

                if (set(-1))
                    sfx(SFX_SHIFT2);
            }

            if (key_hit(k.down)) {
                refresh = true;
                calculate();

                if (set(+1))
                    sfx(SFX_SHIFT2);
            }

            if (key_is_down(k.up) || key_is_down(k.down)) {
                if (das < dasMax) {
                    das++;
                } else {
                    int dir = (key_is_down(k.down) ? -1 : 1);

                    if (set(dir))
                        sfx(SFX_SHIFT2);

                    refresh = true;
                    calculate();

                    das -= arrMax / (1 + 1 * selection != 0);
                }
            } else {
                das = 0;
            }
        }
    }
    return false;
}

void ColorEditorScene::deinit() {
    // clean up
    clearSpriteTiles(712, 16, 1);
    clearSprites(128);
    showSprites(128);
    clearTilemap(25);
    clearTilemap(26);
    clearTilemap(27);

    setLayerScroll(3, 0, 0);

    clearTiles(2, 110, 400);

    clearText();
    clearSmallText();
}

void ColorEditorScene::calculate() {
    COLOR color;
    if (hsvMode)
        color = hsv2rgb(hsv);
    else
        color = color1[0] + (color1[1] << 5) + (color1[2] << 10);

    setPaletteColor(cursorX, 3 - cursorY, color, 1);
    setPaletteColor(16 + cursorX, 3 - cursorY, color, 1);
}

void ColorEditorScene::showMinos() {

    const int x = 208 - 13 * (savefile->settings.aspectRatio != 0);

    for (int i = 0; i < 7; i++) {
        OBJ_ATTR* sprite = &obj_buffer[i + 1];

        sprite_set_attr(sprite, ShapeSquare, 2, 16 * i, i, 2);
        sprite_unhide(sprite, 0);
        const int y = 8 - 4 * (i == 0) + 4 * (i == 3) + colorSelectorHeight * 8;
        const int sx = 4 + i * 3;
        sprite_set_pos(sprite, sx * 8 - 4 * (i == 0), y);
    }
}

void ColorEditorScene::showPalettes() {
    const int add = (savefile->settings.aspectRatio == 1);

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 3; j++) {
            const int y = j * 2 + 5 + colorSelectorHeight;
            const int x = 4 + i * 3 + 1;

            setTile(25, x, y, tileBuild(3 - j, false, false, i));
            setTile(25, x, y + 1, tileBuild(3 - j, false, false, i));
            setTile(25, x + 1, y, tileBuild(3 - j, false, false, i));
            setTile(25, x + 1, y + 1, tileBuild(3 - j, false, false, i));
        }
    }
}

void ColorEditorScene::showCursor() {
    OBJ_ATTR* sprite = &obj_buffer[32];

    const int y = cursorY * 2 + 5 + colorSelectorHeight;
    const int x = 4 + cursorX * 3 + 1;

    sprite_unhide(sprite, 0);
    sprite_set_attr(sprite, ShapeSquare, 1, 712, 15 - !onColors, 0);
    sprite_set_pos(sprite, x * 8, y * 8);
}

void ColorEditorScene::setHSV() {
    COLOR colors[32][16];
    savePalette((COLOR*)&colors);

    COLOR color = colors[cursorX][3 - cursorY];

    hsv = rgb2hsv(color & 31, (color >> 5) & 31, (color >> 10) & 31);
}

void ColorEditorScene::setRGB() {
    COLOR colors[32][16];
    savePalette((COLOR*)&colors);

    COLOR color = colors[cursorX][3 - cursorY];

    color1[0] = color & 31;
    color1[1] = (color >> 5) & 31;
    color1[2] = (color >> 10) & 31;
}

void ColorEditorScene::drawText() {
    clearText();
    int offset = 2;
    if (savefile->settings.aspectRatio == 1) {
        offset += (240 - 214) / 2;
    }

    std::string colorModeString =
        getStringFromKey(savefile->settings.menuKeys.special1);

    std::string colorMode =
        "Press " + colorModeString + " for " + ((!hsvMode) ? "HSV" : "RGB");
    aprints(colorMode, offset, 0, 2);
}

void ColorSelectorScene::init() {
    // background
    setTiles(26, 0, 32 * 32, 35 * !(savefile->settings.lightMode));
}

void ColorSelectorScene::draw() {}

void ColorSelectorScene::update() {
    if (control())
        return;

    canDraw = true;
}

bool ColorSelectorScene::control() {
    MenuKeys k = savefile->settings.menuKeys;

    key_poll();

    if (key_hit(k.confirm)) {
        changeScene(new ColorEditorScene(), Transitions::FADE);
        return true;
    }

    if (key_hit(k.cancel)) {
        previousElement = "Color Editor";

        clearTilemap(27);
        buildBG(3, 0, 27, 0, 1, 0);
        toggleBG(3, true);

        changeScene(new ConfirmSaveScene(), Transitions::FADE);
        return true;
    }

    return false;
}

void ColorSelectorScene::deinit() {}
