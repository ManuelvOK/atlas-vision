#pragma once

enum class ChangeType {
    erase = 'd', // delete
    shift = 'b', // begin
    change_execution_time = 'e' //execution time
};

class ScheduleChange {
public:
    int schedule_id;
    int timestamp;
    ChangeType type;
    int value;

    ScheduleChange(int schedule_id, int timestamp, char type, int value)
        : schedule_id(schedule_id), timestamp(timestamp), type(static_cast<ChangeType>(type)),
          value(value) {}
};
