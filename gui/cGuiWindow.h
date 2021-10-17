#ifndef D2TM_CGUIWINDOW_H
#define D2TM_CGUIWINDOW_H

#include <vector>
#include "cGuiObject.h"
#include "../observers/cMouseObserver.h"

class cGuiWindow : cGuiObject {
public:
    cGuiWindow(cRectangle rect);
    ~cGuiWindow();

    void onNotifyMouseEvent(const s_MouseEvent &event) override;

    void draw() const override;

    void addGuiObject(cGuiObject *guiObject);

private:
    std::vector<cGuiObject *> gui_objects;
};


#endif //D2TM_CGUIWINDOW_H
