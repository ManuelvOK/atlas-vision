#pragma once

/** the visibility of an ATLAS schedule fot the CFS scheduler */
class CfsVisibility {
public:
    int _schedule_id;   /**< ID of visible schedule */
    int _begin;         /**< timestand of visibility begin */
    int _end;           /**< timestand of visibility end */

    /**
     * Constructor
     * @param schedule_id ID of visible schedule
     * @param begin timestamp of visibility begin
     * @param end timestand of visibility end
     */
    CfsVisibility(int schedule_id, int begin, int end)
        : _schedule_id(schedule_id), _begin(begin), _end(end) {}

    /**
     * check if the visibility contains a certain timestamp
     * @param timestamp the timestamp to check
     * @returns True if timestamp is between begin and end of visibility. False otherwise.
     */
    bool is_active_at_time(int timestamp) const;
};
