#pragma once

#include <models/printable.h>
#include <models/schedule.h>

/** the visibility of an ATLAS schedule fot the CFS scheduler */
class CfsVisibility : public Printable {
public:
    int _schedule_id;               /**< ID of visible schedule */
    Schedule *_schedule = nullptr;  /**< visible schedule */
    int _begin;                     /**< timestand of visibility begin */
    int _end;                       /**< timestand of visibility end */

    /**
     * Constructor
     * @param schedule visible schedule
     * @param begin timestamp of visibility begin
     * @param end timestand of visibility end
     */
    CfsVisibility(Schedule *schedule, int begin, int end)
        : _schedule(schedule), _begin(begin), _end(end) {}

    /**
     * check if the visibility contains a certain timestamp
     * @param timestamp the timestamp to check
     * @returns True if timestamp is between begin and end of visibility. False otherwise.
     */
    bool is_active_at_time(int timestamp) const;

    virtual std::string to_string() const override;
};
