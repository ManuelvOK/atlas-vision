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
private:
    /** List of Jobs, the execution of this job depends on */
    std::vector<Job *> _known_dependencies;
    /** List of Jobs, the execution of this job depends on unknowingly */
    std::vector<Job *> _unknown_dependencies;
    /** list of Jobs, whose execution depends on this job */
    std::vector<Job *> _known_dependees;
    /** list of Jobs, whose execution depends on this job unknowingly */
    std::vector<Job *> _unknown_dependees;
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
     * Add a job to the known dependencies
     * @param job job to add
     */
    void add_known_dependency(Job *job);

    /**
     * Add a job to the unknown dependencies
     * @param job job to add
     */
    void add_unknown_dependency(Job *job);

    /**
     * getter for this->_known_dependencies
     * @return known_dependencies
     */
    std::vector<Job *> known_dependencies();

    /**
     * getter for this->_unknown_dependencies
     * @return unknown_dependencies
     */
    std::vector<Job *> unknown_dependencies();

    /**
     * getter for this->_{un,}known_dependencies in one vector
     * @return list of jobs this job depends on
     */
    std::vector<Job *> dependencies();

    /**
     * getter for this->_known_dependees
     * @return known_dependees
     */
    std::vector<Job *> known_dependees();

    /**
     * getter for this->_unknown_dependees
     * @return unknown_dependees
     */
    std::vector<Job *> unknown_dependees();

    /**
     * getter for this->_{un,}known_dependees in one vector
     * @return list of jobs that depend on this job
     */
    std::vector<Job *> dependees();

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

    bool all_dependencies_finished(int timestamp) const;

    bool finished(int timestamp) const;

    virtual std::string to_string() const override;

    bool depends_on(const Job *job) const;

    Schedule *schedule_at_time(int timestamp);

    void set_atlas_schedule(AtlasSchedule *schedule);
};

