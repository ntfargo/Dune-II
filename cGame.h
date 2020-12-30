/*

  Dune II - The Maker

  Author : Stefan Hendriks
  Contact: stefanhen83@gmail.com
  Website: http://dune2themaker.fundynamic.com

  2001 - 2010 (c) code by Stefan Hendriks

  */

// TODO: Clean this class up big time.

#ifndef D2TM_GAME_H
#define D2TM_GAME_H

// forward declaration :/ sigh should really look into these includes and such
class cRectangle;
class cAbstractMentat;

class cGame {

public:

	cGame();
	~cGame();

	std::string game_filename;

	bool windowed;				// windowed
	char version[15];			// version number, or name.

    // Alpha (for fading in/out)
    int iAlphaScreen;           // 255 = opaque , anything else
    int iFadeAction;            // 0 = NONE, 1 = fade out (go to 0), 2 = fade in (go to 255)

    // World stuff
    // 1. Show current conquered regions
    // 2. Show next progress + story (in message bar)
    // 3. Click next region
    // 4. Set up region and go to GAME_BRIEFING, which will do the rest...-> fade out

//    int state;           //
//    int iRegionScene;           // scene
//    int iRegionSceneAlpha;           // scene alpha
//    int iRegionConquer[MAX_REGIONS];     // INDEX = REGION NR , > -1 means conquered..
//    int iRegionHouse[MAX_REGIONS];
//    char cRegionText[MAX_REGIONS][255]; // text for this

    // resolution of the game
	int screen_x;
	int screen_y;
    int ini_screen_width;
    int ini_screen_height;

    bool bPlaySound;            // play sound?
    bool bDisableAI;            // disable AI thinking?
    bool bPlayMusic;            // play any music?
    bool bMp3;                  // use mp3 files instead of midi

	bool bPlaying;				// playing or not
    bool bSkirmish;             // playing a skirmish game  or not
	int  iSkirmishMap;			// what map is selected
	int screenshot;				// screenshot taking number
	int iSkirmishStartPoints;	// random map startpoints

	void init();		// initialize all game variables
	void mission_init(); // initialize variables for mission loading only
	void run();			// run the game

	int map_width;
	int map_height;

    int iRegion;        // what region is selected?
	int iMission;		// what mission are we playing? (= techlevel)

	int selected_structure;
	int hover_unit;

	int paths_created;

    int iMusicVolume;       // volume of the mp3 / midi

//    // Movie playback (scene's from datafile)
//    int TIMER_movie;
//    int iMovieFrame;

    // throttle stuff
	int shake_x;
    int shake_y;
    int TIMER_shake;

    int iMusicType;

    // Condition to win the mission:
    int iWinQuota;              // > 0 means, get this to win the mission, else, destroy all!

    void think_winlose();
    void winning();       // winning
    void losing();        // losing

	bool bPlaceIt;		// placing something? (for structures only)
	bool bPlacedIt;		// for remembering, for combat_mouse stuff..

	void setup_players();

    void think_music();

	void think_mentat();

    void FADE_OUT(); // fade out with current screen_bmp, this is a little game loop itself!

    void prepareMentatForPlayer();

	bool setupGame();
	void shutdown();

	bool isState(int thisState);
	void setState(int newState);

	int getMaxVolume() { return iMaxVolume; }

	cSoundPlayer * getSoundPlayer() {
	    return soundPlayer;
	}

    void combat_mouse();

	int getGroupNumberFromKeyboard();
	void destroyAllUnits(bool);
	void destroyAllStructures(bool);

    int getFadeSelect();

    void think_fading();

    cRectangle * mapViewport;

    void init_skirmish() const;

    void createAndPrepareMentatForHumanPlayer();

    void loadScenario();

private:
	void poll();
	void combat();		// the combat part (main) of the game
	bool isMusicPlaying();

    void setup_skirmish();  // set up a skirmish game
	void stateSelectHouse();		// house selection
	void stateMentat(cAbstractMentat *pMentat);  // state mentat talking and interaction
	void menu();		// main menu
	void stateSelectYourNextConquest();		// region selection

	void runGameState();
	void shakeScreenAndBlitBuffer();
	void handleTimeSlicing();

    bool isResolutionInGameINIFoundAndSet();
    void setScreenResolutionFromGameIniSettings();

	/** game state **/
	int state;

	int iMaxVolume;

	cSoundPlayer *soundPlayer;
	cAbstractMentat *pMentat;

    int fade_select;        // fade color when selected
    bool bFadeSelectDir;    // fade select direction
    void prepareMentatToTellAboutHouse(int house);
};

#endif