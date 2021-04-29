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
    unsigned _execution_time_estimate;   /**< estimated execution time */
    unsigned _dependency_level = 0;     /**< level in dependency DAG. */
    bool _dependency_level_calculated = false;

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
    AtlasJob(const AtlasSimulationModel *atlas_model, unsigned id, unsigned deadline,
             unsigned execution_time_estimate, unsigned execution_time, unsigned submission_time)
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
    unsigned calculate_dependency_level();

    /* amount of time estimated to still be executed */
    unsigned estimated_execution_time_left(unsigned timestamp) const;

    bool all_dependencies_finished(unsigned timestamp) const;

    bool depends_on(const AtlasJob *job) const;

    void set_atlas_schedule(AtlasSchedule *schedule);

    unsigned time_executed(unsigned timestamp) const override;
};
