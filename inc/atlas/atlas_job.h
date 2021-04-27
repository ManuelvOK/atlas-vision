#pragma once

#include <atlas/atlas_schedule.h>
#include <job.h>

enum class AtlasJobState {
    running,
    ready,
    blocking,
    queued
};

class AtlasSimulationModel;
class AtlasJob : public Job<BaseAtlasSchedule> {
private:
    /** List of Jobs, the execution of this job depends on */
    std::vector<AtlasJob *> _known_dependencies;
    /** List of Jobs, the execution of this job depends on unknowingly */
    std::vector<AtlasJob *> _unknown_dependencies;
    /** list of Jobs, whose execution depends on this job */
    std::vector<AtlasJob *> _known_dependees;
    /** list of Jobs, whose execution depends on this job unknowingly */
    std::vector<AtlasJob *> _unknown_dependees;
public:
    const AtlasSimulationModel *_atlas_model;
    int _execution_time_estimate;   /**< estimated execution time */
    int _dependency_level = -1;     /**< level in dependency DAG. -1 before calculated */

    AtlasJobState _state = AtlasJobState::queued;

    AtlasSchedule *_atlas_schedule = nullptr;


    /**
     * Constructor
     * @param id job id
     * @param deadline timestamp for jobs deadline
     * @param execution_time_estimate estimated time for job execution
     * @param execution_time real execution time
     * @param submission_time timestamp of jobs submission
     */
    AtlasJob(const AtlasSimulationModel *atlas_model, int id, int deadline,
             int execution_time_estimate, int execution_time, int submission_time)
        : Job(id, deadline, execution_time, submission_time), _atlas_model(atlas_model),
        _execution_time_estimate(execution_time_estimate) {}

    /**
     * Add a job to the known dependencies
     * @param job job to add
     */
    void add_known_dependency(AtlasJob *job);

    /**
     * Add a job to the unknown dependencies
     * @param job job to add
     */
    void add_unknown_dependency(AtlasJob *job);

    /**
     * getter for this->_known_dependencies
     * @return known_dependencies
     */
    std::vector<AtlasJob *> known_dependencies();

    /**
     * getter for this->_unknown_dependencies
     * @return unknown_dependencies
     */
    std::vector<AtlasJob *> unknown_dependencies();

    /**
     * getter for this->_{un,}known_dependencies in one vector
     * @return list of jobs this job depends on
     */
    std::vector<AtlasJob *> dependencies();

    /**
     * getter for this->_known_dependees
     * @return known_dependees
     */
    std::vector<AtlasJob *> known_dependees();

    /**
     * getter for this->_unknown_dependees
     * @return unknown_dependees
     */
    std::vector<AtlasJob *> unknown_dependees();

    /**
     * getter for this->_{un,}known_dependees in one vector
     * @return list of jobs that depend on this job
     */
    std::vector<AtlasJob *> dependees();

    /**
     * calculate and set the level of this Job inside the dependency DAG
     * @returns dependency level
     */
    int calculate_dependency_level();

    /* amount of time estimated to still be executed */
    int estimated_execution_time_left(int timestamp) const;

    bool all_dependencies_finished(int timestamp) const;

    bool depends_on(const AtlasJob *job) const;

    void set_atlas_schedule(AtlasSchedule *schedule);

    int time_executed(int timestamp) const override;
};
