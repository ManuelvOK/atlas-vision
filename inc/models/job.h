#pragma once

#include <vector>
/** Object representing a job */
class Job {
public:
    int _id;                        /**< job id */
    int _deadline;                  /**< timestamp for the deadline of the job */
    int _execution_time_estimate;   /**< estimated execution time */
    int _execution_time;            /**< real execution time */
    int _submission_time;           /**< timestamp the job gets submitted */
    int _dependency_level = -1;     /**< level in dependency DAG. -1 before calculated */

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
    Job(int id, int deadline, int execution_time_estimate, int execution_time, int submission_time)
        : _id(id), _deadline(deadline), _execution_time_estimate(execution_time_estimate),
          _execution_time(execution_time), _submission_time(submission_time) {}

    /**
     * calculate and set the level of this Job inside the dependency DAG
     * @returns dependency level
     */
    int calculate_dependency_level();
};

