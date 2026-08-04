#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "Utility/Error/Result.h"

#include "Engine/Evt/EvtInstruction.h"

class Blob;

struct EventTrigger {
    int eventId = 0;
    int eventStep = 0;
};

class EvtProgram {
 public:
    static Result<EvtProgram> load(const Blob &rawData);

    void add(int eventId, EvtInstruction ir);
    void clear();

    bool hasEvent(int eventId) const {
        return _eventsById.contains(eventId);
    }

    /**
     * @param eventId                   Event id to look up.
     * @param step                      Step to look up.
     * @return                          The requested instruction, or an error if there is no such event or step.
     */
    Result<EvtInstruction> instruction(int eventId, int step) const;

    /**
     * @param eventId                   Event id.
     * @return                          List of instructions for the provided `eventId`, or `nullptr` if there is
     *                                  no such event.
     */
    const std::vector<EvtInstruction> *function(int eventId) const;

    /**
     * @param triggerType               Event type to look for.
     * @return                          List of all event positions that have the given event type.
     */
    std::vector<EventTrigger> enumerateTriggers(EvtOpcode triggerType);

    /**
     *
     * @param eventId                   Event id to check.
     * @return                          Whether a script exists for the provided `eventId` that shows a hint.
     */
    bool hasHint(int eventId) const;

    /**
     * @param eventId                   Event id to check.
     * @return                          Hint to show, if any. Note that unlike `events()`, this function returns
     *                                  an empty string for non-existent events.
     */
    std::string hint(int eventId) const;

    void dumpAll() const;
    void dump(int eventId) const;

 private:
    std::unordered_map<int, std::vector<EvtInstruction>> _eventsById;
};
