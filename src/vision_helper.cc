#include <schedule_rect.h>
#if 0

int get_hovered_job(int x, int y) {
    for (Schedule_rect &s: viewmodel.EDF_schedules) {
        if (point_inside_rect(x, y, s.render_position())) {
            return s.job_id;
        }
    }
    for (Schedule_rect &s: viewmodel.schedules) {
        if (point_inside_rect(x, y, s.render_position())) {
            return s.job_id;
        }
    }
    return -1;
}



float position_in_player(int x, int y) {
}


void update_window(int width, int height) {
    config.window.width_px = width;
    config.window.height_px = height;
    recompute_config();
}

#endif
