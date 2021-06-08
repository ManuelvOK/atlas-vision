#include <cbs/cbs_schedule.h>

#include <cbs/cbs_job.h>
#include <cbs/constant_bandwidth_server.h>

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

std::string CbsSchedule::to_string() const {
    CbsScheduleData data = this->last_data();
    std::stringstream ss;
    // > s schedule_id job_id core scheduler submission_time begin execution_time
    ss << "s " << this->_id << " " << this->_job->_id << " " << this->_core << " cbs"
       << " " << this->_submission_time << " " << data._begin << " " << data._execution_time
       << std::endl;
    return ss.str();
}

HardRtSchedule::HardRtSchedule(HardRtJob *job, int submission_time, unsigned core, unsigned begin, unsigned execution_time)
    : CbsSchedule(job, submission_time, core, begin, execution_time), _rt_job(job) {}

SoftRtSchedule::SoftRtSchedule(SoftRtJob *job, int submission_time, unsigned core, unsigned begin, unsigned execution_time)
    : CbsSchedule(job, submission_time, core, begin, execution_time), _rt_job(job) {}


GuiScheduleData SoftRtSchedule::get_vision_data_at_time(unsigned timestamp) const {
    GuiScheduleData gui_data = Schedule<CbsScheduleData>::get_vision_data_at_time(timestamp);

    gui_data._row = this->_rt_job->_cbs->id() * 2 + 1;
    return gui_data;
}
