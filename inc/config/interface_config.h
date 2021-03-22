#pragma once

/** Values that are used for the visual appereance of this application */
const class {
public:
    const class { /* time to px conversion */
    public:
        float width_px = .012;
        float height_px = 15;
    } unit;
    const class { /* player related */
    public:
        const int grid_grey = 150;
        const int grid_distance = 250;
        const int grid_dark_grey = 100;
        const int grid_dark_distance = 4;

        const int scheduler_distance_px = 5;
        const int core_distance_px = 5;
        const int arrow_distance_px = 7;
    } player;

    bool changed = false;

} interface_config;
