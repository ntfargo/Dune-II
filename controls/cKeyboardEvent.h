#pragma once

#include <string>
#include <set>

#include "utils/cPoint.h"
#include "enums.h"
#include "utils/common.h"

#define HAVE_STDINT_H
#include <allegro/keyboard.h>

enum class eKeyEventType {
    NONE,
    HOLD,    // key is being pressed down (hold)
    PRESSED, // key is held, then released. Makes a key "pressed".
};

class cKeyboardEvent {

public:
    cKeyboardEvent(eKeyEventType eventType, std::set<int> & keys);

    eKeyEventType eventType = eKeyEventType::NONE;

    inline const std::string toString(const cKeyboardEvent &event) const {
        std::string str;
        char partOne[64];
        sprintf(partOne, "cKeyboardEvent [type=%s], keys: ",
                toStringKeyboardEventType(event.eventType)
        );

        str.append(partOne);
        str.append("[");
        for (auto key : event.keys) {
            char msg[10];
            sprintf(msg, "%s,", scancode_to_name(key));
            str.append(msg);
        }
        str.append("]");

        return str;
    }

    bool hasKey(int scanCode) const {
        return keys.find(scanCode) != keys.end();
    }

    /**
     * Returns true when both scancodes are present in this event
     * @param firstScanCode
     * @param secondScanCode
     * @return
     */
    bool hasKeys(int firstScanCode, int secondScanCode) const {
        return hasKey(firstScanCode) && hasKey(secondScanCode);
    }

    /**
     * Returns true when one of both scancodes are present in this event
     * @param firstScanCode
     * @param secondScanCode
     * @return
     */
    bool hasEitherKey(int firstScanCode, int secondScanCode) const {
        return hasKey(firstScanCode) || hasKey(secondScanCode);
    }

    bool isType(eKeyEventType type) const {
        return eventType == type;
    }

    /**
     * If a numerical key is pressed, return that value.
     * @return
     */
    int getGroupNumber() const;

private:
    inline const char* toStringKeyboardEventType(const eKeyEventType &type) const {
        switch (type) {
            case eKeyEventType::NONE: return "NONE";
            case eKeyEventType::HOLD: return "HOLD";
            case eKeyEventType::PRESSED: return "PRESSED";
            default:
                assert(false);
                break;
        }
        return "";
    }

    std::set<int> keys;
};
