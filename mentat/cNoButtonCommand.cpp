#include "d2tmh.h"

void cNoButtonCommand::execute(cAbstractMentat& mentat) {
    logbook("cNoButtonCommand::execute()");
    // head back to choose house
    game.iHouse=-1;
    game.setState(GAME_SELECT_HOUSE);
    game.FADE_OUT();
}
