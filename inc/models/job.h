#pragma once

#include <vector>

#include <models/schedule.h>
#include <models/printable.h>

enum class JobState {
    running,
    ready,
    blocking,
    queued
};

class AtlasModel;
/** Object representing a job */
class Job : public Printable {
public:
    const AtlasModel *_atlas_model;
    int _id;                        /**< job id */
    int _deadline;                  /**< timestamp for the deadline of the job */
    int _execution_time_estimate;   /**< estimated execution time */
    int _execution_time;            /**< real execution time */
    int _submission_time;           /**< timestamp the job gets submitted */
    int _dependency_level = -1;     /**< level in dependency DAG. -1 before calculated */

    JobState _state = JobState::queued;

    std::vector<Schedule *> _schedules;

    AtlasSchedule *_atlas_schedule = nullptr;

    /** List of Jobs, the execution of this job depends on */
    std::vector<Job *> _known_dependencies;
    /** List of Jobs, the execution of this job depends on unknowingly */
    std::vector<Job *> _unknown_dependencies;

    /**
     * Constructor
     * @param id job id
     * @param deadline timestamp for jobs deadline
     * @param execution_time_estimate estimated time for job execution
     * @param execution_time real execution time
     * @param submission_time timestamp of jobs submission
     */
    Job(const AtlasModel *atlas_model, int id, int deadline, int execution_time_estimate,
        int execution_time, int submission_time)
        : _atlas_model(atlas_model), _id(id), _deadline(deadline),
        _execution_time_estimate(execution_time_estimate), _execution_time(execution_time),
        _submission_time(submission_time) {}

    /**
     * calculate and set the level of this Job inside the dependency DAG
     * @returns dependency level
     */
    int calculate_dependency_level();

    /* amount of time to still be executed */
    int execution_time_left(int timestamp) const;

    /* amount of time estimated to still be executed */
    int estimated_execution_time_left(int timestamp) const;

    int time_executed(int timestamp) const;

    virtual std::string to_string() const override;

    bool depends_on(const Job *job) const;

    void set_atlas_schedule(AtlasSchedule *schedule);
};

