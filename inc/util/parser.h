#pragma once

#include <map>
#include <string>
#include <sstream>
#include <utility>
#include <vector>


class Message;
class CbsSimulationModel;

struct ParsedSchedule {
    unsigned _id;
    unsigned _job_id;
    unsigned _core;
    int _submission_time;
    int _begin;
    unsigned _execution_time;
};

/** Change of a schedule in the simulation */
struct ParsedScheduleChange {
    unsigned _schedule_id;   /**< ID of schedule */
    int _timestamp;     /**< timestamp of schedules change */
    char _type;         /**< typ of change */
    int _value;         /**< value after change */
};

/** Type of schedule change encoded as ascii chars */
enum class ChangeType {
    erase = 'd', // delete
    shift = 'b', // begin
    change_execution_time = 'e' //execution time
};

class BaseParser {
  protected:
    unsigned _n_cores;                                   /**< number of cores */
    std::vector<Message *> _messages;       /**< generated messages */

    /**
     * parse number of cores from input line
     * @param line line to parse with index at first parameter
     */
    void parse_n_cores(std::stringstream *line);

    /**
     * parse message
     * @param line line to parse with index at begin of message
     */
    void parse_message(std::stringstream *line);
};

/** A Parser for the Simulation input */
template <typename S, typename J>
class Parser : public BaseParser {
  protected:
    std::map<unsigned, J *> _jobs;             /**< generated jobs mapped to their id */
    std::map<unsigned, S *> _schedules;        /**< generated schedules mapped to their id */
};
