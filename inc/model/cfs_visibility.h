#pragma once

class CfsVisibility {
public:

    int schedule_id;
    int begin;
    int end;

    CfsVisibility(int schedule_id, int begin, int end)
        : schedule_id(schedule_id), begin(begin), end(end) {}
    bool is_active_at_time(int timestamp) const ;

};
