#pragma once

#include <util/parser.h>


struct ParsedAtlasSchedule : public ParsedSchedule {
    char _scheduler;

    ParsedAtlasSchedule(unsigned id, unsigned job_id, unsigned core, char scheduler,
                        int submission_time, int begin, unsigned execution_time)
        : ParsedSchedule(id, job_id, core, submission_time, begin, execution_time),
          _scheduler(scheduler) {}
};

struct ParsedVisibility {
    unsigned _schedule_id;
    int _begin;
    int _end;
};

struct ParsedDependency {
    char _type;
    unsigned _dependent_job_id;
    unsigned _dependency_id;
};

class CfsVisibility;
class AtlasJob;
class BaseAtlasSchedule;
class AtlasSimulationModel;
class AtlasParser : public Parser<BaseAtlasSchedule, AtlasJob> {
    unsigned _cfs_factor;                                /**< time factor on CFS scheduler */
    std::vector<ParsedAtlasSchedule> _parsed_schedules;  /**< schedule descriptions */
    std::vector<ParsedVisibility> _visibilities;    /**< CFS visibility descriptions */
    std::vector<ParsedDependency> _dependencies;    /**< dependency descriptions */

    /**< schedule change descriptions mapped to their schedules */
    std::map<unsigned, std::vector<ParsedScheduleChange>> _changes;



    /**
     * parses a full line of job description input format
     * @param line line to parse
     **/
    void parse_line(std::string line, AtlasSimulationModel *atlas_model);

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
     * parse dependency
     * @param line stringstream that points to first parameter of dependency specification
     **/
    void parse_dependency(std::stringstream *line);

  public:
    void parse(std::istream *input, AtlasSimulationModel *atlas_model);
};
