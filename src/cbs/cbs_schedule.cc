#include <cbs/cbs_schedule.h>

#include <cbs/cbs_job.h>

CbsSchedule::CbsSchedule(unsigned id, CbsJob *job, unsigned submission_time, unsigned core, unsigned begin, unsigned execution_time)
    : Schedule<CbsScheduleData>(job, submission_time, core),
      _cbs_job(job) {
    BaseSchedule::_next_id = std::max(BaseSchedule::_next_id, id + 1);
    this->_data.emplace(submission_time, CbsScheduleData{submission_time, begin, execution_time,
                                                         false, false});
}

CbsSchedule::CbsSchedule(CbsJob *job, unsigned submission_time, unsigned core, unsigned begin, unsigned execution_time)
    : CbsSchedule(Schedule::next_id(), job, submission_time, core, begin, execution_time) {}

CbsJob *CbsSchedule::cbs_job() const {
    return this->_cbs_job;
}

HardRtSchedule::HardRtSchedule(HardRtJob *job, int submission_time, unsigned core, unsigned begin, unsigned execution_time)
    : CbsSchedule(job, submission_time, core, begin, execution_time), _rt_job(job) {}

SoftRtSchedule::SoftRtSchedule(SoftRtJob *job, int submission_time, unsigned core, unsigned begin, unsigned execution_time)
    : CbsSchedule(job, submission_time, core, begin, execution_time), _rt_job(job) {}
