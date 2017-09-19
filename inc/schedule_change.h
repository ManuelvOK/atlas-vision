#pragma once

enum class change_type {
    erase = 'd', // delete
    shift = 'b', // begin
    change_execution_time = 'e' //execution time
};

class Schedule_change {
public:
    int schedule_id;
    int timestamp;
    change_type type;
    float value;

    Schedule_change(int schedule_id, int timestamp, char type, float value)
        : schedule_id(schedule_id), timestamp(timestamp), type(static_cast<change_type>(type)),
          value(value) {}
};
