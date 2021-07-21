#ifndef D2TM_CPLAYERBRAINSKIRMISH_H
#define D2TM_CPLAYERBRAINSKIRMISH_H

#include "player/brains/cPlayerBrain.h"
#include "player/brains/cPlayerBrainData.h"
#include "player/brains/missions/cPlayerBrainMission.h"
#include <set>

namespace brains {

    const int MISSION_IMPROVE_ECONOMY_BUILD_ADDITIONAL_HARVESTER = 98;
    const int MISSION_IMPROVE_ECONOMY_BUILD_ADDITIONAL_CARRYALL = 99;
    const int MISSION_SCOUT = 97;

    struct s_SkirmishPlayer_PlaceForStructure {
        int structureType;
        int cell;
    };

    /**
     * The cPlayerBrainSkirmish is responsible for being an AI brain during skirmish games.
     */
    class cPlayerBrainSkirmish : public cPlayerBrain {

    public:
        cPlayerBrainSkirmish(cPlayer *player);

        ~cPlayerBrainSkirmish();

        void think() override;

        void onNotify(const s_GameEvent &event) override;

        void addBuildOrder(S_buildOrder order);

    private:
        ePlayerBrainState state;

        ePlayerBrainSkirmishThinkState thinkState;

        ePlayerBrainSkirmishEconomyState economyState;

        int COUNT_badEconomy;

        int TIMER_rest;

        // at which cells did we detect an enemy? Remember those.
        std::set<int> discoveredEnemyAtCell;

        std::vector<cPlayerBrainMission> missions;
        std::vector<S_structurePosition> myBase;
        std::vector<S_buildOrder> buildOrders;

        void onMyStructureDestroyed(const s_GameEvent &event);

        void onMyStructureCreated(const s_GameEvent &event);

        void onMyStructureAttacked(const s_GameEvent &event);

        void onMyStructureDecayed(const s_GameEvent &event);

        void thinkState_Base();

        void thinkState_Missions();

        void thinkState_Evaluate();

        void thinkState_EndGame();

        void thinkState_ProcessBuildOrders();

        void changeThinkStateTo(const ePlayerBrainSkirmishThinkState& newState);

        void changeEconomyStateTo(const ePlayerBrainSkirmishEconomyState& newState);

        static const char* ePlayerBrainSkirmishThinkStateString(const ePlayerBrainSkirmishThinkState &state) {
            switch (state) {
                case ePlayerBrainSkirmishThinkState::PLAYERBRAIN_SKIRMISH_STATE_EVALUATE: return "PLAYERBRAIN_SKIRMISH_STATE_EVALUATE";
                case ePlayerBrainSkirmishThinkState::PLAYERBRAIN_SKIRMISH_STATE_PROCESS_BUILDORDERS: return "PLAYERBRAIN_SKIRMISH_STATE_PROCESS_BUILDORDERS";
                case ePlayerBrainSkirmishThinkState::PLAYERBRAIN_SKIRMISH_STATE_BASE: return "PLAYERBRAIN_SKIRMISH_STATE_BASE";
                case ePlayerBrainSkirmishThinkState::PLAYERBRAIN_SKIRMISH_STATE_ENDGAME: return "PLAYERBRAIN_SKIRMISH_STATE_ENDGAME";
                case ePlayerBrainSkirmishThinkState::PLAYERBRAIN_SKIRMISH_STATE_MISSIONS: return "PLAYERBRAIN_SKIRMISH_STATE_MISSIONS";
                case ePlayerBrainSkirmishThinkState::PLAYERBRAIN_SKIRMISH_STATE_REST: return "PLAYERBRAIN_SKIRMISH_STATE_REST";
                default:
                    assert(false);
                    break;
            }
            return "";
        }

        static const char* ePlayerBrainSkirmishEconomyStateString(const ePlayerBrainSkirmishEconomyState &state) {
            switch (state) {
                case ePlayerBrainSkirmishEconomyState::PLAYERBRAIN_ECONOMY_STATE_NORMAL: return "PLAYERBRAIN_ECONOMY_STATE_NORMAL";
                case ePlayerBrainSkirmishEconomyState::PLAYERBRAIN_ECONOMY_STATE_IMPROVE: return "PLAYERBRAIN_ECONOMY_STATE_IMPROVE";
                case ePlayerBrainSkirmishEconomyState::PLAYERBRAIN_ECONOMY_STATE_BAD: return "PLAYERBRAIN_ECONOMY_STATE_BAD";
                case ePlayerBrainSkirmishEconomyState::PLAYERBRAIN_ECONOMY_STATE_SELL_FOR_CASH: return "PLAYERBRAIN_ECONOMY_STATE_SELL_FOR_CASH";
                default:
                    assert(false);
                    break;
            }
            return "";
        }

        static const char* ePlayerBrainStateString(const ePlayerBrainState &state) {
            switch (state) {
                case ePlayerBrainState::PLAYERBRAIN_ENEMY_DETECTED: return "PLAYERBRAIN_ENEMY_DETECTED";
                case ePlayerBrainState::PLAYERBRAIN_LOSING: return "PLAYERBRAIN_LOSING";
                case ePlayerBrainState::PLAYERBRAIN_PEACEFUL: return "PLAYERBRAIN_PEACEFUL";
                case ePlayerBrainState::PLAYERBRAIN_WINNING: return "PLAYERBRAIN_WINNING";
                default:
                    assert(false);
                    break;
            }
            return "";
        }

        void thinkState_Rest();

        void onEntityDiscoveredEvent(const s_GameEvent &event);

        void produceMissions();

        void addMission(ePlayerBrainMissionKind kind, const std::vector<S_groupKind> &group, int initialDelay,
                        int id);

        bool hasMission(const int id);

        s_SkirmishPlayer_PlaceForStructure thinkAboutNextStructureToBuildAndPlace();

        int getStructureToBuild() const;

        int getStructureIdToBuildWithoutConsideringPowerUsage() const;

        bool hasBuildOrderQueuedForStructure();

        void evaluateEconomyState();
    };

}

#endif //D2TM_CPLAYERBRAINSKIRMISH_H
