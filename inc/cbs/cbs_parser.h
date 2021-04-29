#pragma once

#include <util/parser.h>

struct ParsedCbsSchedule : public ParsedSchedule {
    bool _is_hard_rt;
};

class CbsSchedule;
class CbsJob;
class CbsParser : public Parser<CbsSchedule, CbsJob> {
    std::vector<ParsedCbsSchedule> _parsed_schedules;
    std::map<unsigned, std::vector<ParsedScheduleChange>> _changes;
    std::map<unsigned, unsigned> _cbs_memberships;

    void parse_line(std::string line, CbsSimulationModel *cbs_model);

    void parse_hard_rt_job(std::stringstream *line, CbsSimulationModel *cbs_model);
    void parse_soft_rt_job(std::stringstream *line, CbsSimulationModel *cbs_model);

    void parse_schedule(std::stringstream *line);

    void parse_change(std::stringstream *line);

    void parse_cbs(std::stringstream *line, CbsSimulationModel *cbs_model);

  public:
    void parse(std::istream *input, CbsSimulationModel *cbs_model);
};
