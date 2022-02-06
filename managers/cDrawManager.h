#pragma once

#include "controls/cKeyboardEvent.h"
#include "drawers/cBuildingListDrawer.h"
#include "drawers/cMapDrawer.h"
#include "drawers/cMessageDrawer.h"
#include "drawers/cMiniMapDrawer.h"
#include "drawers/cMouseDrawer.h"
#include "drawers/cOrderDrawer.h"
#include "drawers/cParticleDrawer.h"
#include "drawers/cPlaceItDrawer.h"
#include "drawers/CreditsDrawer.h"
#include "drawers/cSideBarDrawer.h"
#include "drawers/cStructureDrawer.h"

#include "observers/cInputObserver.h"
#include "sMouseEvent.h"

class cPlayer;
class BITMAP;

class cDrawManager : cInputObserver {

public:
    explicit cDrawManager(cPlayer *thePlayer);
    ~cDrawManager();

    void drawCombatState();

    void onNotifyMouseEvent(const s_MouseEvent &event) override;
    void onNotifyKeyboardEvent(const cKeyboardEvent &event) override;

    CreditsDrawer *getCreditsDrawer() { return &m_creditsDrawer; }

    cMessageDrawer *getMessageDrawer() { return &m_messageDrawer; }

    cMiniMapDrawer *getMiniMapDrawer() { return &miniMapDrawer; }

    cOrderDrawer *getOrderDrawer() { return &m_orderDrawer; }

    cMouseDrawer *getMouseDrawer() { return &m_mouseDrawer; }

    cPlaceItDrawer *getPlaceItDrawer() { return &m_placeitDrawer; }

    cBuildingListDrawer *getBuildingListDrawer() { return m_sidebarDrawer.getBuildingListDrawer(); }

    void drawMouse();

    void drawCombatMouse();

    void setPlayerToDraw(cPlayer *playerToDraw);

    void think();

    void init();

protected:
    void drawSidebar();

    void drawCredits();

    void drawStructurePlacing();

    void drawMessage();

    void drawRallyPoint();

    void drawTopBarBackground();

private:
    void drawOptionBar();

    void drawDebugInfoUsages() const;

    void drawNotifications();

    // Properties:
    cSideBarDrawer m_sidebarDrawer;
    CreditsDrawer m_creditsDrawer;
    cOrderDrawer m_orderDrawer;
    cMapDrawer m_mapDrawer;
    cMiniMapDrawer miniMapDrawer;
    cParticleDrawer m_particleDrawer;
    cMessageDrawer m_messageDrawer;
    cPlaceItDrawer m_placeitDrawer;
    cStructureDrawer m_structureDrawer;
    cMouseDrawer m_mouseDrawer;

    BITMAP *m_optionsBar;

    int m_sidebarColor;

    // TODO: unitDrawer

    // TODO: bullet/projectile drawer

    cPlayer *m_player;

    BITMAP *m_topBarBmp;

    cTextDrawer m_textDrawer;

    void onKeyDown(const cKeyboardEvent &event);

    void onKeyPressed(const cKeyboardEvent &event);
};
