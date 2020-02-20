#include <util/parser.h>

#include <iostream>
#include <string>
#include <sstream>

#include <models/cfs_visibility.h>
#include <models/job.h>
#include <models/message.h>
#include <models/schedule.h>
#include <models/schedule_change.h>

void Parser::parse_line(std::string line) {
    std::stringstream ss(line);
    char type = ' ';
    /* first char in each line specifies type of line to parse */
    ss >> type;
    switch (type) {
        case 'c': this->parse_n_cores(&ss);          break;
        case 'j': this->parse_job(&ss);              break;
        case 's': this->parse_schedule(&ss);         break;
        case 'a': this->parse_change(&ss);           break;
        case 'v': this->parse_cfs_visibility(&ss);   break;
        case 'm': this->parse_message(&ss);          break;
        case 'd': this->parse_dependency(&ss); break;
        case ' ':
        case 0:
        case '#': break;
        default: std::cerr << "Parse error: \"" << type << "\" is not a proper type."
                 << std::endl; break;
    }
}


void Parser::parse_n_cores(std::stringstream *line) {
    *line >> this->_n_cores;
}

void Parser::parse_job(std::stringstream *line) {
    int id, deadline, time_estimate, time, submission;
    *line >> id >> deadline >> time_estimate >> time >> submission;
    Job *job = new Job(id, deadline, time_estimate, time, submission);
    this->_jobs.emplace_back(job);
}

void Parser::parse_schedule(std::stringstream *line) {
    int id, job_id, core;
    int submission_time, begin, time;
    char scheduler;
    *line >> id >> job_id >> core >> scheduler >> submission_time >> begin >> time;
    Schedule *schedule = new Schedule(id, job_id, core, scheduler, submission_time, begin, time);
    this->_schedules.emplace(id, schedule);
}

void Parser::parse_change(std::stringstream *line) {
    int schedule_id;
    int timestamp;
    int value = -1;
    char type;
    *line >> type >> timestamp >> schedule_id;
    if (static_cast<ChangeType>(type) != ChangeType::erase) {
        *line >> value;
    }
    ScheduleChange *change = new ScheduleChange(schedule_id, timestamp, type, value);
    this->_changes.emplace_back(change);
}

void Parser::parse_cfs_visibility(std::stringstream *line) {
    int schedule_id;
    int begin, end;
    *line >> schedule_id >> begin >> end;
    CfsVisibility *visibility = new CfsVisibility(schedule_id, begin, end);
    this->_cfs_visibilities.emplace_back(visibility);
}

void Parser::parse_message(std::stringstream *line) {
    int timestamp;
    *line >> timestamp;
    int pos = line->tellg();
    std::string message = line->str();
    message = message.substr(message.find_first_not_of(" ", pos));
    Message *m = new Message(timestamp, message);
    this->_messages.emplace_back(m);
}

void Parser::parse_dependency(std::stringstream *line) {
    int job1 = -1;
    int job2 = -1;
    *line >> job1 >> job2;
    this->_dependencies.emplace_back(job1, job2);
}
