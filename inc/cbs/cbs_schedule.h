#pragma once

#include <schedule.h>

struct CbsScheduleData : public ScheduleData {

};

class CbsSchedule : public Schedule<CbsScheduleData> {
    using Schedule<CbsScheduleData>::Schedule;
};

class HardRtSchedule : public CbsSchedule {
    using CbsSchedule::CbsSchedule;
};

class SoftRtSchedule : public CbsSchedule {
    using CbsSchedule::CbsSchedule;
};
