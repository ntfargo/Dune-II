/*
 * cRandomMapGenerator.cpp
 *
 *  Created on: 16 nov. 2010
 *      Author: Stefan
 */

#include "../include/d2tmh.h"

cRandomMapGenerator::cRandomMapGenerator() {
}

cRandomMapGenerator::~cRandomMapGenerator() {

}

void cRandomMapGenerator::generateRandomMap() {
    // create random map
    map.init(128, 128);

    int a_spice = rnd((game.iSkirmishStartPoints * 8)) + (game.iSkirmishStartPoints * 12);
    int a_rock = 32 + rnd(game.iSkirmishStartPoints*3);
    int a_hill = 2 + rnd(12);

    // rock terrain is placed not near centre, also, we want 4 plateaus be
    // placed not too near to each other
    int maxRockSpots = 18;
    int iSpotRock[18]; // first 4
    memset(iSpotRock, -1, sizeof(iSpotRock));

    int totalSpots = a_spice + a_rock + a_hill;
    float piece = (float)1.0f / totalSpots;

    int iSpot = 0;
    int iFails = 0;

    s_PreviewMap &randomMapEntry = PreviewMap[0];
    randomMapEntry.width = 128;
    randomMapEntry.height = 128;
    memset(randomMapEntry.iStartCell, -1, sizeof(randomMapEntry.iStartCell));

    int iDistance = 16;

    if (game.iSkirmishStartPoints == 2) iDistance = 32;
    if (game.iSkirmishStartPoints == 3) iDistance = 26;
    if (game.iSkirmishStartPoints == 4) iDistance = 22;

    float progress = 0;

    // x = 160
    // y = 180
    draw_sprite(bmp_screen, (BITMAP *) gfxinter[BMP_GENERATING].dat, 160, 180);

    // draw
    while (a_rock > 0) {
        int iCll = map.getRandomCellWithinMapWithSafeDistanceFromBorder(4);
        if (iCll < 0) continue;

        bool bOk = true;
        if (iSpot < maxRockSpots) {
            for (int s = 0; s < maxRockSpots; s++) {
                if (iSpotRock[s] > -1) {
                    if (ABS_length(map.getCellX(iCll), map.getCellY(iCll), map.getCellX(iSpotRock[s]),
                                   map.getCellY(iSpotRock[s])) < iDistance) {
                        bOk = false;
                    } else {
                        iFails++;
                    }
                }
            }
        }

        if (iFails > 10) {
            iFails = 0;
            bOk = true;
        }

        if (bOk) {
            progress += piece;
            mapEditor.createField(iCll, TERRAIN_ROCK, 5500 + rnd(3500));

            if (iSpot < game.iSkirmishStartPoints) {
                randomMapEntry.iStartCell[iSpot] = iCll;
            } else {
                mapEditor.createField(iCll, TERRAIN_MOUNTAIN, 5 + rnd(15));
            }

            if (iSpot < maxRockSpots) {
                iSpotRock[iSpot] = iCll;
            }

            iSpot++;
            a_rock--;

            // blit on screen
            drawProgress(progress);
            blit(bmp_screen, screen, 0, 0, 0, 0, game.screen_x, game.screen_y);

        }

        // take screenshot
        if (key[KEY_F11]) {
            char filename[25];
            sprintf(filename, "%dx%d_%d.bmp", game.screen_x, game.screen_y, game.screenshot);
            save_bmp(filename, bmp_screen, general_palette);
            game.screenshot++;
        }

    }

    // soft out rock a bit
    mapEditor.removeSingleRockSpots();

    // blit on screen
    drawProgress(progress);
    blit(bmp_screen, screen, 0, 0, 0, 0, game.screen_x, game.screen_y);

    mapEditor.removeSingleRockSpots();


    // blit on screen
    drawProgress(progress);
    blit(bmp_screen, screen, 0, 0, 0, 0, game.screen_x, game.screen_y);

    mapEditor.removeSingleRockSpots();

    // blit on screen
    drawProgress(progress);
    blit(bmp_screen, screen, 0, 0, 0, 0, game.screen_x, game.screen_y);

    while (a_spice > 0) {
        int iCll = map.getRandomCellWithinMapWithSafeDistanceFromBorder(0);
        mapEditor.createField(iCll, TERRAIN_SPICE, 2500);
        progress += piece;
        a_spice--;
        // blit on screen
        drawProgress(progress);
        blit(bmp_screen, screen, 0, 0, 0, 0, game.screen_x, game.screen_y);
    }

    while (a_hill > 0) {
        int cell = map.getRandomCellWithinMapWithSafeDistanceFromBorder(0);
        mapEditor.createField(cell, TERRAIN_HILL, 500 + rnd(500));
        a_hill--;
        progress += piece;
        // blit on screen
        drawProgress(progress);
        blit(bmp_screen, screen, 0, 0, 0, 0, game.screen_x, game.screen_y);
    }

    // end of map creation
    mapEditor.smoothMap();

    // blit on screen
    progress += 25;
    drawProgress(progress);
    blit(bmp_screen, screen, 0, 0, 0, 0, game.screen_x, game.screen_y);

    clear_to_color(randomMapEntry.terrain, makecol(0, 0, 0));

    // now put in previewmap 0
    for (int x = 0; x < map.getWidth(); x++)
        for (int y = 0; y < map.getHeight(); y++) {

            drawProgress(progress);

            int cll = map.getCellWithMapDimensions(x, y);
            if (cll < 0) continue;

            int iColor = makecol(194, 125, 60);

            // rock
            int cellType = map.getCellType(cll);
            if (cellType == TERRAIN_ROCK) iColor = makecol(80, 80, 60);
            if (cellType == TERRAIN_MOUNTAIN) iColor = makecol(48, 48, 36);
            if (cellType == TERRAIN_SPICEHILL) iColor = makecol(180, 90, 25); // bit darker
            if (cellType == TERRAIN_SPICE) iColor = makecol(186, 93, 32);
            if (cellType == TERRAIN_HILL) iColor = makecol(188, 115, 50);

            randomMapEntry.mapdata[cll] = cellType;

            for (int s = 0; s < 4; s++) {
                if (randomMapEntry.iStartCell[s] > -1) {
                    int sx = map.getCellX(randomMapEntry.iStartCell[s]);
                    int sy = map.getCellY(randomMapEntry.iStartCell[s]);

                    if (sx == x && sy == y)
                        iColor = makecol(255, 255, 255);
                }
            }

            putpixel(randomMapEntry.terrain, x, y, iColor);
        }

    // blit on screen
    drawProgress(progress);
    blit(bmp_screen, screen, 0, 0, 0, 0, game.screen_x, game.screen_y);
}

void cRandomMapGenerator::drawProgress(float progress) const {
    int iProgress = progress * 211;
    rectfill(bmp_screen, 216, 225, 216 + iProgress, 257, makecol(255, 0, 0));
}
