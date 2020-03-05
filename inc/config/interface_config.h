#pragma once

const class {
public:
    const class {
        public:
        float width_px = .012;
        float height_px = 20;
    } unit;
    const class {
        public:
        int margin_x_px = 0;
        int margin_y_px = 0;
    } job;
    const class {
       public:
       const float offset_y_u = 5;
       const float ATLAS_offset_y_u = 0;
       const float recovery_offset_y_u = 1.3;
       const float CFS_offset_y_u = 2.6;
    } schedule;
    const class {
        public:
        const float width_u = 4.0/30;
        const float height_u = 1;
        const float spacing_u = 2.0/30;
    } color_deadline;
    const class {
        public:
        const int offset_y_u = 10;
        int width_u = 0; // this gets computed later
        const int width_px = 580;
        const int height_u = 1;
        const int height_px = 580;
        const float grid_height_big_u = 1.0/2;
        const float grid_height_small_u = 1.0/6;
        const float poi_max_height_u = 2.0/3;
        const float poi_width_u = 1.0/3;
        const int grid_grey = 150;
        const int grid_dark_grey = 100;
    } player;

    bool changed = false;

} interface_config;
