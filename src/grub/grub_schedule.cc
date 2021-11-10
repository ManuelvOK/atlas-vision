#include <grub/grub_schedule.h>

#include <grub/grub_job.h>
#include <grub/grub_constant_bandwidth_server.h>

GrubSchedule::GrubSchedule(unsigned id, GrubJob *job, int submission_time, unsigned core, int begin,
                         unsigned execution_time)
    : Schedule<GrubScheduleData>(job, submission_time, core),
      _grub_job(job) {
    BaseSchedule::_next_id = std::max(BaseSchedule::_next_id, id + 1);
    this->_data.emplace(submission_time, GrubScheduleData{submission_time, begin, execution_time,
                                                         false, false});
}

GrubSchedule::GrubSchedule(GrubJob *job, int submission_time, unsigned core, int begin,
                         unsigned execution_time)
    : GrubSchedule(Schedule::next_id(), job, submission_time, core, begin, execution_time) {}

GrubJob *GrubSchedule::grub_job() const {
    return this->_grub_job;
}

std::string GrubSchedule::to_string() const {
    GrubScheduleData data = this->last_data();
    std::stringstream ss;
    // > s schedule_id job_id core scheduler submission_time begin execution_time
    ss << "s " << this->_id << " " << this->_job->_id << " " << this->_core << " cbs"
       << " " << this->_submission_time << " " << data._begin << " " << data._execution_time
       << std::endl;
    return ss.str();
}

HardGrubSchedule::HardGrubSchedule(HardGrubJob *job, int submission_time, unsigned core, int begin,
                               unsigned execution_time)
    : GrubSchedule(job, submission_time, core, begin, execution_time), _grub_job(job) {}

SoftGrubSchedule::SoftGrubSchedule(SoftGrubJob *job, int submission_time, unsigned core, int begin,
                               unsigned execution_time)
    : GrubSchedule(job, submission_time, core, begin, execution_time), _grub_job(job) {}


GuiScheduleData SoftGrubSchedule::get_vision_data_at_time(int timestamp) const {
    GuiScheduleData gui_data = Schedule<GrubScheduleData>::get_vision_data_at_time(timestamp);

    gui_data._row = this->_grub_job->_cbs->id() * 2 + 1;
    return gui_data;
}

