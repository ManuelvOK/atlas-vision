#include <cbs/cbs_simulation_action.h>

template<>
void CbsSubmissionAction<HardRtJob>::execute() {
    // Add to queue
}

template<>
void CbsSubmissionAction<SoftRtJob>::execute() {
    // change deadline
    // Add to queue
}

void CbsDeadlineAction::execute() {
    // generate message
}

void CbsFillAction::execute() {
    // Check queues, and decide EDF-wise
}

template<>
void CbsBeginScheduleAction<HardRtSchedule>::execute() {
    // Add End Action
}

template<>
void CbsBeginScheduleAction<SoftRtSchedule>::execute() {
    // Add End Action
}

template<>
void CbsEndScheduleAction<HardRtSchedule>::execute() {
    // Add Fill Action
}

template<>
void CbsEndScheduleAction<SoftRtSchedule>::execute() {
    // Add Fill Action
}
