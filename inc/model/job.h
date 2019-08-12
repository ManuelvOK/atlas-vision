#pragma once

#include <vector>
/**
 * Object representing a job
 */
class Job {
public:
    /**
     *   job id
     */
    int id;
    /**
     *   timestamp for the deadline of the job
     */
    int deadline;
    /**
     *   estimated execution time
     */
    int execution_time_estimate;
    /**
     *   real execution time
     */
    int execution_time;
    /**
     *   timestamp the job gets submitted
     */
    int submission_time;

    /**
     * TODO: documentation
     */
    std::vector<Job *> known_dependencies;

    int dependency_level = -1;

    Job(int id, int deadline, int execution_time_estimate, int execution_time, int submission_time)
        : id(id), deadline(deadline), execution_time_estimate(execution_time_estimate),
          execution_time(execution_time), submission_time(submission_time) {}

    int calculate_dependency_level();

};

