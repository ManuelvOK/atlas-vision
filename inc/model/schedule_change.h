#pragma once

/**
 * Type of schedule change encoded as ascii chars
 */
enum class ChangeType {
    erase = 'd', // delete
    shift = 'b', // begin
    change_execution_time = 'e' //execution time
};

/**
 * Change of a schedule in the simulation
 */
class ScheduleChange {
public:
    int _schedule_id; /**< ID of schedule */
    int _timestamp; /**< timestamp of schedules change */
    ChangeType _type; /**< typ of change */
    int _value; /**< value after change */


    /**
     * Constructor
     *
     * @param schedule_id
     *   ID of schduled
     * @param tiestamp
     *   timestamp of schedule change
     * @param type
     *   type of change
     * @param
     *   value after change
     */
    ScheduleChange(int schedule_id, int timestamp, char type, int value)
        : _schedule_id(schedule_id), _timestamp(timestamp), _type(static_cast<ChangeType>(type)),
          _value(value) {}
};
