#include "d2tmh.h"

cMainMenuGameState::cMainMenuGameState(cGame &theGame) : cGameState(theGame), textDrawer(cTextDrawer(bene_font)) {
    bmp_D2TM_Title = (BITMAP *) gfxinter[BMP_D2TM].dat;

    int logoWidth = bmp_D2TM_Title->w;
    int logoHeight = bmp_D2TM_Title->h;

    logoX = (game.screen_x / 2) - (logoWidth / 2);
    logoY = (logoHeight/10);

    mainMenuWidth = 130;
    mainMenuHeight = 143;

    // adjust x and y according to resolution, we can add because the above values
    // assume 640x480 resolution, and logoX/logoY are already taking care of > resolutions
    mainMenuFrameX = 257 + logoX;
    mainMenuFrameY = 319 + logoY;

    // Buttons:
    int buttonsX = mainMenuFrameX + 4;

    // PLAY
    int playY = 323 + logoY;

    int buttonHeight = textDrawer.getFontHeight();
    int buttonWidth = mainMenuWidth - 8;

    const cRectangle &window = cRectangle(mainMenuFrameX, mainMenuFrameY, mainMenuWidth, mainMenuHeight);
    gui_window = new cGuiWindow(window);

    const cRectangle &campaign = cRectangle(buttonsX, playY, buttonWidth, buttonHeight);
    cGuiButton *gui_btn_SelectHouse = new cGuiButton(textDrawer, campaign, "Campaign", eGuiButtonRenderKind::OPAQUE_WITH_BORDER);
    gui_btn_SelectHouse->setTextAlignHorizontal(eGuiTextAlignHorizontal::LEFT);
    gui_btn_SelectHouse->setGui_ColorButton(makecol(128, 0, 128));
    gui_btn_SelectHouse->setOnLeftMouseButtonClickedAction(new cGuiActionSelectHouse());
    gui_window->addGuiObject(gui_btn_SelectHouse);

    int skirmishY = 344 + logoY;
    const cRectangle &skirmish = cRectangle(buttonsX, skirmishY, buttonWidth, buttonHeight);
    cGuiButton *gui_btn_Skirmish = new cGuiButton(textDrawer, skirmish, "Skirmish", eGuiButtonRenderKind::OPAQUE_WITH_BORDER);
    gui_btn_Skirmish->setOnLeftMouseButtonClickedAction(new cGuiActionSetupSkirmishGame());
    gui_window->addGuiObject(gui_btn_Skirmish);

    int multiplayerY = 364 + logoY;
    const cRectangle &multiplayer = cRectangle(buttonsX, multiplayerY, buttonWidth, buttonHeight);
    cGuiButton *gui_btn_Multiplayer = new cGuiButton(textDrawer, multiplayer, "Multiplayer", eGuiButtonRenderKind::OPAQUE_WITH_BORDER);
    gui_btn_Multiplayer->setTextColor(makecol(225, 225, 225));
    gui_btn_Multiplayer->setTextColorHover(makecol(128, 128, 128));
    gui_btn_Multiplayer->setOnLeftMouseButtonClickedAction(new cGuiActionFadeOutOnly());
    gui_window->addGuiObject(gui_btn_Multiplayer);

    // LOAD
    int loadY = 384 + logoY;
    const cRectangle &load = cRectangle(buttonsX, loadY, buttonWidth, buttonHeight);
    cGuiButton *gui_btn_Load = new cGuiButton(textDrawer, load, "Load", eGuiButtonRenderKind::OPAQUE_WITH_BORDER);
    gui_btn_Load->setTextColor(makecol(225, 225, 225));
    gui_btn_Load->setTextColorHover(makecol(128, 128, 128));
    gui_btn_Load->setOnLeftMouseButtonClickedAction(new cGuiActionFadeOutOnly());
    gui_window->addGuiObject(gui_btn_Load);

    // OPTIONS
    int optionsY = 404 + logoY;
    const cRectangle &options = cRectangle(buttonsX, optionsY, buttonWidth, buttonHeight);
    cGuiButton *gui_btn_Options = new cGuiButton(textDrawer, options, "Options", eGuiButtonRenderKind::OPAQUE_WITH_BORDER);
    gui_btn_Options->setTextColor(makecol(225, 225, 225));
    gui_btn_Options->setTextColorHover(makecol(128, 128, 128));
    gui_btn_Options->setOnLeftMouseButtonClickedAction(new cGuiActionFadeOutOnly());
    gui_window->addGuiObject(gui_btn_Options);

    // HALL OF FAME
    int hofY = 424 + logoY;
    const cRectangle &hof = cRectangle(buttonsX, hofY, buttonWidth, buttonHeight);
    cGuiButton *gui_btn_Hof = new cGuiButton(textDrawer, hof, "Hall of Fame", eGuiButtonRenderKind::OPAQUE_WITH_BORDER);
    gui_btn_Hof->setTextColor(makecol(225, 225, 225));
    gui_btn_Hof->setTextColorHover(makecol(128, 128, 128));
    gui_btn_Hof->setOnLeftMouseButtonClickedAction(new cGuiActionFadeOutOnly());
    gui_window->addGuiObject(gui_btn_Hof);

    // EXIT
    int exitY = 444 + logoY;
    const cRectangle &exit = cRectangle(buttonsX, exitY, buttonWidth, buttonHeight);
    cGuiButton *gui_btn_Exit = new cGuiButton(textDrawer, exit, "Exit", eGuiButtonRenderKind::OPAQUE_WITH_BORDER);
    gui_btn_Exit->setOnLeftMouseButtonClickedAction(new cGuiActionExitGame());
    gui_window->addGuiObject(gui_btn_Exit);
}

cMainMenuGameState::~cMainMenuGameState() {
    delete gui_window;
}

void cMainMenuGameState::thinkFast() {

}

void cMainMenuGameState::draw() const {
    if (DEBUGGING) {
        for (int x = 0; x < game.screen_x; x += 60) {
            for (int y = 0; y < game.screen_y; y += 20) {
                rect(bmp_screen, x, y, x + 50, y + 10, makecol(64, 64, 64));
                putpixel(bmp_screen, x, y, makecol(255, 255, 255));
                alfont_textprintf(bmp_screen, bene_font, x, y, makecol(32, 32, 32), "Debug");
            }
        }
    }

    draw_sprite(bmp_screen, bmp_D2TM_Title, logoX, logoY);

    GUI_DRAW_FRAME(mainMenuFrameX, mainMenuFrameY, mainMenuWidth,mainMenuHeight);

    gui_window->draw();

    int creditsX = (game.screen_x / 2) - (alfont_text_length(bene_font, "CREDITS") / 2);
    GUI_DRAW_BENE_TEXT_MOUSE_SENSITIVE(creditsX, 1, "CREDITS", makecol(64, 64, 64));


    // draw version
    textDrawer.drawTextBottomRight(game.version);

    // mp3 addon?
    if (game.bMp3) {
        textDrawer.drawTextBottomLeft("Music: MP3 ADD-ON");
    } else {
        textDrawer.drawTextBottomLeft("Music: MIDI");
    }

    if (DEBUGGING) {
        char mouseTxt[255];
        sprintf(mouseTxt, "%d, %d", mouse_x, mouse_y);
        textDrawer.drawText(0, 0, mouseTxt);
    }

    // MOUSE
    game.getMouse()->draw();

    if (key[KEY_ESC]) {
        game.bPlaying=false;
    }
}

void cMainMenuGameState::onNotifyMouseEvent(const s_MouseEvent &event) {
    gui_window->onNotifyMouseEvent(event);
}

eGameStateType cMainMenuGameState::getType() {
    return GAMESTATE_MAIN_MENU;
}
