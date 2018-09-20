#pragma once

class Cfs_visibility {
public:

    int schedule_id;
    int begin;
    int end;

    Cfs_visibility(int schedule_id, int begin, int end)
        : schedule_id(schedule_id), begin(begin), end(end) {}
    bool is_active_at_time(int timestamp) const ;

};
