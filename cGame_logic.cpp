/*

  Dune II - The Maker

  Author : Stefan Hendriks
  Contact: stefanhen83@gmail.com
  Website: http://dune2themaker.fundynamic.com

  2001 - 2011 (c) code by Stefan Hendriks

  -----------------------------------------
  Initialization of variables
  Game logic
  Think functions
  -----------------------------------------

  */

#include <vector>
#include <algorithm>
#include <random>
#include "include/d2tmh.h"
#include "cGame.h"


cGame::cGame() {
	screen_x = 800;
	screen_y = 600;
    windowed = false;
    bPlaySound = true;
    bPlayMusic = true;
    bMp3 = false;
	// default INI screen width and height is not loaded
	// if not loaded, we will try automatic setup
	ini_screen_width=-1;
	ini_screen_height=-1;

    memset(version, 0, sizeof(version));
    sprintf(version, "0.5.0");
}


void cGame::init() {
	iMaxVolume = 220;

	screenshot=0;
	bPlaying=true;

    bSkirmish=false;
	iSkirmishStartPoints=2;

    // Alpha (for fading in/out)
    iAlphaScreen=0;           // 255 = opaque , anything else
    iFadeAction=2;            // 0 = NONE, 1 = fade out (go to 0), 2 = fade in (go to 255)

    iRegionState=1;// default = 0
    iRegionScene=0;           // scene
    iRegionSceneAlpha=0;           // scene
    memset(iRegionConquer, -1, sizeof(iRegionConquer));
    memset(iRegionHouse, -1, sizeof(iRegionHouse));
    memset(cRegionText, 0, sizeof(cRegionText));
    //int iConquerRegion[MAX_REGIONS];     // INDEX = REGION NR , > -1 means conquered..

    iSkirmishMap=-1;

    iMusicVolume=128; // volume is 0...

	paths_created=0;
	hover_unit=-1;

    state = GAME_MENU;

    iWinQuota=-1;              // > 0 means, get this to win the mission, else, destroy all!

	selected_structure=-1;

	// mentat
	memset(mentat_sentence, 0, sizeof(mentat_sentence));

	bPlaceIt=false;			// we do not place
	bPlacedIt=false;

	map_width  = 64;
	map_height = 64;

	mouse_tile = MOUSE_NORMAL;

	fade_select=255;

    bFadeSelectDir=true;    // fade select direction

    iRegion=1;          // what region ? (calumative, from player perspective, NOT the actual region number)
	iMission=0;         // calculated by mission loading (region -> mission calculation)
	iHouse=-1;			// what house is selected for playing?

    shake_x=0;
    shake_y=0;
    TIMER_shake=0;

    iMusicType=MUSIC_MENU;

    TIMER_movie=0;
    iMovieFrame=-1;

	map.init(map_width, map_height);

	for (int i=0; i < MAX_PLAYERS; i++) {
		player[i].init(i);
        aiplayer[i].init(i);
    }

	for (int i=0; i < MAX_UNITS; i++) {
	    unit[i].init(i);
	}

	for (int i=0; i < MAX_PARTICLES; i++) {
	    particle[i].init();
	}

	// Units & Structures are already initialized in map.init()
	if (game.bMp3 && mp3_music) {
	    almp3_stop_autopoll_mp3(mp3_music); // stop auto poll
	}

	// Load properties
	INI_Install_Game(game_filename);

	iMentatSpeak=-1;			// = sentence to draw and speak with (-1 = not ready)

	TIMER_mentat_Speaking=-1;	// speaking = time
	TIMER_mentat_Mouth=0;
	TIMER_mentat_Eyes=0;
	TIMER_mentat_Other=0;

	iMentatMouth=3;			// frames	... (mouth)
	iMentatEyes=3;				// ... for mentat ... (eyes)
	iMentatOther=0;			// ... animations . (book/ring)

    mp3_music=NULL;
}

// TODO: Bad smell (duplicate code)
// initialize for missions
void cGame::mission_init() {
    mapCamera->resetZoom();

    iMusicVolume=128; // volume is 0...

	paths_created=0;
	hover_unit=-1;

    iWinQuota=-1;              // > 0 means, get this to win the mission, else, destroy all!

	selected_structure=-1;

	// mentat
	memset(mentat_sentence, 0, sizeof(mentat_sentence));

	bPlaceIt=false;			// we do not place
	bPlacedIt=false;

	mouse_tile = MOUSE_NORMAL;

	fade_select=255;

    bFadeSelectDir=true;    // fade select direction

    shake_x=0;
    shake_y=0;
    TIMER_shake=0;

    TIMER_movie=0;
    iMovieFrame=-1;

    map.init(game.map_width, game.map_height);

    // clear out players but not entirely
    for (int i=0; i < MAX_PLAYERS; i++) {
        int h = player[i].getHouse();

        player[i].init(i);
        player[i].setHouse(h);

        aiplayer[i].init(i);

        if (bSkirmish) {
            player[i].credits = 2500;
        }
    }

    // instantiate the creditDrawer with the appropriate player. Only
    // possible once game has been initialized and player has been created.
    assert(drawManager);
    assert(drawManager->getCreditsDrawer());
    drawManager->getCreditsDrawer()->setCredits();
}


void cGame::think_winlose() {
    bool bSucces=false;
    bool bFailed=true;

    // determine if player is still alive
    for (int i=0; i < MAX_STRUCTURES; i++)
        if (structure[i])
            if (structure[i]->getOwner() == 0)
            {
                bFailed=false; // no, we are not failing just yet
                break;
            }

    // determine if any unit is found
    if (bFailed)
    {
        // check if any unit is ours, if not, we have a problem (airborn does not count)
        for (int i=0; i < MAX_UNITS; i++)
            if (unit[i].isValid())
                if (unit[i].iPlayer == 0)
                {
                    bFailed=false;
                    break;
                }
    }


    // win by money quota
    if (iWinQuota > 0)
    {
        if (player[0].credits >= iWinQuota)
        {
            // won!
            bSucces=true;
        }
    }
    else
    {
        // determine if any player (except sandworm) is dead
        bool bAllDead=true;
        for (int i=0; i < MAX_STRUCTURES; i++)
            if (structure[i])
                if (structure[i]->getOwner() > 0 && structure[i]->getOwner() != AI_WORM)
                {
                    bAllDead=false;
                    break;
                }

        if (bAllDead)
        {
                // check units now
            for (int i=0; i < MAX_UNITS; i++)
                if (unit[i].isValid())
                    if (unit[i].iPlayer > 0 && unit[i].iPlayer != AI_WORM)
                        if (units[unit[i].iType].airborn == false)
                        {
                               bAllDead=false;
                               break;
                        }

        }

        if (bAllDead)
            bSucces=true;

    }


    // On succes...
    if (bSucces) {
        state = GAME_WINNING;

        shake_x=0;
        shake_y=0;
        TIMER_shake=0;

		play_voice(SOUND_VOICE_07_ATR);

        playMusicByType(MUSIC_WIN);

        // copy over
        blit(bmp_screen, bmp_winlose, 0, 0, 0, 0, screen_x, screen_y);

        allegroDrawer->drawCenteredSprite(bmp_winlose, (BITMAP *)gfxinter[BMP_WINNING].dat);
    }

    if (bFailed)
    {
        state = GAME_LOSING;

        shake_x=0;
        shake_y=0;
        TIMER_shake=0;

		play_voice(SOUND_VOICE_08_ATR);

        playMusicByType(MUSIC_LOSE);

        // copy over
        blit(bmp_screen, bmp_winlose, 0, 0, 0, 0, screen_x, screen_y);

        allegroDrawer->drawCenteredSprite(bmp_winlose, (BITMAP *)gfxinter[BMP_LOSING].dat);
    }
}

// MOVIE: Play frames
void cGame::think_movie()
{
    if (gfxmovie != NULL) {
        TIMER_movie++;

        if (TIMER_movie > 20) {
            iMovieFrame++;

            if (gfxmovie[iMovieFrame].type == DAT_END ||
				gfxmovie[iMovieFrame].type != DAT_BITMAP) {
                iMovieFrame=0;
			}
            TIMER_movie=0;
		}
    }
}

//TODO: move to mentat classes
void cGame::think_mentat()
{

	if (TIMER_mentat_Speaking > 0) {
		TIMER_mentat_Speaking--;
	}

	if (TIMER_mentat_Speaking == 0)	{
		// calculate speaking stuff

		iMentatSpeak += 2; // makes 0, 2, etc.

		if (iMentatSpeak > 8) {
			iMentatSpeak = -2;
			TIMER_mentat_Speaking=-1;
			return;
		}

		// lentgh calculation of time
		int iLength = strlen(mentat_sentence[iMentatSpeak]);
		iLength += strlen(mentat_sentence[iMentatSpeak+1]);

		if (iLength < 2) {
			iMentatSpeak = -2;
			TIMER_mentat_Speaking=-1;
			return;
		}

		TIMER_mentat_Speaking = iLength*12;
	}

	if (TIMER_mentat_Mouth > 0) {
		TIMER_mentat_Mouth--;
	} else if (TIMER_mentat_Mouth == 0) {

		if (TIMER_mentat_Speaking > 0) {
			int iOld = iMentatMouth;

			if (iMentatMouth == 0) {
				// when mouth is shut, we wait a bit.
				if (rnd(100) < 45) {
					iMentatMouth += (1 + rnd(4));
				} else {
					TIMER_mentat_Mouth=3; // wait
				}

				// correct any frame
				if (iMentatMouth > 4) {
					iMentatMouth-=5;
				}
			} else {
				iMentatMouth += (1 + rnd(4));

				if (iMentatMouth > 4) {
					iMentatMouth-=5;
				}
			}

			// Test if we did not set the timer, when not, we changed stuff, and we
			// have to make sure we do not reshow the same animation.. which looks
			// odd!
			if (TIMER_mentat_Mouth == 0) {
				if (iMentatMouth == iOld) {
					iMentatMouth++;
				}

				// correct if nescesary:
				if (iMentatMouth > 4) {
					iMentatMouth-=5;
				}

				// Done!
			}
		} else {
			iMentatMouth=0; // when there is no sentence, do not animate mouth
		}

		TIMER_mentat_Mouth=-1; // this way we make sure we do not update it too much
	} // speaking


	if (TIMER_mentat_Eyes > 0)
	{
		TIMER_mentat_Eyes--;
	}
	else
	{
		int i = rnd(100);

        int iWas = iMentatEyes;

        if (i < 30)
			iMentatEyes = 3;
		else if (i >= 30 && i < 60)
			iMentatEyes = 0;
		else
			iMentatEyes=4;

        // its the same
        if (iMentatEyes == iWas)
            iMentatEyes = rnd(4);

        if (iMentatEyes != 4)
            TIMER_mentat_Eyes = 90 + rnd(160);
        else
            TIMER_mentat_Eyes = 30;
	}

	// think wohoo
	if (TIMER_mentat_Other > 0)
	{
		TIMER_mentat_Other--;
	}
	else
	{
		iMentatOther = rnd(5);
	}

}

// TODO: Move to music related class (MusicPlayer?)
void cGame::think_music()
{
    if (!game.bPlayMusic) // no music enabled, so no need to think
        return;

    // all this does is repeating music in the same theme.
    if (iMusicType < 0)
        return;

    if (!isMusicPlaying()) {

        if (iMusicType == MUSIC_ATTACK) {
            iMusicType = MUSIC_PEACE; // set back to peace
        }

        playMusicByType(iMusicType);
    }
}

bool cGame::isMusicPlaying() {
    if (bMp3 && mp3_music) {
        int s = almp3_poll_mp3(mp3_music);
        return !(s == ALMP3_POLL_PLAYJUSTFINISHED || s == ALMP3_POLL_NOTPLAYING);
    }

    // MIDI mode:
    return MIDI_music_playing();
}

void cGame::poll()
{
	cMouse::updateState();
	cGameControlsContext * context = player[HUMAN].getGameControlsContext();
	context->updateState();

    clear(bmp_screen);
	mouse_tile = MOUSE_NORMAL;

	// change this when selecting stuff
	int mc = context->getMouseCell();
	if (mc > -1) {

		// check if any unit is 'selected'
		for (int i=0; i < MAX_UNITS; i++) {
			if (unit[i].isValid()) {
				if (unit[i].iPlayer == 0) {
					if (unit[i].bSelected) {
						mouse_tile = MOUSE_MOVE;
						break;
					}
				}
			}
		}

        if (mouse_tile == MOUSE_MOVE)
        {
        	// change to attack cursor if hovering over enemy unit
			if (mapUtils->isCellVisibleForPlayerId(HUMAN, mc)) {

                int idOfUnitOnCell = map.getCellIdUnitLayer(mc);

                if (idOfUnitOnCell > -1)
				{
					int id = idOfUnitOnCell;

					if (unit[id].iPlayer > 0)
						mouse_tile = MOUSE_ATTACK;
				}

                int idOfStructureOnCell = map.getCellIdStructuresLayer(mc);

                if (idOfStructureOnCell > -1)
				{
					int id = idOfStructureOnCell;

					if (structure[id]->getOwner() > 0)
						mouse_tile = MOUSE_ATTACK;
				}

				if (key[KEY_LCONTROL]) {
					mouse_tile = MOUSE_ATTACK;
				}

				if (key[KEY_ALT]) {
					mouse_tile = MOUSE_MOVE;
				}

			} // visible
        }
    }

    if (mouse_tile == MOUSE_NORMAL)
    {
        // when selecting a structure
        if (game.selected_structure > -1)
        {
            int id = game.selected_structure;
            if (structure[id]->getOwner() == 0)
                if (key[KEY_LCONTROL])
                    mouse_tile = MOUSE_RALLY;

        }
    }

	bPlacedIt=false;

	//selected_structure=-1;
	hover_unit=-1;

	// update power
	// update total spice capacity
	for (int p = 0; p < MAX_PLAYERS; p++) {
		cPlayer * thePlayer = &player[p];
		thePlayer->use_power = structureUtils.getTotalPowerUsageForPlayer(thePlayer);
		thePlayer->has_power = structureUtils.getTotalPowerOutForPlayer(thePlayer);
		// update spice capacity
		thePlayer->max_credits = structureUtils.getTotalSpiceCapacityForPlayer(thePlayer);
	}
}

void cGame::combat() {

	if (iFadeAction == 1) // fading out
    {
        draw_sprite(bmp_screen, bmp_fadeout, 0, 0);
        return;
    }

    if (iAlphaScreen == 0)
        iFadeAction = 2;
    // -----------------
	bPlacedIt = bPlaceIt;

	drawManager->draw();
	interactionManager->interact();

    // think win/lose
    think_winlose();
}

void cGame::draw_mentat(const int iType)
{
	select_palette( general_palette  );

    // movie
    draw_movie(iType);

	// draw proper background
	if (iType == ATREIDES)
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[MENTATA].dat, 0, 0);
	else if (iType == HARKONNEN)
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[MENTATH].dat, 0, 0);
	else if (iType == ORDOS)
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[MENTATO].dat, 0, 0);
	else // bene
    {
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[MENTATM].dat, 0, 0);

        // when not speaking, draw 'do you wish to join house x'
        if (TIMER_mentat_Speaking < 0)
        {
            draw_sprite(bmp_screen, (BITMAP *)gfxmentat[MEN_WISH].dat, 16, 16);

			// todo house description
        }
    }

    if (cMouse::isLeftButtonClicked())
        if (TIMER_mentat_Speaking > 0)
        {
            TIMER_mentat_Speaking = 1;
        }

    // SPEAKING ANIMATIONS IS DONE IN MENTAT()



}

void cGame::MENTAT_draw_eyes(int iMentat)
{
	int iDrawX=128;
	int iDrawY=240;

	// now draw eyes
	if (iMentat  < 0)
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[BEN_EYES01+ iMentatEyes].dat, iDrawX, iDrawY);



    if (iMentat  == HARKONNEN)
    {
        iDrawX = 64;
        iDrawY = 256;
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[HAR_EYES01+ iMentatEyes].dat, iDrawX, iDrawY);
   }

    if (iMentat  == ATREIDES)
    {
        iDrawX = 80;
        iDrawY = 241;
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[ATR_EYES01+ iMentatEyes].dat, iDrawX, iDrawY);
   }


    if (iMentat  == ORDOS)
    {
        iDrawX = 32;
        iDrawY = 240;
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[ORD_EYES01+ iMentatEyes].dat, iDrawX, iDrawY);
   }

}


void cGame::MENTAT_draw_mouth(int iMentat)
{

	int iDrawX=112;
	int iDrawY=272;

	// now draw speaking and such
	if (iMentat  < 0)
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[BEN_MOUTH01+ iMentatMouth].dat, iDrawX, iDrawY);


    if (iMentat  == HARKONNEN)
    {
        iDrawX = 64;
        iDrawY = 288;
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[HAR_MOUTH01+ iMentatMouth].dat, iDrawX, iDrawY);
    }

    // 31, 270

    if (iMentat  == ATREIDES)
    {
        iDrawX = 80;
        iDrawY = 273;
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[ATR_MOUTH01+ iMentatMouth].dat, iDrawX, iDrawY);
    }

    if (iMentat  == ORDOS)
    {
        iDrawX = 31;
        iDrawY = 270;
		draw_sprite(bmp_screen, (BITMAP *)gfxmentat[ORD_MOUTH01+ iMentatMouth].dat, iDrawX, iDrawY);
    }
}


// draw mentat
void cGame::mentat(int iType)
{
    if (iFadeAction == 1) // fading out
    {
        draw_sprite(bmp_screen, bmp_fadeout, 0, 0);
        return;
    }

    if (iAlphaScreen == 0)
        iFadeAction = 2;
    // -----------------

    bool bFadeOut=false;

	// draw speaking animation, and text, etc

	if (iType > -1)
		draw_mentat(iType); // draw houses

	MENTAT_draw_mouth(iType);
	MENTAT_draw_eyes(iType);

	// draw text
	if (iMentatSpeak >= 0)
	{
	alfont_textprintf(bmp_screen, bene_font, 17,17, makecol(0,0,0), "%s", mentat_sentence[iMentatSpeak]);
	alfont_textprintf(bmp_screen, bene_font, 16,16, makecol(255,255,255), "%s", mentat_sentence[iMentatSpeak]);
	alfont_textprintf(bmp_screen, bene_font, 17,33, makecol(0,0,0), "%s", mentat_sentence[iMentatSpeak+1]);
	alfont_textprintf(bmp_screen, bene_font, 16,32, makecol(255,255,255), "%s", mentat_sentence[iMentatSpeak+1]);
	}

	// mentat mouth
	if (TIMER_mentat_Mouth <= 0)
	{
		TIMER_mentat_Mouth = 13+rnd(5);
	}


    if (TIMER_mentat_Speaking < 0)
    {
        // NO
        draw_sprite(bmp_screen, (BITMAP *)gfxmentat[BTN_REPEAT].dat, 293, 423);

        if ((mouse_x > 293 && mouse_x < 446) && (mouse_y > 423 && mouse_y < 468))
            if (cMouse::isLeftButtonClicked())
            {
                // head back to choose house
                iMentatSpeak=-1; // prepare speaking
                //state = GAME_SELECT_HOUSE;
            }

        // YES/PROCEED
        draw_sprite(bmp_screen, (BITMAP *)gfxmentat[BTN_PROCEED].dat, 466, 423);
        if ((mouse_x > 446 && mouse_x < 619) && (mouse_y >423 && mouse_y < 468))
            if (cMouse::isLeftButtonClicked())
            {
                if (isState(GAME_BRIEFING))
                {
                // proceed, play mission
                state = GAME_PLAYING;
				drawManager->getMessageDrawer()->initCombatPosition();

                // CENTER MOUSE
                cMouse::positionMouseCursor(game.screen_x/2, game.screen_y/2);

                bFadeOut=true;

                playMusicByType(MUSIC_PEACE);
                }
                else if (state == GAME_WINBRIEF)
                {
                //
					if (bSkirmish)
					{
						state = GAME_SETUPSKIRMISH;
                        init_skirmish();
						playMusicByType(MUSIC_MENU);
					}
					else
					{

                    state = GAME_REGION;
                    REGION_SETUP(game.iMission, game.iHouse);

					drawManager->getMessageDrawer()->initRegionPosition();

                    // PLAY THE MUSIC
                    playMusicByType(MUSIC_CONQUEST);
					}

					// PREPARE NEW MENTAT BABBLE
                    iMentatSpeak=-1;

                    bFadeOut=true;
                }
                else if (state == GAME_LOSEBRIEF)
                {
                //
					if (bSkirmish)
					{
						state = GAME_SETUPSKIRMISH;
                        init_skirmish();
						playMusicByType(MUSIC_MENU);
					}
					else
					{
						if (game.iMission > 1)
						{
							state = GAME_REGION;

							game.iMission--; // we did not win
							REGION_SETUP(game.iMission, game.iHouse);
							drawManager->getMessageDrawer()->initRegionPosition();

							// PLAY THE MUSIC
							playMusicByType(MUSIC_CONQUEST);
						}
						else
						{
							state = GAME_BRIEFING;
							playMusicByType(MUSIC_BRIEFING);
						}

					}
                    // PREPARE NEW MENTAT BABBLE
                    iMentatSpeak=-1;

                    bFadeOut=true;
                }

            }


    }

    draw_sprite(bmp_screen, (BITMAP *)gfxdata[mouse_tile].dat, mouse_x, mouse_y);

    if (bFadeOut) {
        FADE_OUT();
    }

}

// draw menu
void cGame::menu()
{
    // FADING STUFF
    if (iFadeAction == 1) // fading out
    {
        draw_sprite(bmp_screen, bmp_fadeout, 0, 0);
        return;
    }

    if (iAlphaScreen == 0) {
        iFadeAction = 2;
    }
    // -----------------

    bool bFadeOut=false;

	if (DEBUGGING)
	{

		for (int x=0; x < game.screen_x; x+= 60)
		{
			for (int y=0; y < game.screen_y; y+= 20)
			{
				rect(bmp_screen, x, y, x+50, y+10, makecol(64,64,64));
				putpixel(bmp_screen, x, y, makecol(255,255,255));
				alfont_textprintf(bmp_screen, bene_font, x, y, makecol(32,32,32), "Debug");
			}

		}
	}

    cTextDrawer textDrawer = cTextDrawer(bene_font);

	// draw main menu title (picture is 640x480)
//	cAllegroDrawer allegroDrawer;
//	allegroDrawer.drawSpriteCenteredRelativelyVertical(bmp_screen, (BITMAP *)gfxinter[BMP_D2TM].dat, 0.3);
//    GUI_DRAW_FRAME(257, 319, 130,143);
//	// draw menu
	int logoWidth = ((BITMAP*)gfxinter[BMP_D2TM].dat)->w;
	int logoHeight = ((BITMAP*)gfxinter[BMP_D2TM].dat)->h;

	int logoX = (game.screen_x / 2) - (logoWidth / 2);
	int logoY = (logoHeight/10);

	draw_sprite(bmp_screen,(BITMAP *)gfxinter[BMP_D2TM].dat,  logoX, logoY);

	int mainMenuFrameX = 257;
	int mainMenuFrameY = 319;
	int mainMenuWidth = 130;
	int mainMenuHeight = 143;

	// adjust x and y according to resolution, we can add because the above values
	// assume 640x480 resolution, and logoX/logoY are already taking care of > resolutions
	mainMenuFrameX += logoX;
	mainMenuFrameY += logoY;

    GUI_DRAW_FRAME(mainMenuFrameX, mainMenuFrameY, mainMenuWidth,mainMenuHeight);

	// Buttons:
	int buttonsX = mainMenuFrameX + 4;

	// PLAY
	int playY = 323 + logoY;
	if (GUI_DRAW_BENE_TEXT_MOUSE_SENSITIVE(buttonsX, playY, "Campaign", makecol(255, 0, 0)))
	{
		if (cMouse::isLeftButtonClicked())
		{
			state = GAME_SELECT_HOUSE; // select house
			bFadeOut = true;
		}
	}

	// SKIRMISH
	int skirmishY = 344 + logoY;
	if (GUI_DRAW_BENE_TEXT_MOUSE_SENSITIVE(buttonsX, skirmishY, "Skirmish", makecol(255, 0, 0)))
	{
		if (cMouse::isLeftButtonClicked())
		{
			game.state = GAME_SETUPSKIRMISH;
			bFadeOut = true;
			INI_PRESCAN_SKIRMISH();

            init_skirmish();
        }
	}

    // MULTIPLAYER
	int multiplayerY = 364 + logoY;
	if (GUI_DRAW_BENE_TEXT_MOUSE_SENSITIVE(buttonsX, multiplayerY, "Multiplayer", makecol(128, 128, 128)))
	{
		if (cMouse::isLeftButtonClicked())
		{
			// NOT YET IMPLEMENTED
			bFadeOut = true;
		}
	}

    // LOAD
	int loadY = 384 + logoY;
	if (GUI_DRAW_BENE_TEXT_MOUSE_SENSITIVE(buttonsX, loadY, "Load", makecol(128, 128, 128)))
	{
		if (cMouse::isLeftButtonClicked())
		{
			// NOT YET IMPLEMENTED
			bFadeOut = true;
		}
	}

    // OPTIONS
	int optionsY = 404 + logoY;
	if (GUI_DRAW_BENE_TEXT_MOUSE_SENSITIVE(buttonsX, optionsY, "Options", makecol(128, 128, 128)))
	{
		if (cMouse::isLeftButtonClicked())
		{
			// NOT YET IMPLEMENTED
			bFadeOut = true;
		}
	}

	// HALL OF FAME
	int hofY = 424 + logoY;
	if (GUI_DRAW_BENE_TEXT_MOUSE_SENSITIVE(buttonsX, hofY, "Hall of Fame", makecol(128, 128, 128)))
	{
		if (cMouse::isLeftButtonClicked())
		{
			// NOT YET IMPLEMENTED
			bFadeOut = true;
		}
	}

	// EXIT
	int exitY = 444 + logoY;
	if (GUI_DRAW_BENE_TEXT_MOUSE_SENSITIVE(buttonsX, exitY, "Exit", makecol(255, 0, 0)))
	{
		if (cMouse::isLeftButtonClicked())
		{
			bFadeOut = true;
			game.bPlaying = false;
		}
	}

	int creditsX = (screen_x / 2) - (alfont_text_length(bene_font, "CREDITS") / 2);
	GUI_DRAW_BENE_TEXT_MOUSE_SENSITIVE(creditsX, 1, "CREDITS", makecol(64, 64, 64));


    // draw version
	textDrawer.drawTextBottomRight(version);

	// mp3 addon?
	if (bMp3) {
		textDrawer.drawTextBottomLeft("Music: MP3 ADD-ON");
    } else {
		textDrawer.drawTextBottomLeft("Music: MIDI");
    }

   	// MOUSE
    draw_sprite(bmp_screen, (BITMAP *)gfxdata[mouse_tile].dat, mouse_x, mouse_y);

	if (key[KEY_ESC]) {
		bPlaying=false;
	}

    if (bFadeOut) {
        game.FADE_OUT();
    }

}

void cGame::init_skirmish() const {
    game.mission_init();

    for (int p = 0; p < AI_WORM; p++) {
        player[p].credits = 2500;
        player[p].iTeam = p;
    }
}

void cGame::setup_skirmish() {
    // FADING STUFF
    if (iFadeAction == 1) // fading out
    {
        draw_sprite(bmp_screen, bmp_fadeout, 0, 0);
        return;
    }

    if (iAlphaScreen == 0)
        iFadeAction = 2;
    // -----------------

    int darkishBackgroundColor = makecol(32, 32, 32);
    int darkishBorderColor = makecol(227, 229, 211);
    int yellow = makecol(255, 207, 41);

    bool bFadeOut=false;

    draw_sprite(bmp_screen,(BITMAP *)gfxinter[BMP_GAME_DUNE].dat, game.screen_x * 0.2, (game.screen_y * 0.5));

	for (int dy=0; dy < game.screen_y; dy+=2) {
		line(bmp_screen, 0, dy, screen_x, dy, makecol(0,0,0));
	}

	int topBarWidth = screen_x + 4;
	int topBarHeight = 21;
	int previewMapHeight = 129;
	int previewMapWidth = 129;

    int sidebarWidth = 158;

    // title box
    GUI_DRAW_FRAME(-1, -1, topBarWidth, topBarHeight);

    int creditsX = (screen_x / 2) - (alfont_text_length(bene_font, "Skirmish") / 2);
    GUI_DRAW_BENE_TEXT(creditsX, 1, "Skirmish");

    int topRightBoxWidth = 276;

    // Players title bar
    int playerTitleBarWidth = screen_x - topRightBoxWidth;
    int playerTitleBarHeight = topBarHeight;
    int playerTitleBarX = 0;
    int playerTitleBarY = topBarHeight;
    GUI_DRAW_FRAME_WITH_COLORS(playerTitleBarX, playerTitleBarY, playerTitleBarWidth, playerTitleBarHeight, makecol(255, 255, 255), darkishBackgroundColor );

    // this is the box at the right from the Player list
    int topRightBoxHeight = playerTitleBarHeight + previewMapHeight;
    int topRightBoxX = screen_x - topRightBoxWidth;
    int topRightBoxY = topBarHeight;
    GUI_DRAW_FRAME(topRightBoxX, topRightBoxY, topRightBoxWidth, topRightBoxHeight);

    // player list
    int playerListWidth = playerTitleBarWidth;
    int playerListBarHeight = topRightBoxHeight;
    int playerListBarX = 0;
    int playerListBarY = playerTitleBarY + topBarHeight;
    GUI_DRAW_FRAME_WITH_COLORS(playerListBarX, playerListBarY, playerListWidth, playerListBarHeight, makecol(255, 255, 255), darkishBackgroundColor);

    // map list
    int mapListHeight = screen_y - (topBarHeight + topRightBoxHeight + topBarHeight + topBarHeight);
    int mapListWidth = topRightBoxWidth;
    int mapListTopX = screen_x - mapListWidth;
    int mapListTopY = topRightBoxY + topRightBoxHeight; // ??

    int mapListFrameX = screen_x - mapListWidth;
    int mapListFrameY = (playerListBarY + playerListBarHeight) - playerTitleBarHeight;
    int mapListFrameWidth = screen_x - mapListFrameX;
    int mapListFrameHeight = topBarHeight;

    // rectangle for map list
    GUI_DRAW_FRAME_WITH_COLORS(mapListTopX, mapListTopY, mapListWidth, mapListHeight, darkishBorderColor, darkishBackgroundColor);

    int previewMapY = topBarHeight + 6;
    int previewMapX = screen_x - (previewMapWidth + 6);

    // TITLE: Map list
    GUI_DRAW_FRAME_WITH_COLORS(mapListFrameX, mapListFrameY, mapListFrameWidth, mapListFrameHeight, darkishBorderColor, darkishBackgroundColor);

    cTextDrawer textDrawer = cTextDrawer(bene_font);
    textDrawer.drawTextCentered("Maps", mapListFrameX, mapListFrameWidth, mapListFrameY + 4, yellow);

    int iStartingPoints=0;

    ///////
	// DRAW PREVIEW MAP
	//////

	// iSkirmishMap holds an index of which map to load, where index 0 means random map generated, although
	// this is only meaningful for rendering, the loading (more below) of that map does not care if it is
	// randomly generated or not.
	if (iSkirmishMap > -1) {
	    // Render skirmish map as-is (pre-loaded map)
		if (iSkirmishMap > 0) {
            if (PreviewMap[iSkirmishMap].name[0] != '\0') {
                if (PreviewMap[iSkirmishMap].terrain) {
                    draw_sprite(bmp_screen, PreviewMap[iSkirmishMap].terrain, previewMapX, previewMapY);
                }

                // count starting points
                for (int s = 0; s < 5; s++) {
                    if (PreviewMap[iSkirmishMap].iStartCell[s] > -1) {
                        iStartingPoints++;
                    }
                }
            }
        }
        else
        {
            // render the 'random generated skirmish map'
            iStartingPoints = iSkirmishStartPoints;

            // when mouse is hovering, draw it, else do not
            if ((mouse_x >= previewMapX && mouse_x < (previewMapX + previewMapWidth) && (mouse_y >= previewMapY && mouse_y < (previewMapY + previewMapHeight))))
            {
                if (PreviewMap[iSkirmishMap].name[0] != '\0') {
                    if (PreviewMap[iSkirmishMap].terrain) {
                        draw_sprite(bmp_screen, PreviewMap[iSkirmishMap].terrain, previewMapX, previewMapY);
                    }
                }
            }
            else
            {
                if (PreviewMap[iSkirmishMap].name[0] != '\0') {
                    if (PreviewMap[iSkirmishMap].terrain) {
                        draw_sprite(bmp_screen, (BITMAP *)gfxinter[BMP_UNKNOWNMAP].dat, previewMapX, previewMapY);
                    }
                }
            }
        }
	}

	int widthOfSomething = 274; //??
	int startPointsX = screen_x - widthOfSomething;
	int startPointsY = previewMapY;
	int startPointHitBoxWidth = 130;
	int startPointHitBoxHeight = 16;

	textDrawer.drawTextWithOneInteger(startPointsX, startPointsY, "Startpoints: %d", iStartingPoints);

	bool bDoRandomMap=false;

	if ((mouse_x >= startPointsX && mouse_x <= (startPointsX + startPointHitBoxWidth)) && (mouse_y >= startPointsY && mouse_y <= (startPointsY + startPointHitBoxHeight)))
	{
        textDrawer.drawTextWithOneInteger(startPointsX, startPointsY, makecol(255, 0, 0), "Startpoints: %d", iStartingPoints);

		if (cMouse::isLeftButtonClicked())
		{
			iSkirmishStartPoints++;

			if (iSkirmishStartPoints > 4) {
				iSkirmishStartPoints = 2;
			}

			bDoRandomMap=true;
		}

		if (cMouse::isRightButtonClicked())
		{
			iSkirmishStartPoints--;

			if (iSkirmishStartPoints < 2) {
				iSkirmishStartPoints = 4;
			}

			bDoRandomMap=true;
		}
	}

	int const iHeightPixels=topBarHeight;

	int iDrawY=-1;
	int iDrawX=screen_x - widthOfSomething;
	int iEndX=screen_y;
	int iColor=makecol(255,255,255);

	// yes, this means higher resolutions can show more maps.. for now
	int maxMapsInList=std::min((mapListHeight / iHeightPixels), MAX_SKIRMISHMAPS);


    // for every map that we read , draw here
    for (int i=0; i < maxMapsInList; i++) {
		if (PreviewMap[i].name[0] != '\0')
		{
			bool bHover=false;

			iDrawY=mapListFrameY+(i*iHeightPixels)+i+iHeightPixels; // skip 1 bar because the 1st = 'random map'

            bHover = GUI_DRAW_FRAME(iDrawX, iDrawY, mapListFrameWidth, iHeightPixels);

            iColor=makecol(255,255,255);

			if (bHover)	{
			    // Mouse reaction
                iColor=makecol(255,0,0);

				if (cMouse::isLeftButtonClicked())
				{
                    GUI_DRAW_FRAME_PRESSED(iDrawX, iDrawY, mapListFrameWidth, iHeightPixels);
					iSkirmishMap=i;

					if (i == 0) {
						bDoRandomMap=true;
					}
				}
			}

			if (i == iSkirmishMap) {
				iColor=yellow;
				if (bHover) {
				    iColor = makecol(225, 177, 21); // a bit darker yellow to give some visual clue
				}
                GUI_DRAW_FRAME_PRESSED(iDrawX, iDrawY, mapListFrameWidth, iHeightPixels);
			}

			textDrawer.drawText(mapListFrameX + 4, iDrawY+4, iColor, PreviewMap[i].name);
		}
    }

	alfont_textprintf(bmp_screen, bene_font, 4, 26, makecol(0,0,0), "Player      House      Credits       Units    Team");
	alfont_textprintf(bmp_screen, bene_font, 4, 25, makecol(255,255,255), "Player      House      Credits       Units    Team");


	bool bHover=false;

	// draw players who will be playing ;)
	for (int p=0; p < (AI_WORM-1); p++)	{
		int iDrawY=playerListBarY + 4 +(p*22);
		if (p < iStartingPoints) {
			// player playing or not
            cAIPlayer &aiPlayer = aiplayer[p];
            if (p == HUMAN)	{
				alfont_textprintf(bmp_screen, bene_font, 4,iDrawY+1, makecol(0,0,0), "Human");
				alfont_textprintf(bmp_screen, bene_font, 4,iDrawY, makecol(255,255,255), "Human");
			} else {

				alfont_textprintf(bmp_screen, bene_font, 4,iDrawY+1, makecol(0,0,0), "  CPU");

				// move hovers over... :/
				if ((mouse_x >= 4 && mouse_x <= 73) && (mouse_y >= iDrawY && mouse_y <= (iDrawY+16))) {
					if (aiPlayer.bPlaying) {
                        alfont_textprintf(bmp_screen, bene_font, 4, iDrawY, makecol(fade_select, 0, 0), "  CPU");
                    } else {
					    // not available
                        alfont_textprintf(bmp_screen, bene_font, 4, iDrawY,
                                          makecol((fade_select / 2), (fade_select / 2), (fade_select / 2)), "  CPU");
                    }

					if (cMouse::isLeftButtonClicked())	{
						if (aiPlayer.bPlaying) {
                            aiPlayer.bPlaying = false;
                        } else {
                            aiPlayer.bPlaying = true;
                        }
					}
				}
				else
				{
					if (aiPlayer.bPlaying)
						alfont_textprintf(bmp_screen, bene_font, 4,iDrawY, makecol(255,255,255), "  CPU");
					else
						alfont_textprintf(bmp_screen, bene_font, 4,iDrawY, makecol(128,128,128), "  CPU");
				}
			}

			// HOUSE
			bHover=false;
			char cHouse[30];
			memset(cHouse, 0, sizeof(cHouse));

			if (player[p].getHouse() == ATREIDES) {
				sprintf(cHouse, "Atreides");
			} else if (player[p].getHouse() == HARKONNEN) {
				sprintf(cHouse, "Harkonnen");
			} else if (player[p].getHouse() == ORDOS) {
				sprintf(cHouse, "Ordos");
			} else if (player[p].getHouse() == SARDAUKAR) {
				sprintf(cHouse, "Sardaukar");
			} else {
				sprintf(cHouse, "Random");
			}

			alfont_textprintf(bmp_screen, bene_font, 74,iDrawY+1, makecol(0,0,0), "%s", cHouse);

			if ((mouse_x >= 74 && mouse_x <= 150) && (mouse_y >= iDrawY && mouse_y <= (iDrawY+16)))
				bHover=true;

			if (p == 0)
			{
				alfont_textprintf(bmp_screen, bene_font, 74,iDrawY, makecol(255,255,255), "%s", cHouse);
			}
			else
			{
				if (aiPlayer.bPlaying)
					alfont_textprintf(bmp_screen, bene_font, 74,iDrawY, makecol(255,255,255), "%s", cHouse);
				else
					alfont_textprintf(bmp_screen, bene_font, 74,iDrawY, makecol(128,128,128), "%s", cHouse);

			}

			if (bHover)
			{
				if (aiPlayer.bPlaying)
					alfont_textprintf(bmp_screen, bene_font, 74,iDrawY, makecol(fade_select,0,0), "%s", cHouse);
				else
					alfont_textprintf(bmp_screen, bene_font, 74,iDrawY, makecol((fade_select/2),(fade_select/2),(fade_select/2)), "%s", cHouse);


				if (cMouse::isLeftButtonClicked())
				{
					player[p].setHouse((player[p].getHouse()+1));
					if (p > 0)
					{
						if (player[p].getHouse() > 4) {
							player[p].setHouse(0);
						}
					}
					else
					{
						if (player[p].getHouse() > 3) {
							player[p].setHouse(0);
						}
					}
				}

				if (cMouse::isRightButtonClicked())
				{
					player[p].setHouse((player[p].getHouse()-1));
					if (p > 0)
					{
						if (player[p].getHouse() < 0) {
							player[p].setHouse(4);
						}
					}
					else
					{
						if (player[p].getHouse() < 0) {
							player[p].setHouse(3);
						}
					}
				}
			}

			// Credits
			bHover=false;

			alfont_textprintf(bmp_screen, bene_font, 174,iDrawY+1, makecol(0,0,0), "%d", (int)player[p].credits);

			//rect(bmp_screen, 174, iDrawY, 230, iDrawY+16, makecol(255,255,255));

			if ((mouse_x >= 174 && mouse_x <= 230) && (mouse_y >= iDrawY && mouse_y <= (iDrawY+16)))
				bHover=true;

			if (p == 0)
			{
				alfont_textprintf(bmp_screen, bene_font, 174,iDrawY, makecol(255,255,255), "%d", (int)player[p].credits);
			}
			else
			{
				if (aiPlayer.bPlaying)
					alfont_textprintf(bmp_screen, bene_font, 174,iDrawY, makecol(255,255,255), "%d", (int)player[p].credits);
				else
					alfont_textprintf(bmp_screen, bene_font, 174,iDrawY, makecol(128,128,128), "%d", (int)player[p].credits);

			}

			if (bHover)
			{
				if (aiPlayer.bPlaying)
					alfont_textprintf(bmp_screen, bene_font, 174,iDrawY, makecol(fade_select,0,0), "%d", (int)player[p].credits);
				else
					alfont_textprintf(bmp_screen, bene_font, 174,iDrawY, makecol((fade_select/2),(fade_select/2),(fade_select/2)), "%d", player[p].credits);

				if (cMouse::isLeftButtonClicked())
				{
					player[p].credits += 500;
					if (player[p].credits > 10000) {
						player[p].credits = 1000;
					}
				}

				if (cMouse::isRightButtonClicked())
				{
					player[p].credits -= 500;
					if (player[p].credits < 1000) {
						player[p].credits = 10000;
					}
				}
			}

			// Units
			bHover = false;

			alfont_textprintf(bmp_screen, bene_font, 269,iDrawY+1, makecol(0,0,0), "%d", aiPlayer.iUnits);

			//rect(bmp_screen, 269, iDrawY, 290, iDrawY+16, makecol(255,255,255));

			if ((mouse_x >= 269 && mouse_x <= 290) && (mouse_y >= iDrawY && mouse_y <= (iDrawY+16)))
				bHover=true;

			if (p == 0)
			{
				alfont_textprintf(bmp_screen, bene_font, 269, iDrawY, makecol(255,255,255), "%d", aiPlayer.iUnits);
			}
			else
			{
				if (aiPlayer.bPlaying)
					alfont_textprintf(bmp_screen, bene_font, 269, iDrawY, makecol(255,255,255), "%d", aiPlayer.iUnits);
				else
					alfont_textprintf(bmp_screen, bene_font, 269, iDrawY, makecol(128,128,128), "%d", aiPlayer.iUnits);

			}

			if (bHover)
			{
				if (aiPlayer.bPlaying)
					alfont_textprintf(bmp_screen, bene_font, 269, iDrawY, makecol(fade_select,0,0), "%d", aiPlayer.iUnits);
				else
					alfont_textprintf(bmp_screen, bene_font, 269, iDrawY, makecol((fade_select/2),(fade_select/2),(fade_select/2)), "%d", aiPlayer.iUnits);

				if (cMouse::isLeftButtonClicked())
				{
					aiPlayer.iUnits++;
					if (aiPlayer.iUnits > 10) {
                        aiPlayer.iUnits = 1;
					}
				}

				if (cMouse::isRightButtonClicked())
				{
					aiPlayer.iUnits--;
					if (aiPlayer.iUnits < 1) {
                        aiPlayer.iUnits = 10;
					}
				}
			}

			// Team
			bHover=false;
		}
	}

    GUI_DRAW_FRAME(-1, screen_y - topBarHeight, screen_x + 2, topBarHeight + 2);


	// back
	int backButtonWidth = textDrawer.textLength(" BACK");
    int backButtonHeight = topBarHeight;
	int backButtonY = screen_y - topBarHeight;
	int backButtonX = 0;
    textDrawer.drawTextBottomLeft(" BACK");

    // start
    int startButtonWidth = textDrawer.textLength("START");
    int startButtonHeight = topBarHeight;
    int startButtonY = screen_y - topBarHeight;
    int startButtonX = screen_x - startButtonWidth;

    textDrawer.drawTextBottomRight("START");

	if (bDoRandomMap) {
		randomMapGenerator.generateRandomMap();
	}

    // back
    if (MOUSE_WITHIN_RECT(backButtonX, backButtonY, backButtonWidth, backButtonHeight)) {
        textDrawer.drawTextBottomLeft(makecol(255,0,0), " BACK");

        if (cMouse::isLeftButtonClicked()) {
            bFadeOut=true;
            state = GAME_MENU;
        }
    }

    if (MOUSE_WITHIN_RECT(startButtonX, startButtonY, startButtonWidth, startButtonHeight)) {
        textDrawer.drawTextBottomRight(makecol(255, 0, 0), "START");

        // this needs to be before setup_players :/
        iMission=9; // high tech level (TODO: make this customizable)

        game.setup_players();

        // START
        if ((cMouse::isLeftButtonClicked() && iSkirmishMap > -1)) {
            cCellCalculator *cellCalculator = new cCellCalculator(&map);
            // Starting skirmish mode
            bSkirmish=true;

            /* set up starting positions */
            std::vector<int> iStartPositions;

            int startCellsOnSkirmishMap=0;
            for (int s=0; s < 5; s++) {
                int startPosition = PreviewMap[iSkirmishMap].iStartCell[s];
                if (startPosition < 0) continue;
                iStartPositions.push_back(startPosition);
            }

            startCellsOnSkirmishMap = iStartPositions.size();

            // REGENERATE MAP DATA FROM INFO
            for (int c=0; c < MAX_CELLS; c++) {
                mapEditor.createCell(c, PreviewMap[iSkirmishMap].mapdata[c], 0);
            }

            mapEditor.smoothMap();

            if (DEBUGGING) {
                logbook("Starting positions before shuffling:");
                for (int i = 0; i < startCellsOnSkirmishMap; i++) {
                    char msg[255];
                    sprintf(msg, "iStartPositions[%d] = [%d]", i, iStartPositions[i]);
                    logbook(msg);
                }
            }

            logbook("Shuffling starting positions");
            std::random_shuffle(iStartPositions.begin(), iStartPositions.end());

            if (DEBUGGING) {
                logbook("Starting positions after shuffling:");
                for (int i = 0; i < startCellsOnSkirmishMap; i++) {
                    char msg[255];
                    sprintf(msg, "iStartPositions[%d] = [%d]", i, iStartPositions[i]);
                    logbook(msg);
                }
            }

            // set up players and their units
            for (int p=0; p < AI_WORM; p++)	{

                cPlayer &cPlayer = player[p];
                int iHouse = cPlayer.getHouse();

                // house = 0 means pick random house
                if (iHouse==0 && p < 4) { // (all players above 4 are non-playing AI 'sides'
                    bool bOk=false;

                    while (bOk == false) {
                        if (p > HUMAN) // cpu player
                            iHouse = rnd(4)+1;
                        else // human may not be sardaukar
                            iHouse = rnd(3)+1; // hark = 1, atr = 2, ord = 3, sar = 4

                        bool bFound=false;
                        for (int pl=0; pl < AI_WORM; pl++) {
                            if (player[pl].getHouse() > 0 && player[pl].getHouse() == iHouse) {
                                bFound=true;
                            }
                        }

                        if (!bFound) {
                            bOk=true;
                        }
                    }
                }

                if (p == 5) {
                    iHouse = FREMEN;
                }

                cPlayer.setHouse(iHouse);

                // not playing.. do nothing
                if (aiplayer[p].bPlaying == false) {
                    continue;
                }

                // set credits
                cPlayer.focus_cell = iStartPositions[p];

                // Set map position
                if (p == HUMAN) {
                    mapCamera->centerAndJumpViewPortToCell(cPlayer.focus_cell);
                }

                // create constyard
                cAbstractStructure *s = cStructureFactory::getInstance()->createStructure(cPlayer.focus_cell, CONSTYARD, p);

                // when failure, create mcv instead
                if (s == NULL) {
                    UNIT_CREATE(cPlayer.focus_cell, MCV, p, true);
                }

                // amount of units
                int u=0;

                // create units
                while (u < aiplayer[p].iUnits) {
                    int iX=iCellGiveX(cPlayer.focus_cell);
                    int iY=iCellGiveY(cPlayer.focus_cell);
                    int iType=rnd(12);

                    iX-=4;
                    iY-=4;
                    iX+=rnd(9);
                    iY+=rnd(9);

                    // convert house specific stuff
                    if (cPlayer.getHouse() == ATREIDES) {
                        if (iType == DEVASTATOR || iType == DEVIATOR) {
                            iType = SONICTANK;
                        }

                        if (iType == TROOPERS) {
                            iType = INFANTRY;
                        }

                        if (iType == TROOPER) {
                            iType = SOLDIER;
                        }

                        if (iType == RAIDER) {
                            iType = TRIKE;
                        }
                    }

                    // ordos
                    if (cPlayer.getHouse() == ORDOS) {
                        if (iType == DEVASTATOR || iType == SONICTANK) {
                            iType = DEVIATOR;
                        }

                        if (iType == TRIKE) {
                            iType = RAIDER;
                        }
                    }

                    // harkonnen
                    if (cPlayer.getHouse() == HARKONNEN) {
                        if (iType == DEVIATOR || iType == SONICTANK) {
                            iType = DEVASTATOR;
                        }

                        if (iType == TRIKE || iType == RAIDER) {
                            iType = QUAD;
                        }

                        if (iType == SOLDIER) {
                            iType = TROOPER;
                        }

                        if (iType == INFANTRY) {
                            iType = TROOPERS;
                        }
                    }

                    int cell = cellCalculator->getCellWithMapBorders(iX, iY);
                    int r = UNIT_CREATE(cell, iType, p, true);
                    if (r > -1)
                    {
                        u++;
                    }
                }

                char msg[255];
                sprintf(msg,"Wants %d amount of units; amount created %d", aiplayer[p].iUnits, u);
                cLogger::getInstance()->log(LOG_TRACE, COMP_SKIRMISHSETUP, "Creating units", msg, OUTC_NONE, p, iHouse);
            }

            bFadeOut=true;
            playMusicByType(MUSIC_PEACE);

            // TODO: spawn a few worms
            iHouse=player[HUMAN].getHouse();
            state = GAME_PLAYING;
            drawManager->getMessageDrawer()->initCombatPosition();

            // delete cell calculator
            delete cellCalculator;
        } // mouse clicks on START (and skirmish map is selected)
    } // mouse hovers over "START"

   	// MOUSE
    draw_sprite(bmp_screen, (BITMAP *)gfxdata[mouse_tile].dat, mouse_x, mouse_y);

    if (bFadeOut) {
        game.FADE_OUT();
    }
}

// select house
void cGame::stateSelectHouse() {
    // FADING STUFF
    if (iFadeAction == 1) // fading out
    {
        draw_sprite(bmp_screen, bmp_fadeout, 0, 0);
        return;
    }

    if (iAlphaScreen == 0)
        iFadeAction = 2;
    // -----------------

    bool bFadeOut=false;

	// Render the planet Dune a bit downward
    BITMAP *duneBitmap = (BITMAP *) gfxinter[BMP_GAME_DUNE].dat;
    draw_sprite(bmp_screen, duneBitmap, ((game.screen_x - duneBitmap->w)), ((game.screen_y - (duneBitmap->h * 0.90))));

	// HOUSES
    BITMAP *sprite = (BITMAP *) gfxinter[BMP_SELECT_YOUR_HOUSE].dat;
    int selectYourHouseXCentered = (game.screen_x / 2) - sprite->w / 2;
    draw_sprite(bmp_screen, sprite, selectYourHouseXCentered, 0);

    int selectYourHouseY = game.screen_y * 0.35f;

    int columnWidth = game.screen_x / 7; // empty, atr, empty, har, empty, ord, empty (7 columns)
    int offset = (columnWidth / 2) - (((BITMAP *)gfxinter[BMP_SELECT_HOUSE_ATREIDES].dat)->w / 2);
    cRectangle houseAtreides = cRectangle((columnWidth * 1) + offset, selectYourHouseY, 90, 98);
    cRectangle houseOrdos = cRectangle((columnWidth * 3) + offset, selectYourHouseY, 90, 98);
    cRectangle houseHarkonnen = cRectangle((columnWidth * 5) + offset, selectYourHouseY, 90, 98);
    allegroDrawer->blitSprite((BITMAP *)gfxinter[BMP_SELECT_HOUSE_ATREIDES].dat, bmp_screen, &houseAtreides);
    allegroDrawer->blitSprite((BITMAP *)gfxinter[BMP_SELECT_HOUSE_ORDOS].dat, bmp_screen, &houseOrdos);
    allegroDrawer->blitSprite((BITMAP *)gfxinter[BMP_SELECT_HOUSE_HARKONNEN].dat, bmp_screen, &houseHarkonnen);

    cTextDrawer textDrawer = cTextDrawer(bene_font);

    // back
    cRectangle *backButtonRect = textDrawer.getAsRectangle(0, screen_y - textDrawer.getFontHeight(), " BACK");
    textDrawer.drawText(backButtonRect->getX(), backButtonRect->getY(), makecol(255,255,255), " BACK");

    if (backButtonRect->isMouseOver()) {
        textDrawer.drawText(backButtonRect->getX(), backButtonRect->getY(), makecol(255, 0, 0), " BACK");
    }

    if (cMouse::isLeftButtonClicked()) {
        if (cMouse::isOverRectangle(&houseAtreides)) {
            iHouse=ATREIDES;

            play_sound_id(SOUND_ATREIDES);

            LOAD_SCENE("platr"); // load planet of atreides

            state = GAME_TELLHOUSE;
            iMentatSpeak=-1;
            bFadeOut=true;
        } else if (cMouse::isOverRectangle(&houseOrdos)) {
            iHouse=ORDOS;

            play_sound_id(SOUND_ORDOS);

            LOAD_SCENE("plord"); // load planet of ordos

            state = GAME_TELLHOUSE;
            iMentatSpeak=-1;
            bFadeOut=true;
        } else if (cMouse::isOverRectangle(&houseHarkonnen)) {
            iHouse=HARKONNEN;

            play_sound_id(SOUND_HARKONNEN);

            LOAD_SCENE("plhar"); // load planet of harkonnen

            state = GAME_TELLHOUSE;
            iMentatSpeak=-1;
            bFadeOut=true;
        } else if (backButtonRect->isMouseOver()) {
            bFadeOut=true;
            state = GAME_MENU;
        }
    }

    delete backButtonRect;

	// MOUSE
    draw_sprite(bmp_screen, (BITMAP *)gfxdata[mouse_tile].dat, mouse_x, mouse_y);

    if (bFadeOut) {
        game.FADE_OUT();
    }
}



void cGame::preparementat(bool bTellHouse)
{
    // clear first
    memset(mentat_sentence, 0, sizeof(mentat_sentence));

	if (bTellHouse)
	{
		if (iHouse == ATREIDES)
		{
            INI_LOAD_BRIEFING(ATREIDES, 0, INI_DESCRIPTION);
            //LOAD_BRIEFING("atreides.txt");
		}
		else if (iHouse == HARKONNEN)
		{
            INI_LOAD_BRIEFING(HARKONNEN, 0, INI_DESCRIPTION);
            //LOAD_BRIEFING("harkonnen.txt");
		}
		else if (iHouse == ORDOS)
		{
            INI_LOAD_BRIEFING(ORDOS, 0, INI_DESCRIPTION);
            //LOAD_BRIEFING("ordos.txt");
		}
	}
	else
	{
        if (state == GAME_BRIEFING)
        {
        	game.setup_players();
			INI_Load_scenario(iHouse, iRegion);
			INI_LOAD_BRIEFING(iHouse, iRegion, INI_BRIEFING);
        }
        else if (state == GAME_WINBRIEF)
        {
            if (rnd(100) < 50)
                LOAD_SCENE("win01"); // ltank
            else
                LOAD_SCENE("win02"); // ltank

            INI_LOAD_BRIEFING(iHouse, iRegion, INI_WIN);
        }
        else if (state == GAME_LOSEBRIEF)
        {
            if (rnd(100) < 50)
                LOAD_SCENE("lose01"); // ltank
            else
                LOAD_SCENE("lose02"); // ltank

            INI_LOAD_BRIEFING(iHouse, iRegion, INI_LOSE);
        }
    }

	logbook("MENTAT: sentences prepared 1");
	iMentatSpeak=-2;			// = sentence to draw and speak with (-1 = not ready, -2 means starting)
	TIMER_mentat_Speaking=0; //	0 means, set it up
}

void cGame::tellhouse()
{
    // FADING

    if (iFadeAction == 1) // fading out
    {
        draw_sprite(bmp_screen, bmp_fadeout, 0, 0);
        return;
    }

    if (iAlphaScreen == 0)
        iFadeAction = 2;
    // -----------------

    bool bFadeOut=false;

	draw_mentat(-1); // draw benegesserit

	// -1 means prepare
	if (iMentatSpeak == -1)
		preparementat(true); // prepare for house telling
	else if (iMentatSpeak > -1)
	{
		mentat(-1); // speak dammit!
	}
	else if (iMentatSpeak == -2)
	{
		// do you wish to , bla bla?
	}

    // draw buttons

    if (TIMER_mentat_Speaking < 0)
    {
        // NO
        draw_sprite(bmp_screen, (BITMAP *)gfxmentat[BTN_NO].dat, 293, 423);

        if ((mouse_x > 293 && mouse_x < 446) && (mouse_y > 423 && mouse_y < 468))
            if (cMouse::isLeftButtonClicked())
            {
                // head back to choose house
                iHouse=-1;
                state = GAME_SELECT_HOUSE;
                bFadeOut=true;
            }

        // YES
        draw_sprite(bmp_screen, (BITMAP *)gfxmentat[BTN_YES].dat, 466, 423);
        if ((mouse_x > 446 && mouse_x < 619) && (mouse_y >423 && mouse_y < 468))
            if (cMouse::isLeftButtonClicked())
            {
                // yes!
                state = GAME_BRIEFING; // briefing
                iMission = 1;
                iRegion  = 1;
                iMentatSpeak=-1; // prepare speaking

                player[0].setHouse(iHouse);

                // play correct mentat music
                playMusicByType(MUSIC_BRIEFING);
                bFadeOut=true;

            }


    }

    // draw mouse
    draw_sprite(bmp_screen, (BITMAP *)gfxdata[mouse_tile].dat, mouse_x, mouse_y);

    if (bFadeOut)
        FADE_OUT();

}

// select your next conquest
void cGame::region() {
	int mouse_tile = MOUSE_NORMAL;

    if (iFadeAction == 1) // fading out
    {
        draw_sprite(bmp_screen, bmp_fadeout, 0, 0);
        return;
    }

    if (iAlphaScreen == 0)
        iFadeAction = 2;
    // -----------------

    bool bFadeOut=false;

    // STEPS:
    // 1. Show current conquered regions
    // 2. Show next progress + story (in message bar)
    // 3. Click next region
    // 4. Set up region and go to GAME_BRIEFING, which will do the rest...-> fade out

    select_palette(player[0].pal);

    // region = one we have won, only changing after we have chosen this one
    if (iRegionState <= 0)
        iRegionState = 1;

    if (iRegionSceneAlpha > 255)
        iRegionSceneAlpha = 255;

    // tell the story
    if (iRegionState == 1) {
        // depending on the mission, we tell the story
        if (iRegionScene == 0)
        {
            REGION_SETUP(iMission, iHouse);
            iRegionScene++;
            drawManager->getMessageDrawer()->setMessage("3 Houses have come to Dune.");
            iRegionSceneAlpha=-5;
        }
        else if (iRegionScene == 1)
        {
            // draw the
            set_trans_blender(0,0,0,iRegionSceneAlpha);
            draw_trans_sprite(bmp_screen, (BITMAP *)gfxinter[BMP_GAME_DUNE].dat, 0, 12);
            char * cMessage = drawManager->getMessageDrawer()->getMessage();
            if (cMessage[0] == '\0' && iRegionSceneAlpha >= 255)
            {
            	drawManager->getMessageDrawer()->setMessage("To take control of the land.");
                iRegionScene++;
                iRegionSceneAlpha=-5;
            }
        }
        else if (iRegionScene == 2)
        {
            draw_sprite(bmp_screen, (BITMAP *)gfxinter[BMP_GAME_DUNE].dat, 0, 12);
            set_trans_blender(0,0,0,iRegionSceneAlpha);
            draw_trans_sprite(bmp_screen, (BITMAP *)gfxworld[WORLD_DUNE].dat, 16, 73);
            char * cMessage = drawManager->getMessageDrawer()->getMessage();
            if (cMessage[0] == '\0' && iRegionSceneAlpha >= 255)
            {
            	drawManager->getMessageDrawer()->setMessage("That has become divided.");
                iRegionScene++;
                iRegionSceneAlpha=-5;
            }
        }
        else if (iRegionScene == 3) {
            draw_sprite(bmp_screen, (BITMAP *)gfxworld[WORLD_DUNE].dat, 16, 73);
            set_trans_blender(0,0,0,iRegionSceneAlpha);
            draw_trans_sprite(bmp_screen, (BITMAP *)gfxworld[WORLD_DUNE_REGIONS].dat, 16, 73);

            if (iRegionSceneAlpha >= 255) {
                iRegionScene = 4;
                iRegionState++;
            }
        }
        else if (iRegionScene > 3)
            iRegionState++;

        if (iRegionSceneAlpha < 255) {
			iRegionSceneAlpha+=5;
		}

        // when  mission is 1, do the '3 houses has come to dune, blah blah story)
    }

    // Draw
    draw_sprite(bmp_screen, (BITMAP *)gfxworld[BMP_NEXTCONQ].dat, 0, 0);

    int iLogo=-1;

    // Draw your logo
    if (iHouse == ATREIDES)
        iLogo = BMP_NCATR;

    if (iHouse == ORDOS)
        iLogo=BMP_NCORD;

    if (iHouse == HARKONNEN)
        iLogo=BMP_NCHAR;

    // Draw 4 times the logo (in each corner)
    if (iLogo > -1) {
	    draw_sprite(bmp_screen, (BITMAP *)gfxworld[iLogo].dat, 0,0);
	    draw_sprite(bmp_screen, (BITMAP *)gfxworld[iLogo].dat, (640)-64,0);
	    draw_sprite(bmp_screen, (BITMAP *)gfxworld[iLogo].dat, 0,(480)-64);
	    draw_sprite(bmp_screen, (BITMAP *)gfxworld[iLogo].dat, (640)-64,(480)-64);
    }

    char * cMessage = drawManager->getMessageDrawer()->getMessage();
    if (iRegionState == 2) {
        // draw dune first
        draw_sprite(bmp_screen, (BITMAP *)gfxworld[WORLD_DUNE].dat, 16, 73);
        draw_sprite(bmp_screen, (BITMAP *)gfxworld[WORLD_DUNE_REGIONS].dat, 16, 73);

        // draw here stuff
        for (int i=0; i < 27; i++)
            REGION_DRAW(world[i]);

        // Animate here (so add regions that are conquered)
        char * cMessage = drawManager->getMessageDrawer()->getMessage();

        bool bDone=true;
        for (int i=0; i < MAX_REGIONS; i++)
        {
            // anything in the list
            if (iRegionConquer[i] > -1)
            {
                int iRegNr = iRegionConquer[i];

                if (iRegionHouse[i] > -1)
                {
                    // when the region is NOT this house, turn it into this house
                    if (world[iRegNr].iHouse != iRegionHouse[i])
                    {

                        if ((cRegionText[i][0] != '\0' && cMessage[0] == '\0') ||
                            (cRegionText[i][0] == '\0'))
                        {

                        // set this up
                        world[iRegNr].iHouse = iRegionHouse[i];
                        world[iRegNr].iAlpha = 1;

                        if (cRegionText[i][0] != '\0') {
                        	drawManager->getMessageDrawer()->setMessage(cRegionText[i]);
                        }

                        bDone=false;
                        break;

                        }
                        else
                        {
                            bDone=false;
                            break;

                        }
                    }
                    else
                    {
                        // house = ok
                        if (world[iRegNr].iAlpha >= 255)
                        {
                            // remove from list
                            iRegionConquer[i] = -1;
                            iRegionHouse[i] = -1; //
                            bDone=false;

                            break;
                        }
                        else if (world[iRegNr].iAlpha < 255)
                        {
                            bDone=false;
                            break; // not done yet, so wait before we do another region!
                        }
                    }
                }
            }
        }

        if (bDone && cMessage[0] == '\0') {
            iRegionState++;
            drawManager->getMessageDrawer()->setMessage("Select your next region.");
        }
    } else if (iRegionState == 3) {

        // draw dune first
        draw_sprite(bmp_screen, (BITMAP *)gfxworld[WORLD_DUNE].dat, 16, 73);
        draw_sprite(bmp_screen, (BITMAP *)gfxworld[WORLD_DUNE_REGIONS].dat, 16, 73);

        // draw here stuff
        for (int i=0; i < 27; i++)
            REGION_DRAW(world[i]);

        int r = REGION_OVER();

        bool bClickable=false;

        if (r > -1)
            if (world[r].bSelectable)
            {
                    world[r].iAlpha = 255;
                    mouse_tile = MOUSE_ATTACK;
                    bClickable=true;
            }

        if (cMouse::isLeftButtonClicked() && bClickable)
        {
            // selected....
            int iReg=0;
            for (int ir=0; ir < MAX_REGIONS; ir++)
                if (world[ir].bSelectable)
                    if (ir != r)
                        iReg++;
                    else
                        break;

            // calculate region stuff, and add it up
            int iNewReg=0;
            if (iMission == 0)	iNewReg=1;
            if (iMission == 1)	iNewReg=2;
            if (iMission == 2)	iNewReg=5;
            if (iMission == 3)	iNewReg=8;
            if (iMission == 4)	iNewReg=11;
            if (iMission == 5)	iNewReg=14;
            if (iMission == 6)	iNewReg=17;
            if (iMission == 7)	iNewReg=20;
            if (iMission == 8)	iNewReg=22;

            iNewReg += iReg;

            //char msg[255];
            //sprintf(msg, "Mission = %d", game.iMission);
            //allegro_message(msg);

            game.mission_init();
            game.iRegionState=0;
            game.state = GAME_BRIEFING;
            game.iRegion = iNewReg;
            game.iMission++;						// FINALLY ADD MISSION NUMBER...
            //    iRegion++;
            iMentatSpeak=-1;

            INI_Load_scenario(iHouse, game.iRegion);

            //sprintf(msg, "Mission = %d", game.iMission);
            //allegro_message(msg);

            playMusicByType(MUSIC_BRIEFING);

            //allegro_message(msg);

            bFadeOut=true;

            // Calculate mission from region:
            // region 1 = mission 1
            // region 2, 3, 4 = mission 2
            // region 5, 6, 7 = mission 3
            // region 8, 9, 10 = mission 4
            // region 11,12,13 = mission 5
            // region 14,15,16 = mission 6
            // region 17,18,19 = mission 7
            // region 20,21    = mission 8
            // region 22 = mission 9

        }
    }

    // draw message
    // TODO: Use own message drawer here or something, with other coordinates
	drawManager->getMessageDrawer()->draw();
 //    if (iMessageAlpha > -1)
	// {
	// 	set_trans_blender(0,0,0,iMessageAlpha);
	// 	BITMAP *temp = create_bitmap(480,30);
	// 	clear_bitmap(temp);
	// 	rectfill(temp, 0,0,480,40, makecol(255,0,255));
	// 	//draw_sprite(temp, (BITMAP *)gfxinter[BMP_MESSAGEBAR].dat, 0,0);
 //
	// 	// draw message
	// 	alfont_textprintf(temp, game_font, 13,18, makecol(0,0,0), cMessage);
 //
	// 	// draw temp
	// 	draw_trans_sprite(bmp_screen, temp, 73, 358);
 //
	// 	destroy_bitmap(temp);
	// }


    // mouse
    if (mouse_tile == MOUSE_ATTACK)
        draw_sprite(bmp_screen, (BITMAP *)gfxdata[mouse_tile].dat, mouse_x-16, mouse_y-16);
    else
        draw_sprite(bmp_screen, (BITMAP *)gfxdata[mouse_tile].dat, mouse_x, mouse_y);

    if (bFadeOut)
        FADE_OUT();

	vsync();

}

void cGame::destroyAllUnits(bool bHumanPlayer) {
	if (bHumanPlayer) {
		for (int i=0; i < MAX_UNITS; i++) {
			if (unit[i].isValid()) {
				if (unit[i].iPlayer == 0) {
					unit[i].die(true, false); {
					}
				}
			}
		}
	} else {
		for (int i=0; i < MAX_UNITS; i++) {
			if (unit[i].isValid()) {
				if (unit[i].iPlayer > 0) {
					unit[i].die(true, false);
				}
			}
		}
	}
}

void cGame::destroyAllStructures(bool bHumanPlayer) {
	if (bHumanPlayer) {
		for (int i=0; i < MAX_STRUCTURES; i++) {
			if (structure[i]) {
				if (structure[i]->getOwner() == 0) {
					structure[i]->die();
				}
			}
		}
	} else {
		for (int i=0; i < MAX_STRUCTURES; i++) {
			if (structure[i]) {
				if (structure[i]->getOwner() > 0) {
					structure[i]->die();
				}
			}
		}
	}
}

int cGame::getGroupNumberFromKeyboard() {
	if (key[KEY_1]) {
		return 1;
	}
	if (key[KEY_2]) {
		return 2;
	}
	if (key[KEY_3]) {
		return 3;
	}
	if (key[KEY_4]) {
		return 4;
	}
	if (key[KEY_5]) {
		return 5;
	}
	return 0;
}

void cGame::handleTimeSlicing() {
	if (iRest > 0) {
		rest(iRest);
	}
}

void cGame::shakeScreenAndBlitBuffer() {
    if (TIMER_shake == 0) {
		TIMER_shake = -1;
	}
	// blitSprite on screen

	if (TIMER_shake > 0)
	{
		// the more we get to the 'end' the less we 'throttle'.
		// Structure explosions are 6 time units per cell.
		// Max is 9 cells (9*6=54)
		// the max border is then 9. So, we do time / 6
		if (TIMER_shake > 69) TIMER_shake = 69;

		int offset = TIMER_shake / 5;
		if (offset > 9)
			offset = 9;

		shake_x = -abs(offset/2) + rnd(offset);
		shake_y = -abs(offset/2) + rnd(offset);

		blit(bmp_screen, bmp_throttle, 0, 0, 0+shake_x, 0+shake_y, screen_x, screen_y);
		blit(bmp_throttle, screen, 0, 0, 0, 0, screen_x, screen_y);
	}
	else
	{
		// when fading
		if (iAlphaScreen == 255)
			blit(bmp_screen, screen, 0, 0, 0, 0, screen_x, screen_y);
		else
		{
			BITMAP *temp = create_bitmap(game.screen_x, game.screen_y);
			assert(temp != NULL);
			clear(temp);
			fblend_trans(bmp_screen, temp, 0, 0, iAlphaScreen);
			blit(temp, screen, 0, 0, 0, 0, screen_x, screen_y);
			destroy_bitmap(temp);
		}
	}
}

void cGame::runGameState() {
    switch (state) {
		case GAME_PLAYING:
			combat();
			break;
		case GAME_BRIEFING:
			if (iMentatSpeak == -1) {
				preparementat(false);
			}
			mentat(iHouse);
			break;
		case GAME_SETUPSKIRMISH:
			setup_skirmish();
			break;
		case GAME_MENU:
			menu();
			break;
		case GAME_REGION:
			region();
			break;
		case GAME_SELECT_HOUSE:
            stateSelectHouse();
			break;
		case GAME_TELLHOUSE:
			tellhouse();
			break;
		case GAME_WINNING:
			winning();
			break;
		case GAME_LOSING:
			losing();
			break;
		case GAME_WINBRIEF:
			if (iMentatSpeak == -1) {
				preparementat(false);
			}
			mentat(iHouse);
			break;
		case GAME_LOSEBRIEF:
			if (iMentatSpeak == -1) {
				preparementat(false);
			}
			mentat(iHouse);
			break;
	}
}

/**
	Main game loop
*/
void cGame::run() {
	set_trans_blender(0, 0, 0, 128);

	while (bPlaying) {
		TimeManager.processTime();
		poll();
		handleTimeSlicing();
		runGameState();
		interactionManager->interactWithKeyboard();
		shakeScreenAndBlitBuffer();
		frame_count++;
	}
}


/**
	Shutdown the game
*/
void cGame::shutdown() {
	cLogger *logger = cLogger::getInstance();
	logger->logHeader("SHUTDOWN");

	if (soundPlayer) {
        soundPlayer->destroyAllSounds();
        delete soundPlayer;
    }

    if (mapViewport) {
        delete mapViewport;
    }

    if (drawManager) {
        delete drawManager;
    }

    if (mapCamera) {
        delete mapCamera;
    }

    if (mapUtils) {
        delete mapUtils;
    }

    if (interactionManager) {
        delete interactionManager;
    }

    cStructureFactory::destroy();
    cSideBarFactory::destroy();
    cBuildingListFactory::destroy();

    for (int i = 0; i < MAX_PLAYERS; i++) {
        player[i].destroyAllegroBitmaps();
    }

    delete allegroDrawer;
    cMouse::destroy();

    if (gfxdata) {
        unload_datafile(gfxdata);
    }
    if (gfxinter) {
        unload_datafile(gfxinter);
    }
    if (gfxworld) {
        unload_datafile(gfxworld);
    }
    if (gfxmentat) {
        unload_datafile(gfxmentat);
    }
    if (gfxmovie) {
        unload_datafile(gfxmovie);
    }

    // Destroy font of Allegro FONT library
	alfont_destroy_font(game_font);
	alfont_destroy_font(bene_font);

	// Exit the font library (must be first)
	alfont_exit();

	logbook("Allegro FONT library shut down.");

	// MP3 Library
	if (mp3_music) {
		almp3_stop_autopoll_mp3(mp3_music); // stop auto poll
		almp3_destroy_mp3(mp3_music);
	}

	logbook("Allegro MP3 library shut down.");

	// Now we are all neatly closed, we exit Allegro and return to OS.
	allegro_exit();

	logbook("Allegro shut down.");
	logbook("Thanks for playing.");

    cLogger::destroy();
}

bool cGame::isResolutionInGameINIFoundAndSet() {
    return game.ini_screen_height != -1 && game.ini_screen_width != -1;
}

void cGame::setScreenResolutionFromGameIniSettings() {
    if (game.ini_screen_width < 800) {
        game.ini_screen_width = 800;
        logbook("INI screen width < 800; unsupported; will set to 800.");
    }
    if (game.ini_screen_height < 600) {
        game.ini_screen_height = 600;
        logbook("INI screen height < 600; unsupported; will set to 600.");
    }
    game.screen_x = game.ini_screen_width;
    game.screen_y = game.ini_screen_height;
    char msg[255];
    sprintf(msg, "Resolution %dx%d loaded from ini file.", game.ini_screen_width, game.ini_screen_height);
    cLogger::getInstance()->log(LOG_INFO, COMP_ALLEGRO, "Resolution from ini file", msg);
}

/**
	Setup the game

	Should not be called twice.
*/
bool cGame::setupGame() {
	cLogger *logger = cLogger::getInstance();

	game.init(); // Must be first!

	logger->clearLogFile();

	logger->logHeader("Dune II - The Maker");
	logger->logCommentLine(""); // whitespace

	logger->logHeader("Version information");
	char msg[255];
	sprintf(msg, "Version %s, Compiled at %s , %s", game.version, __DATE__, __TIME__);
	logger->log(LOG_INFO, COMP_VERSION, "Initializing", msg);

	// init game
	if (game.windowed) {
		logger->log(LOG_INFO, COMP_SETUP, "Initializing", "Windowed mode");
	} else {
		logger->log(LOG_INFO, COMP_SETUP, "Initializing", "Fullscreen mode");
	}

	// TODO: load eventual game settings (resolution, etc)

	// Logbook notification
	logger->logHeader("Allegro");

	// ALLEGRO - INIT
	if (allegro_init() != 0) {
		logger->log(LOG_FATAL, COMP_ALLEGRO, "Allegro init", allegro_id, OUTC_FAILED);
		return false;
	}

    allegroDrawer = new cAllegroDrawer();

	logger->log(LOG_INFO, COMP_ALLEGRO, "Allegro init", allegro_id, OUTC_SUCCESS);

	int r = install_timer();
	if (r > -1) {
		logger->log(LOG_INFO, COMP_ALLEGRO, "Initializing timer functions", "install_timer()", OUTC_SUCCESS);
	}
	else
	{
		allegro_message("Failed to install timer");
		logger->log(LOG_FATAL, COMP_ALLEGRO, "Initializing timer functions", "install_timer()", OUTC_FAILED);
		return false;
	}

	alfont_init();
	logger->log(LOG_INFO, COMP_ALLEGRO, "Initializing ALFONT", "alfont_init()", OUTC_SUCCESS);
	install_keyboard();
	logger->log(LOG_INFO, COMP_ALLEGRO, "Initializing Allegro Keyboard", "install_keyboard()", OUTC_SUCCESS);
	install_mouse();
	cMouse::init();
	logger->log(LOG_INFO, COMP_ALLEGRO, "Initializing Allegro Mouse", "install_mouse()", OUTC_SUCCESS);

	/* set up the interrupt routines... */
	game.TIMER_shake=0;

	LOCK_VARIABLE(allegro_timerUnits);
	LOCK_VARIABLE(allegro_timerGlobal);
	LOCK_VARIABLE(allegro_timerSecond);

	LOCK_FUNCTION(allegro_timerunits);
	LOCK_FUNCTION(allegro_timerglobal);
	LOCK_FUNCTION(allegro_timerfps);

	// Install timers
	install_int(allegro_timerunits, 100); // 100 miliseconds
	install_int(allegro_timerglobal, 5); // 5 miliseconds
	install_int(allegro_timerfps, 1000); // 1000 miliseconds (seconds)

	logger->log(LOG_INFO, COMP_ALLEGRO, "Set up timer related variables", "LOCK_VARIABLE/LOCK_FUNCTION", OUTC_SUCCESS);

	frame_count = fps = 0;

	// set window title
	char title[128];
	sprintf(title, "Dune II - The Maker [%s] - (by Stefan Hendriks)", game.version);

	// Set window title
	set_window_title(title);
	logger->log(LOG_INFO, COMP_ALLEGRO, "Set up window title", title, OUTC_SUCCESS);

    int colorDepth = desktop_color_depth();
    set_color_depth(colorDepth);

    char colorDepthMsg[255];
    sprintf(colorDepthMsg,"Desktop color dept is %d.", colorDepth);
    cLogger::getInstance()->log(LOG_INFO, COMP_ALLEGRO, "Analyzing desktop color depth.", colorDepthMsg);


	// TODO: read/write rest value so it does not have to 'fine-tune'
	// but is already set up. Perhaps even offer it in the options screen? So the user
	// can specify how much CPU this game may use?

	if (game.windowed) {
		cLogger::getInstance()->log(LOG_INFO, COMP_ALLEGRO, "Windowed mode requested.", "");

		if (isResolutionInGameINIFoundAndSet()) {
			setScreenResolutionFromGameIniSettings();
		}

        r = set_gfx_mode(GFX_AUTODETECT_WINDOWED, game.screen_x, game.screen_y, game.screen_x, game.screen_y);

		char msg[255];
		sprintf(msg, "Initializing graphics mode (windowed) with resolution %d by %d, colorDepth %d.", game.screen_x, game.screen_y, colorDepth);

		if (r > -1) {
			logger->log(LOG_INFO, COMP_ALLEGRO, msg, "Succesfully created window with graphics mode.", OUTC_SUCCESS);
		} else {
		    allegro_message("Failed to initialize graphics mode");
		    return false;
		}
	} else {
        /**
         * Fullscreen mode
        */

		bool mustAutoDetectResolution = false;
		if (isResolutionInGameINIFoundAndSet()) {
			setScreenResolutionFromGameIniSettings();
			r = set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, game.screen_x, game.screen_y, game.screen_x, game.screen_y);
			char msg[255];
			sprintf(msg,"Setting up %dx%d resolution from ini file.", game.ini_screen_width, game.ini_screen_height);
			cLogger::getInstance()->log(LOG_INFO, COMP_ALLEGRO, "Custom resolution from ini file.", msg);
            mustAutoDetectResolution = (r > -1);
		} else {
            cLogger::getInstance()->log(LOG_INFO, COMP_ALLEGRO, "Custom resolution from ini file.", "No resolution defined in ini file.");
		}

		// find best possible resolution
		if (!mustAutoDetectResolution) {
            cLogger::getInstance()->log(LOG_INFO, COMP_ALLEGRO, "Autodetecting resolutions", "Commencing");
            // find best possible resolution
            cBestScreenResolutionFinder bestScreenResolutionFinder;
            bestScreenResolutionFinder.checkResolutions();
            bool result = bestScreenResolutionFinder.acquireBestScreenResolutionFullScreen();

            // success
            if (result) {
                logger->log(LOG_INFO, COMP_ALLEGRO, "Initializing graphics mode (fullscreen)", "Succesfully initialized graphics mode.", OUTC_SUCCESS);
            } else {
                logger->log(LOG_INFO, COMP_ALLEGRO, "Initializing graphics mode (fullscreen)", "Failed to initializ graphics mode.", OUTC_FAILED);
                allegro_message("Fatal error:\n\nCould not start game.\n\nGraphics mode (fullscreen) could not be initialized.");
                return false;
            }
		}

	}

	alfont_text_mode(-1);
	logger->log(LOG_INFO, COMP_ALLEGRO, "Font settings", "Set mode to -1", OUTC_SUCCESS);


	game_font = alfont_load_font("data/arakeen.fon");

	if (game_font != NULL) {
		logger->log(LOG_INFO, COMP_ALFONT, "Loading font", "loaded arakeen.fon", OUTC_SUCCESS);
		alfont_set_font_size(game_font, GAME_FONTSIZE); // set size
	} else {
		logger->log(LOG_INFO, COMP_ALFONT, "Loading font", "failed to load arakeen.fon", OUTC_FAILED);
		allegro_message("Fatal error:\n\nCould not start game.\n\nFailed to load arakeen.fon");
		return false;
	}


	bene_font = alfont_load_font("data/benegess.fon");

	if (bene_font != NULL) {
		logger->log(LOG_INFO, COMP_ALFONT, "Loading font", "loaded benegess.fon", OUTC_SUCCESS);
		alfont_set_font_size(bene_font, 10); // set size
	} else {
		logger->log(LOG_INFO, COMP_ALFONT, "Loading font", "failed to load benegess.fon", OUTC_FAILED);
		allegro_message("Fatal error:\n\nCould not start game.\n\nFailed to load benegess.fon");
		return false;
	}

	small_font = alfont_load_font("data/small.ttf");

	if (small_font != NULL) {
		logger->log(LOG_INFO, COMP_ALFONT, "Loading font", "loaded small.ttf", OUTC_SUCCESS);
		alfont_set_font_size(small_font, 10); // set size
	} else {
		logger->log(LOG_INFO, COMP_ALFONT, "Loading font", "failed to load small.ttf", OUTC_FAILED);
		allegro_message("Fatal error:\n\nCould not start game.\n\nFailed to load small.ttf");
		return false;
	}

	if (set_display_switch_mode(SWITCH_BACKGROUND) < 0)	{
		set_display_switch_mode(SWITCH_PAUSE);
		logbook("Display 'switch and pause' mode set");
	} else {
		logbook("Display 'switch to background' mode set");
	}

	int maxSounds = getAmountReservedVoicesAndInstallSound();
	memset(msg, 0, sizeof(msg));

	if (maxSounds > -1) {
		sprintf(msg, "Successfully installed sound. %d voices reserved", maxSounds);
		logger->log(LOG_INFO, COMP_SOUND, "Initialization", msg, OUTC_SUCCESS);
	} else {
		logger->log(LOG_INFO, COMP_SOUND, "Initialization", "Failed installing sound.", OUTC_FAILED);
	}
	soundPlayer = new cSoundPlayer(maxSounds);

	/***
	Bitmap Creation
	***/
    mapViewport = new cRectangle(0, cSideBar::TopBarHeight, game.screen_x-cSideBar::SidebarWidth, game.screen_y-cSideBar::TopBarHeight);

	bmp_screen = create_bitmap(game.screen_x, game.screen_y);

	if (bmp_screen == NULL)
	{
		allegro_message("Failed to create a memory bitmap");
		logbook("ERROR: Could not create bitmap: bmp_screen");
		return false;
	}
	else
	{
		logbook("Memory bitmap created: bmp_screen");
		clear(bmp_screen);
	}

    bmp_throttle = create_bitmap(game.screen_x, game.screen_y);

	if (bmp_throttle == NULL)
	{
		allegro_message("Failed to create a memory bitmap");
		logbook("ERROR: Could not create bitmap: bmp_throttle");
		return false;
	}
	else {
		logbook("Memory bitmap created: bmp_throttle");
	}

	bmp_winlose = create_bitmap(game.screen_x, game.screen_y);

	if (bmp_winlose == NULL)
	{
		allegro_message("Failed to create a memory bitmap");
		logbook("ERROR: Could not create bitmap: bmp_winlose");
		return false;
	}
	else {
		logbook("Memory bitmap created: bmp_winlose");
	}

	bmp_fadeout = create_bitmap(game.screen_x, game.screen_y);

	if (bmp_fadeout == NULL)
	{
		allegro_message("Failed to create a memory bitmap");
		logbook("ERROR: Could not create bitmap: bmp_fadeout");
		return false;
	}
	else {
		logbook("Memory bitmap created: bmp_fadeout");
	}

	/*** End of Bitmap Creation ***/
	set_color_conversion(COLORCONV_MOST);

	logbook("Color conversion method set");

	// setup mouse speed
	set_mouse_speed(0,0);

	logbook("MOUSE: Mouse speed set");

	logbook("\n----");
	logbook("GAME ");
	logbook("----");

	/*** Data files ***/

	// load datafiles
	gfxdata = load_datafile("data/gfxdata.dat");
	if (gfxdata == NULL) {
		logbook("ERROR: Could not hook/load datafile: gfxdata.dat");
		return false;
	} else {
		logbook("Datafile hooked: gfxdata.dat");
		memcpy(general_palette, gfxdata[PALETTE_D2TM].dat, sizeof general_palette);
	}

	gfxaudio = load_datafile("data/gfxaudio.dat");
	if (gfxaudio == NULL)  {
		logbook("ERROR: Could not hook/load datafile: gfxaudio.dat");
		return false;
	} else {
		logbook("Datafile hooked: gfxaudio.dat");
	}

	gfxinter = load_datafile("data/gfxinter.dat");
	if (gfxinter == NULL)  {
		logbook("ERROR: Could not hook/load datafile: gfxinter.dat");
		return false;
	} else {
		logbook("Datafile hooked: gfxinter.dat");
	}

	gfxworld = load_datafile("data/gfxworld.dat");
	if (gfxworld == NULL) {
		logbook("ERROR: Could not hook/load datafile: gfxworld.dat");
		return false;
	} else {
		logbook("Datafile hooked: gfxworld.dat");
	}

	gfxmentat = load_datafile("data/gfxmentat.dat");
	if (gfxworld == NULL) {
		logbook("ERROR: Could not hook/load datafile: gfxmentat.dat");
		return false;
	} else {
		logbook("Datafile hooked: gfxmentat.dat");
	}

	gfxmovie = NULL; // nothing loaded at start. This is done when loading a mission briefing.

	// randomize timer
	unsigned int t = (unsigned int) time(0);
	char seedtxt[80];
	sprintf(seedtxt, "Seed is %d", t);
	logbook(seedtxt);
	srand(t);

	game.bPlaying = true;
	game.screenshot = 0;
	game.state = -1;

	// Mouse stuff
	mouse_status = MOUSE_STATE_NORMAL;
	mouse_tile = 0;

	set_palette(general_palette);

	// normal sounds are loud, the music is lower (its background music, so it should not be disturbing)
	set_volume(iMaxVolume, 150);

	// A few messages for the player
	logbook("Installing:  PLAYERS");
	INSTALL_PLAYERS();
	logbook("Installing:  HOUSES");
	INSTALL_HOUSES();
	install_structures();
	install_bullets();
	install_units();
	logbook("Installing:  WORLD");
	INSTALL_WORLD();

	if (mapCamera) {
	    delete mapCamera;
	}
	mapCamera = new cMapCamera();

	if (drawManager) {
	    delete drawManager;
	}
	drawManager = new cDrawManager(&player[HUMAN]);

	if (mapUtils) {
	    delete mapUtils;
	}
	mapUtils = new cMapUtils(&map);

	game.init();

	// do install_upgrades after game.init, because game.init loads the INI file and then has the very latest
	// unit/structures catalog loaded - which the install_upgrades depends on.
    install_upgrades();

	game.setup_players();

	playMusicByType(MUSIC_MENU);

	// all has installed well. Lets rock and roll.
	return true;

}

/**
 * Set up players
 */
void cGame::setup_players() {
	if (interactionManager) {
		delete interactionManager;
	}

	// make sure each player has an own item builder
	for (int i = HUMAN; i < MAX_PLAYERS; i++) {
		cPlayer * thePlayer = &player[i];

		cSideBar * sidebar = cSideBarFactory::getInstance()->createSideBar(thePlayer, iHouse);
		thePlayer->setSideBar(sidebar);

		cBuildingListUpdater * buildingListUpdater = new cBuildingListUpdater(thePlayer);
		thePlayer->setBuildingListUpdater(buildingListUpdater);

        cItemBuilder * itemBuilder = new cItemBuilder(thePlayer, buildingListUpdater);
        thePlayer->setItemBuilder(itemBuilder);

		cStructurePlacer * structurePlacer = new cStructurePlacer(thePlayer);
		thePlayer->setStructurePlacer(structurePlacer);

		cOrderProcesser * orderProcesser = new cOrderProcesser(thePlayer);
		thePlayer->setOrderProcesser(orderProcesser);

		cGameControlsContext * gameControlsContext = new cGameControlsContext(thePlayer);
		thePlayer->setGameControlsContext(gameControlsContext);

		// set tech level
		thePlayer->setTechLevel(game.iMission);
	}

	interactionManager = new cInteractionManager(&player[HUMAN]);
}

bool cGame::isState(int thisState) {
	return (state == thisState);
}

void cGame::setState(int thisState) {
	state = thisState;
}

int cGame::getFadeSelect() {
    return fade_select;
}

void cGame::think_fading() {
    // Fading / pulsating of selected stuff
    if (bFadeSelectDir) {
        fade_select++;

        // when 255, then fade back
        if (fade_select > 254) bFadeSelectDir=false;

        return;
    }

    fade_select--;
    // not too dark,
    if (fade_select < 32) bFadeSelectDir = true;
}

cGame::~cGame() {
    // cannot do this, because when game is being quit, and the cGame object being deleted, Allegro has been shut down
    // already, so the deletion of drawManager has to happen *before* that, hence look in shutdown() method
//    if (drawManager) {
//        delete drawManager;
//    }
}
