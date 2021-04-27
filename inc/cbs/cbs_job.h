#pragma once

#include <cbs/cbs_schedule.h>
#include <job.h>

class CbsJob: public Job<CbsSchedule> {
};

class HardRtJob: public CbsJob {
};

class SoftRtJob: public CbsJob {
};
