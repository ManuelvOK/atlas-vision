#include <controllers/atlas_controller.h>

#include <SDL_GUI/inc/gui/drawable.h>
#include <SDL_GUI/inc/gui/rgb.h>
#include <SDL_GUI/inc/util/tree.h>

#include <gui/arrow.h>

AtlasController::AtlasController(AtlasModel *atlas_model, InterfaceModel *interface_model, const SDL_GUI::KeyboardInputModel<InputKey> *keyboard_input_model) : _atlas_model(atlas_model), _interface_model(interface_model), _keyboard_input_model(keyboard_input_model) {
    this->init_this();
}

void AtlasController::init_this() {
    this->_interface_model->init_colors(this->_atlas_model->_jobs.size());
    std::map<int, std::vector<int>> submissions;
    int max_submissions = 0;
    std::map<int, std::vector<int>> deadlines;
    int max_deadlines = 0;
    for (const Job *job: this->_atlas_model->_jobs) {
        /* subs */
        submissions[job->_submission_time].push_back(job->_id);
        max_submissions = std::max(max_submissions, static_cast<int>(submissions[job->_submission_time].size()));

        /* dls */
        deadlines[job->_deadline].push_back(job->_id);
        max_deadlines = std::max(max_deadlines, static_cast<int>(deadlines[job->_deadline].size()));
    };

    SDL_GUI::Tree<SDL_GUI::Drawable> *tree = this->_interface_model->drawable_tree();
    SDL_GUI::TreeNode<SDL_GUI::Drawable> *deadline_rect = tree->filter([](SDL_GUI::Drawable *d){return d->has_attribute("deadline");})[0];

    std::function<int(float)> px_width = std::bind(&InterfaceModel::px_width, this->_interface_model, std::placeholders::_1);
    /* create subs */
    for (std::pair<int, std::vector<int>> submissions_at_time: submissions) {
        int submission_position_x = submissions_at_time.first;

        /* TODO: get rid of magic number */
        int offset = deadline_rect->node()->height() - 7 * (submissions_at_time.second.size() - 1);
        for (int job_id: submissions_at_time.second) {
            Arrow *a = new Arrow({0, offset - 1});
            /* recompute the position based on the scale */
            a->add_recalculation_callback([submission_position_x,px_width](SDL_GUI::Drawable *d) {
                    d->set_x(px_width(submission_position_x));
                });
            a->_default_style._color = this->_interface_model->get_color(job_id);
            deadline_rect->add_child(a);
            /* TODO: get rid of magic number */
            offset += 7;
        }
    }

    /* create deadlines */
    for (std::pair<int, std::vector<int>> deadlines_at_time: deadlines) {
        int deadline_position_x = deadlines_at_time.first;

        /* TODO: get rid of magic number */
        int offset = 7 * (deadlines_at_time.second.size() - 1);
        for (int job_id: deadlines_at_time.second) {
            Arrow *a = new Arrow({deadline_position_x, offset-1}, Arrow::direction::DOWN);
            /* recompute the position based on the scale */
            a->add_recalculation_callback([deadline_position_x,px_width](SDL_GUI::Drawable *d) {
                    d->set_x(px_width(deadline_position_x));
                });
            a->_default_style._color = this->_interface_model->get_color(job_id);
            deadline_rect->add_child(a);
            /* TODO: get rid of magic number */
            offset -= 7;
        }
    }
}

void AtlasController::update() {
    if (this->_keyboard_input_model->is_down(InputKey::PLAYER_ZOOM_IN)) {
        this->_interface_model->_unit_width += 0.01;
    }
    if (this->_keyboard_input_model->is_down(InputKey::PLAYER_ZOOM_OUT)) {
        this->_interface_model->_unit_width = std::max<float>(this->_interface_model->_unit_width - 0.01, this->_interface_model->_unit_width_min);
    }
}
