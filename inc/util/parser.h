#pragma once

#include <map>
#include <string>
#include <sstream>
#include <utility>
#include <vector>


class CfsVisibility;
class AtlasJob;
class Message;
class BaseAtlasSchedule;
class AtlasSimulationModel;
class CbsSimulationModel;

/** Type of schedule change encoded as ascii chars */
enum class ChangeType {
    erase = 'd', // delete
    shift = 'b', // begin
    change_execution_time = 'e' //execution time
};

/** Change of a schedule in the simulation */
struct ParsedChange {
    int _schedule_id;   /**< ID of schedule */
    int _timestamp;     /**< timestamp of schedules change */
    char _type;         /**< typ of change */
    int _value;         /**< value after change */
};

struct ParsedSchedule {
    int _id;
    int _job_id;
    int _core;
    char _scheduler;
    int _submission_time;
    int _begin;
    int _execution_time;
};

struct ParsedVisibility {
    int _schedule_id;
    int _begin;
    int _end;
};

struct ParsedDependency {
    char _type;
    int _dependent_job_id;
    int _dependency_id;
};

/** A Parser for the Simulation input */
class Parser {
    int _n_cores;                                   /**< number of cores */
    int _cfs_factor;                                /**< time factor on CFS scheduler */
    std::vector<ParsedSchedule> _parsed_schedules;  /**< schedule descriptions */
    std::vector<ParsedVisibility> _visibilities;    /**< CFS visibility descriptions */
    std::vector<ParsedDependency> _dependencies;    /**< dependency descriptions */

    /**< schedule change descriptions mapped to their schedules */
    std::map<int, std::vector<ParsedChange>> _changes;

    std::map<int, AtlasJob *> _jobs;             /**< generated jobs mapped to their id */
    std::map<int, BaseAtlasSchedule *> _schedules;   /**< generated schedules mapped to their id */
    std::vector<Message *> _messages;       /**< generated messages */

    /**
     * parses a full line of job description input format
     * @param line line to parse
     **/
    void parse_line(std::string line, AtlasSimulationModel *atlas_model);

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
    void parse_job(std::stringstream *line, AtlasSimulationModel *atlas_model);

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

public:
    void parse(std::istream *input, AtlasSimulationModel *atlas_model);
    void parse(std::istream *input, CbsSimulationModel *atlas_model);
};
