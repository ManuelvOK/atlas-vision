#include <cbs/cbs_parser.h>

#include <iostream>

#include <cbs/cbs_job.h>
#include <cbs/cbs_simulation_model.h>
#include <cbs/constant_bandwidth_server.h>

void CbsParser::parse_line(std::string line, CbsSimulationModel *cbs_model) {
    std::stringstream ss(line);
    char type = ' ';
    /* first char in each line specifies type of line to parse */
    ss >> type;
    switch (type) {
        case 'c': this->parse_n_cores(&ss);                break;
        case 'r': this->parse_hard_rt_job(&ss, cbs_model); break;
        case 'j': this->parse_soft_rt_job(&ss, cbs_model); break;
        case 's': this->parse_schedule(&ss);               break;
        case 'a': this->parse_change(&ss);                 break;
        case 'm': this->parse_message(&ss);                break;
        case 'S': this->parse_cbs(&ss, cbs_model);         break;
        case ' ':
        case 0:
        case '#': break;
        default: std::cerr << "Parse error: \"" << type << "\" is not a proper type."
                           << std::endl; break;
    }
}

void CbsParser::parse_hard_rt_job(std::stringstream *line, CbsSimulationModel *cbs_model) {
    unsigned id, deadline, execution_time, submission_time;
    *line >> id >> deadline >> execution_time >> submission_time;
    cbs_model->add_job(new HardRtJob(id, deadline, execution_time, submission_time));
}

void CbsParser::parse_soft_rt_job(std::stringstream *line, CbsSimulationModel *cbs_model) {
    unsigned id, execution_time, submission_time, cbs_id;
    *line >> id >> execution_time >> submission_time >> cbs_id;
    cbs_model->add_job(new SoftRtJob(id, execution_time, submission_time));
    this->_cbs_memberships[id] = cbs_id;
}

void CbsParser::parse_schedule(std::stringstream *line) {
    (void) line;
    /* TODO: implement */
}

void CbsParser::parse_change(std::stringstream *line) {
    (void) line;
    /* TODO: implement */
}

void CbsParser::parse_cbs(std::stringstream *line, CbsSimulationModel *cbs_model) {
    unsigned id, budget, period;
    *line >> id >> budget >> period;
    cbs_model->_servers.emplace(id, ConstantBandwidthServer{id, budget, period});
}


void CbsParser::parse(std::istream *input, CbsSimulationModel *cbs_model) {
    /* parse input. Jobs and Cbs' get directly created */
    std::string line;
    while (std::getline(*input, line)) {
        this->parse_line(line, cbs_model);
    }

    cbs_model->_n_cores = this->_n_cores;

    /* Add Jobs to Cbs */
    for (SoftRtJob *job: cbs_model->_soft_rt_jobs) {
        unsigned cbs_id = this->_cbs_memberships[job->_id];
        job->_cbs = &cbs_model->_servers.at(cbs_id);
    }

}
