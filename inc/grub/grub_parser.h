#pragma once

#include <util/parser.h>

struct ParsedGrubSchedule : public ParsedSchedule {
    bool _is_hard_rt;
};

class GrubSimulationModel;
class GrubSchedule;
class GrubJob;
class GrubParser : public Parser<GrubSchedule, GrubJob> {
    std::vector<ParsedGrubSchedule> _parsed_schedules;
    std::map<int, std::vector<ParsedScheduleChange>> _changes;
    std::map<unsigned, unsigned> _cbs_memberships;

    void parse_line(std::string line, GrubSimulationModel *model);

    void parse_hard_rt_job(std::stringstream *line, GrubSimulationModel *model);
    void parse_soft_rt_job(std::stringstream *line, GrubSimulationModel *model);

    void parse_schedule(std::stringstream *line);

    void parse_change(std::stringstream *line);

    void parse_cbs(std::stringstream *line, GrubSimulationModel *model);

  public:
    void parse(std::istream *input, GrubSimulationModel *model);
};

