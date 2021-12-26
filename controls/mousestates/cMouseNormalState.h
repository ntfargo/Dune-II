#ifndef D2TM_CMOUSENORMALSTATE_H
#define D2TM_CMOUSENORMALSTATE_H

#include "cMouseState.h"

/**
 * A mouse normal state is at the battlefield, and is the default state the mouse is in. It can select units, drag
 * a border (for multi-select) and drag the viewport
 *
 */
class cMouseNormalState : public cMouseState {

public:
    explicit cMouseNormalState(cPlayer * player, cGameControlsContext *context, cMouse * mouse);
    ~cMouseNormalState();

    void onNotifyMouseEvent(const s_MouseEvent &event) override;

    void onMouseLeftButtonClicked(const s_MouseEvent &event);

    void onMouseRightButtonPressed(const s_MouseEvent &event);

    void onMouseRightButtonClicked(const s_MouseEvent &event);

    void onMouseMovedTo(const s_MouseEvent &event);
};


#endif //D2TM_CMOUSENORMALSTATE_H
