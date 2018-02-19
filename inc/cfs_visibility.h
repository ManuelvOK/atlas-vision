#pragma once

class Cfs_visibility {
public:

    int schedule_id;
    float begin;
    float end;

    Cfs_visibility(int schedule_id, float begin, float end)
        : schedule_id(schedule_id), begin(begin), end(end) {}
    bool is_active_at_time(float timestamp) const ;

};
