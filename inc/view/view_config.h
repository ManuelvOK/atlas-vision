#pragma once

class ViewConfig {
public:
    class {
        public:
        int width_px = 1000;
        int height_px = 400;
        int margin_x_px = 20;
        int margin_y_px = 20;
    } window;
    class {
        public:
        float width_px = .012;
        float height_px = 20;
    } unit;
    class {
        public:
        int margin_x_px = 0;
        int margin_y_px = 0;
    } job;
    class {
        public:
       const float offset_y_u = 5;
       const float ATLAS_offset_y_u = 0;
       const int ATLAS_grey = 180;
       const float recovery_offset_y_u = 1.3;
       const int recovery_grey = 190;
       const float CFS_offset_y_u = 2.6;
       const int CFS_grey = 200;
    } schedule;
    class {
        public:
        const float margin_x_u = -1.0/30;
        const float margin_y_u = -2.0/20;
        const float width_u = 2.0/30;
        const float height_u = 24.0/20;
    } deadline;
    class {
        public:
        const float width_u = 4.0/30;
        const float height_u = 1;
        const float spacing_u = 2.0/30;
    } color_deadline;
    class {
        public:
        const int offset_y_u = 10;
        int width_u = 0; // this gets computed later
        const int height_u = 1;
        const float grid_height_big_u = 1.0/2;
        const float grid_height_small_u = 1.0/6;
        const float poi_max_height_u = 2.0/3;
        const float poi_width_u = 1.0/3;
        const int grid_grey = 150;
        const int grid_dark_grey = 100;
    } player;

    bool changed = false;

    ViewConfig() : window(), unit(), job(), schedule(), deadline(), color_deadline(), player() {}
};
