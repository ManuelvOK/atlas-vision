#include <atlas/atlas_schedule.h>

#include <atlas/atlas_job.h>
#include <atlas/cfs_visibility.h>

bool same_data(const AtlasScheduleData &a, const AtlasScheduleData &b) {
    return a._scheduler == b._scheduler
           and a._begin == b._begin
           and a._execution_time == b._execution_time;
}

BaseAtlasSchedule::BaseAtlasSchedule(unsigned id, AtlasJob *job, unsigned submission_time,
                                     unsigned core, AtlasSchedulerType scheduler, unsigned begin,
                                     unsigned execution_time, bool end_known)
    : Schedule<AtlasScheduleData>(id, job, submission_time, core),
      _atlas_job(job) {
    BaseSchedule::_next_id = std::max(BaseSchedule::_next_id, id + 1);
    this->_data.emplace(submission_time,
                        AtlasScheduleData{submission_time, begin, execution_time, false,
                                          end_known, scheduler});
}

BaseAtlasSchedule::BaseAtlasSchedule(AtlasJob *job, unsigned submission_time, unsigned core,
                                     AtlasSchedulerType scheduler, unsigned begin,
                                     unsigned execution_time, bool end_known)
    : BaseAtlasSchedule(Schedule::next_id(), job, submission_time, core, scheduler, begin,
                        execution_time, end_known) {}

AtlasJob *BaseAtlasSchedule::atlas_job() const {
    return this->_atlas_job;
}

GuiScheduleData BaseAtlasSchedule::get_vision_data_at_time(unsigned timestamp) const {
    AtlasScheduleData data = this->data_at_time(timestamp);
    GuiScheduleData gui_data = Schedule<AtlasScheduleData>::get_vision_data_at_time(timestamp);

    gui_data._row = static_cast<unsigned>(data._scheduler);
    return gui_data;
}

CfsSchedule::CfsSchedule(AtlasSchedule *s, unsigned submission_time, unsigned begin,
                         unsigned execution_time)
    : BaseAtlasSchedule(s) {
    this->_submission_time = submission_time;
    this->_data.clear();
    this->_data.emplace(submission_time,
                        AtlasScheduleData{submission_time, begin, execution_time, false, false,
                                          AtlasSchedulerType::CFS});
}

EarlyCfsSchedule::EarlyCfsSchedule(AtlasSchedule *s, unsigned submission_time, unsigned begin,
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
