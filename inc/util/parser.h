#pragma once

#include <map>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#include <models/atlas_model.h>

class CfsVisibility;
class Job;
class Message;
class Schedule;
class ScheduleChange;

/** A Parser for the Simulation input */
class Parser {
  public:
    AtlasModel *_atlas_model;
    int _n_cores;                                   /**< number of cores */
    int _cfs_factor;                                /**< time factor on CFS scheduler */
    std::vector<Job *> _jobs;                       /**< Job descriptions */
    std::vector<ScheduleChange *> _changes;         /**< schedule change descriptions */
    std::vector<CfsVisibility *> _cfs_visibilities; /**< CFS visibility descriptions */
    std::vector<Message *> _messages;               /**< message descriptions */

    /** schedule descriptions, mapped to its timestamp */
    std::map<int, Schedule *> _schedules;

    /** description of dependencies between jobs */
    std::vector<std::tuple<int, int, bool>> _dependencies;

    /**
     * parses a full line of job description input format
     * @param line line to parse
     **/
    void parse_line(std::string line);

    /**
     * parse number of cores from input line
     * @param line line to parse with index at first parameter
     */
    void parse_n_cores(std::stringstream *line);

    /**
     * parse cfs factor from input line
     * @param line line to parse with index at first parameter
     */
    void parse_cfs_factor(std::stringstream *line);

    /**
     * parse job from input line
     * @param line line to parse with index at first parameter
     */
    void parse_job(std::stringstream *line);

    /**
     * parse schedule from input line
     * @param line line to parse with index at first parameter
     */
    void parse_schedule(std::stringstream *line);

    /**
     * parse schedule altering from input line
     * changes are only parsed to apply them later
     * @param line line to parse with index at first parameter
     */
    void parse_change(std::stringstream *line);

    /**
     * parse cfs visibility. This is the foremost ATLAS job that cfs can schedule
     * @param line line to parse with index at first parameter
     */
    void parse_cfs_visibility(std::stringstream *line);

    /**
     * parse message
     * @param line line to parse with index at begin of message
     */
    void parse_message(std::stringstream *line);

    /**
     * parse dependency
     * @param line stringstream that points to first parameter of dependency specification
     **/
    void parse_dependency(std::stringstream *line);
};
