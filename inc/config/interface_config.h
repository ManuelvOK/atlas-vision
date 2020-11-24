#pragma once

/** Values that are used for the visual appereance of this application */
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
       const int ATLAS_offset_y = 10;
       const int recovery_offset_y = 40;
       const int CFS_offset_y = 70;
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
        const int grid_grey = 150;
        const int grid_distance = 250;
        const int grid_dark_grey = 100;
        const int grid_dark_distance = 4;
    } player;

    bool changed = false;

} interface_config;
