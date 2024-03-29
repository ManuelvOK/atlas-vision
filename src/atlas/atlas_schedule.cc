#include <atlas/atlas_schedule.h>

#include <atlas/atlas_job.h>
#include <atlas/cfs_visibility.h>

bool same_data(const AtlasScheduleData &a, const AtlasScheduleData &b) {
    return a._scheduler == b._scheduler
           and a._begin == b._begin
           and a._execution_time == b._execution_time;
}

BaseAtlasSchedule::BaseAtlasSchedule(unsigned id, AtlasJob *job, int submission_time,
                                     unsigned core, AtlasSchedulerType scheduler, int begin,
                                     unsigned execution_time, bool end_known)
    : Schedule<AtlasScheduleData>(id, job, submission_time, core),
      _atlas_job(job) {
    BaseSchedule::_next_id = std::max(BaseSchedule::_next_id, id + 1);
    this->_data.emplace(submission_time,
                        AtlasScheduleData{submission_time, begin, execution_time, false,
                                          end_known, scheduler});
}

BaseAtlasSchedule::BaseAtlasSchedule(AtlasJob *job, int submission_time, unsigned core,
                                     AtlasSchedulerType scheduler, int begin,
                                     unsigned execution_time, bool end_known)
    : BaseAtlasSchedule(Schedule::next_id(), job, submission_time, core, scheduler, begin,
                        execution_time, end_known) {}

AtlasJob *BaseAtlasSchedule::atlas_job() const {
    return this->_atlas_job;
}

GuiScheduleData BaseAtlasSchedule::get_vision_data_at_time(int timestamp) const {
    AtlasScheduleData data = this->data_at_time(timestamp);
    GuiScheduleData gui_data = Schedule<AtlasScheduleData>::get_vision_data_at_time(timestamp);

    gui_data._row = static_cast<unsigned>(data._scheduler);
    return gui_data;
}

std::string BaseAtlasSchedule::to_string() const {
    AtlasScheduleData data = this->last_data();
    static std::map<AtlasSchedulerType, char> scheduler_char = {
        {AtlasSchedulerType::ATLAS, 'a'},
        {AtlasSchedulerType::recovery, 'r'},
        {AtlasSchedulerType::CFS, 'c'},
    };
    std::stringstream ss;
    // > s schedule_id job_id core scheduler submission_time begin execution_time
    ss << "s " << this->_id << " " << this->_job->_id << " " << this->_core
       << " " << scheduler_char[data._scheduler] << " " << this->_submission_time
       << " " << data._begin << " " << data._execution_time << std::endl;
    return ss.str();
}

CfsSchedule::CfsSchedule(AtlasSchedule *s, int submission_time, int begin, unsigned execution_time)
    : BaseAtlasSchedule(s) {
    this->_submission_time = submission_time;
    this->_data.clear();
    this->_data.emplace(submission_time,
                        AtlasScheduleData{submission_time, begin, execution_time, false, false,
                                          AtlasSchedulerType::CFS});
}

EarlyCfsSchedule::EarlyCfsSchedule(AtlasSchedule *s, int submission_time, int begin,
                                   unsigned execution_time)
    : CfsSchedule(s, submission_time, begin, execution_time),
    _atlas_schedule(s) {
    //this->_visibility = new CfsVisibility(this, begin, this->_atlas_schedule->last_data()._begin);
}

CfsVisibility EarlyCfsSchedule::create_visibility() const {
    return CfsVisibility(this->_atlas_schedule, this->first_data()._begin,
                         std::min(this->last_data().end(),
                                  this->_atlas_schedule->last_data()._begin));
}

RecoverySchedule::RecoverySchedule(const BaseAtlasSchedule *s) :
    BaseAtlasSchedule(s) {
    for (auto &data: this->_data) {
        data.second._scheduler = AtlasSchedulerType::recovery;
    }
}
