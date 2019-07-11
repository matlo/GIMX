/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#include <cursesw.h>
#else
#include <ncursesw/ncurses.h>
#endif

#include "display.h"

#include "calibration.h"
#include <gimxinput/include/ginput.h>
#include "gimx.h"
#include <controller.h>
#include <stats.h>

#define CROSS_CHAR '*'
#define SHIFT_ESC _("Press Shift+Esc to exit.")

#ifndef WIN32
#define STICK_Y_L 11
#else
#define STICK_Y_L 13
#endif
#define STICK_X_L 21

#define BUTTON_LENGTH sizeof("shifter forward: 65535")

#define BUTTON_Y_L STICK_Y_L
#define BUTTON_X_L BUTTON_LENGTH + 1

#define LSTICK_Y_P 2
#define LSTICK_X_P 2

#define RSTICK_Y_P LSTICK_Y_P
#define RSTICK_X_P LSTICK_X_P + STICK_X_L + 1

#define BUTTON_Y_P LSTICK_Y_P
#define BUTTON_X_P RSTICK_X_P + STICK_X_L + 1

#define CAL_Y_P LSTICK_Y_P + STICK_Y_L
#define CAL_X_P 2


struct gwindow
{

    struct {
        int x;
        int y;
    } origin;
    struct {
        int x;
        int y;
    } size;
    bool use_border;
    WINDOW* window_handle;
    void (*draw)(struct gwindow *, struct gcalibration * ) ; // function drawing stuff inside the window
};


struct gstep
{
    struct gwindow windows[GWINDOWS_MAX];
};

struct gmode
{

    struct gstep steps[GSTEPS_MAX];
};

struct ggui {
    struct gmode gmodes[GMODES_MAX];
};

e_cal_modes prev_mode = MODE_STATUS;
e_cal_steps prev_step = STEP_1;



/*
 * Draws statuses visible in every step expect in MODE_STATUS
 * In separated function and it doesn't need window so it doesn't need to be included in every step
 * Also text is dynamically positioned basing on window size so it couldn't be included in normal function
 */
static void draw_statuses(struct gcalibration *calibration) {
    //static const char *editModeStringMap[] = { "Disabled", "Enabled"};
    static const char *modes_string_map[] = { "Basic Mode", "Advanced Mode", "Expert Mode" };
    static int step_text_len = strlen("Current step: 5 of 5"); //
    static int next_step_text_len = strlen("Prev. Step (F1) | Next Step (F4)");

    //mvprintw(LINES-2, 1, "Edit mode: %s (Press F1 to change)", editModeStringMap[calibration->isEditEnabled]);
    mvprintw(LINES - 2, 1, "Prev. Parameter (F2) | Next Parameter (F3)");
    mvprintw(LINES - 1, 1, "Current calibration mode: %s", modes_string_map[calibration->cal_mode]);

    // Starts from right bottom
    mvprintw(LINES - 1, COLS - step_text_len - 1, "Current step: %d of %d", (calibration->cal_step) + 1,
            calibration->cal_modes_max_step[calibration->cal_mode]);
    mvprintw(LINES - 2, COLS - next_step_text_len - 1, "Prev. Step (F1) | Next Step (F4)");
}

static WINDOW* draw_window(int size_y, int size_x, int window_y, int window_x, bool use_border) {
    WINDOW* newWindow = newwin(size_y, size_x, window_y, window_x);
    if (use_border)
        box(newWindow, 0, 0);

    return newWindow;
}

static void draw_status_title(struct gwindow *window, struct gcalibration *calibration __attribute__((unused))) {

    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    static int title_text_len = strlen("Game Input MultipleXer Calibration Tool");
    static int title_choose_text_len = strlen("Choose calibration mode from list");

    mvwprintw(window->window_handle, 1, (window->size.x / 2) - title_text_len / 2,
            "Game Input MultipleXer Calibration Tool");
    mvwprintw(window->window_handle, 2, (window->size.x / 2) - title_choose_text_len / 2,
            "Choose calibration mode from list");

    wnoutrefresh(window->window_handle);
}

static void draw_status_menu(struct gwindow *window, struct gcalibration *calibration __attribute__((unused))) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    // Window label outside current window
    mvprintw(window->origin.y - 1, window->origin.x + 1, "Available modes:");

    mvwprintw(window->window_handle, 1, 1, "Basic Mode (Ctrl + F1)");
    mvwprintw(window->window_handle, 2, 1, "Advanced Mode (Ctrl + F2)");
    mvwprintw(window->window_handle, 3, 1, "Expert Mode (Ctrl + F3)");

    wnoutrefresh(window->window_handle);
}

static void draw_status_description(struct gwindow *window, struct gcalibration *calibration __attribute__((unused))) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    // Window label outside current window
    mvprintw(window->origin.y - 1, window->origin.x + 1, "Description:");
    wnoutrefresh(window->window_handle);
}

// Hack to get multiline text working with window border. It's just window in window with smaller size and position
static void draw_status_description_inner(struct gwindow *window,
        struct gcalibration *calibration __attribute__((unused))) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    static const char* test_text =
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum mollis in ex vel volutpat. Nulla quis neque elit. Ut laoreet mauris nec mi dictum ultricies. Proin luctus nibh quis nisi gravida, in hendrerit tortor mollis. In hac habitasse platea dictumst. Aenean ac dapibus eros. Curabitur eget augue eu lorem euismod efficitur. Donec et arcu ut justo consectetur aliquam. Integer fringilla blandit est et consequat.\n\nInteger eget augue dui. In ac cursus eros. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean quis magna eu nibh mollis vehicula. Curabitur laoreet lectus sed tristique efficitur. Maecenas pellentesque, erat et faucibus lacinia, lorem nunc malesuada mauris, a vestibulum arcu dolor eu ex. Morbi consectetur pellentesque nunc et pulvinar. Suspendisse id gravida quam. Sed tincidunt, ligula faucibus pharetra rutrum, justo justo sollicitudin lacus, eu suscipit mauris odio ac arcu. Duis lobortis quam a mauris lobortis, vehicula tempor mauris dictum. Quisque vehicula varius lectus nec iaculis. Phasellus porttitor fermentum tristique. Duis massa erat, interdum ornare erat in, viverra sodales felis. Suspendisse potenti.";

    mvwaddstr(window->window_handle, 0, 0, test_text);
    wnoutrefresh(window->window_handle);
}

static void draw_status_some_text(struct gwindow *window, struct gcalibration *calibration __attribute__((unused))) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    // Window label outside current window
    mvwprintw(window->window_handle, 0, 0, "Maybe some text here?\nVersion maybe or something");
    wnoutrefresh(window->window_handle);
}

static void draw_step1_profile(struct gwindow *window, struct gcalibration *calibration) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    mvprintw(window->origin.y - 1, window->origin.x + 1, "Current parameter:");

    if (*calibration->current_cal == CC) {
        wattron(window->window_handle, COLOR_PAIR(4));
    }

    mvwprintw(window->window_handle, 1, 1, "Profile: %d  ", *calibration->config);
    wnoutrefresh(window->window_handle);

    if (*calibration->current_cal == CC) {
        wattron(window->window_handle, COLOR_PAIR(1));
    }

}

static void draw_step1_description(struct gwindow *window, struct gcalibration *calibration __attribute__((unused))) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    // Window label outside current window
    mvprintw(window->origin.y - 1, window->origin.x + 1, "Description:");
    wnoutrefresh(window->window_handle);
}

static void draw_step1_description_inner_profile(struct gwindow *window,
        struct gcalibration *calibration __attribute__((unused))) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    static const char* test_text =
            "Profile selection allows user to have something blah blah. Proin luctus nibh quis nisi gravida, in hendrerit tortor mollis. In hac habitasse platea dictumst. Aenean ac dapibus eros. Curabitur eget augue eu lorem euismod efficitur. Donec et arcu ut justo consectetur aliquam. Integer fringilla blandit est et consequat.\n\nInteger eget augue dui. In ac cursus eros. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean quis magna eu nibh mollis vehicula. Curabitur laoreet lectus sed tristique efficitur. Maecenas pellentesque, erat et faucibus lacinia, lorem nunc malesuada mauris, a vestibulum arcu dolor eu ex. Morbi consectetur pellentesque nunc et pulvinar. Suspendisse id gravida quam. Sed tincidunt, ligula faucibus pharetra rutrum, justo justo sollicitudin lacus, eu suscipit mauris odio ac arcu. Duis lobortis quam a mauris lobortis, vehicula tempor mauris dictum. Quisque vehicula varius lectus nec iaculis. Phasellus porttitor fermentum tristique. Duis massa erat, interdum ornare erat in, viverra sodales felis. Suspendisse potenti.";

    mvwaddstr(window->window_handle, 0, 0, test_text);
    wnoutrefresh(window->window_handle);
}

static void draw_step2_sensitivity(struct gwindow *window, struct gcalibration *calibration) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);
    // TODO: HANDLE NULL
    mvprintw(window->origin.y - 1, window->origin.x + 1, "Current parameter:");

    if (*calibration->current_cal == MX) {
        wattron(window->window_handle, COLOR_PAIR(4));
    }

    mvwprintw(window->window_handle, 1, 1, "Sensitivity: %.2f ", *calibration->mouse_cal->mx);
    wnoutrefresh(window->window_handle);

    if (*calibration->current_cal == MX) {
        wattron(window->window_handle, COLOR_PAIR(1));
    }
}

static void draw_step2_description(struct gwindow *window, struct gcalibration *calibration __attribute__((unused))) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    // Window label outside current window
    mvprintw(window->origin.y - 1, window->origin.x + 1, "Description:");
    wnoutrefresh(window->window_handle);
}

static void draw_step2_description_inner_sensitivity(struct gwindow *window,
        struct gcalibration *calibration __attribute__((unused))) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    static const char* test_text =
            "You can tune sensitivity here matey.In hac habitasse platea dictumst. Aenean ac dapibus eros. Curabitur eget augue eu lorem euismod efficitur. Donec et arcu ut justo consectetur aliquam. Integer fringilla blandit est et consequat.\n\nInteger eget augue dui. In ac cursus eros. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean quis magna eu nibh mollis vehicula. Curabitur laoreet lectus sed tristique efficitur. Maecenas pellentesque, erat et faucibus lacinia, lorem nunc malesuada mauris, a vestibulum arcu dolor eu ex. Morbi consectetur pellentesque nunc et pulvinar. Suspendisse id gravida quam. Sed tincidunt, ligula faucibus pharetra rutrum, justo justo sollicitudin lacus, eu suscipit mauris odio ac arcu. Duis lobortis quam a mauris lobortis, vehicula tempor mauris dictum. Quisque vehicula varius lectus nec iaculis. Phasellus porttitor fermentum tristique. Duis massa erat, interdum ornare erat in, viverra sodales felis. Suspendisse potenti.";

    mvwaddstr(window->window_handle, 0, 0, test_text);
    wnoutrefresh(window->window_handle);
}

static void draw_step3_dzx_dzy(struct gwindow *window, struct gcalibration *calibration) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);
    // TODO: HANDLE NULL
    mvprintw(window->origin.y - 1, window->origin.x + 1, "Current parameter:");

    if (*calibration->current_cal == DZX) {
        wattron(window->window_handle, COLOR_PAIR(4));
    }

    mvwprintw(window->window_handle, 1, 1, "DZX: %d ", *calibration->mouse_cal->dzx);

    if (*calibration->current_cal == DZX) {
        wattron(window->window_handle, COLOR_PAIR(1));
    }

    if (*calibration->current_cal == DZY) {
        wattron(window->window_handle, COLOR_PAIR(4));
    }

    mvwprintw(window->window_handle, 3, 1, "DZY: %d  ", *calibration->mouse_cal->dzy);

    if (*calibration->current_cal == DZY) {
        wattron(window->window_handle, COLOR_PAIR(1));
    }

    wnoutrefresh(window->window_handle);
}

static void draw_step3_description(struct gwindow *window, struct gcalibration *calibration __attribute__((unused))) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    // Window label outside current window
    mvprintw(window->origin.y - 1, window->origin.x + 1, "Description:");
    wnoutrefresh(window->window_handle);
}

static void draw_step3_description_innerdzxdzy(struct gwindow *window,
        struct gcalibration *calibration __attribute__((unused))) {
    window->window_handle = draw_window(window->size.y, window->size.x, window->origin.y, window->origin.x,
            window->use_border);

    static const char* test_text = "Something something";

    mvwaddstr(window->window_handle, 0, 0, test_text);
    wnoutrefresh(window->window_handle);
}

// TODO, move this to display.h

#define GUI_MAX_X 85
#define GUI_MAX_Y 24

#define GUI_TYPICAL_TEXT_ORIGIN_X 0
#define GUI_TYPICAL_TEXT_ORIGIN_Y 3
#define GUI_TYPICAL_DESCRIPTION_ORIGIN_X 34
#define GUI_TYPICAL_DESCRIPTION_ORIGIN_Y 3
#define GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_X 35
#define GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_Y 4

#define GUI_TYPICAL_TEXT_SIZE_X 25
#define GUI_TYPICAL_TEXT_SIZE_Y 10
#define GUI_TYPICAL_DESCRIPTION_SIZE_X 46
#define GUI_TYPICAL_DESCRIPTION_SIZE_Y 16
#define GUI_TYPICAL_DESCRIPTION_INNER_SIZE_X 44
#define GUI_TYPICAL_DESCRIPTION_INNER_SIZE_Y 14

struct ggui guid =
{
    .gmodes =
    {
            // MODE_BASIC
        {
            .steps =
            {
                {
                    .windows =
                    {
                        {
                            .origin = { .x = GUI_TYPICAL_TEXT_ORIGIN_X, .y = GUI_TYPICAL_TEXT_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_TEXT_SIZE_X, .y = GUI_TYPICAL_TEXT_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step1_profile
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step1_description
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_Y },
                            .use_border = 0,
                            .draw = draw_step1_description_inner_profile
                        },

                    },
                },
                {
                    .windows =
                    {
                        {
                            .origin = { .x = GUI_TYPICAL_TEXT_ORIGIN_X, .y = GUI_TYPICAL_TEXT_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_TEXT_SIZE_X, .y = GUI_TYPICAL_TEXT_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step2_sensitivity
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step2_description
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_Y },
                            .use_border = 0,
                            .draw = draw_step2_description_inner_sensitivity
                        },

                    },
                 },
            },
        },
        {    // MODE_ADVANCED
            .steps =
            {
                {
                    .windows =
                    {
                        {
                            .origin = { .x = GUI_TYPICAL_TEXT_ORIGIN_X, .y = GUI_TYPICAL_TEXT_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_TEXT_SIZE_X, .y = GUI_TYPICAL_TEXT_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step1_profile
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step1_description
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_Y },
                            .use_border = 0,
                            .draw = draw_step1_description_inner_profile
                        },

                    },
                },
                {
                    .windows =
                    {
                        {
                            .origin = { .x = GUI_TYPICAL_TEXT_ORIGIN_X, .y = GUI_TYPICAL_TEXT_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_TEXT_SIZE_X, .y = GUI_TYPICAL_TEXT_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step2_sensitivity
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step2_description
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_Y },
                            .use_border = 0,
                            .draw = draw_step2_description_inner_sensitivity
                        },

                    },
                 },
                 {
                     .windows =
                     {
                         {
                             .origin = { .x = GUI_TYPICAL_TEXT_ORIGIN_X, .y = GUI_TYPICAL_TEXT_ORIGIN_Y },
                             .size = { .x = GUI_TYPICAL_TEXT_SIZE_X, .y = GUI_TYPICAL_TEXT_SIZE_Y },
                             .use_border = 1,
                             .draw = draw_step3_dzx_dzy
                         },
                         {

                             .origin = { .x = GUI_TYPICAL_DESCRIPTION_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_ORIGIN_Y },
                             .size = { .x = GUI_TYPICAL_DESCRIPTION_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_SIZE_Y },
                             .use_border = 1,
                             .draw = draw_step3_description
                         },
                         {

                             .origin = { .x = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_Y },
                             .size = { .x = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_Y },
                             .use_border = 0,
                             .draw = draw_step3_description_innerdzxdzy
                         },

                    },
                 },
            },
        },
        {    // MODE_EXPERT
            .steps =
            {
                {
                    .windows =
                    {
                        {
                            .origin = { .x = GUI_TYPICAL_TEXT_ORIGIN_X, .y = GUI_TYPICAL_TEXT_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_TEXT_SIZE_X, .y = GUI_TYPICAL_TEXT_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step1_profile
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step1_description
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_Y },
                            .use_border = 0,
                            .draw = draw_step1_description_inner_profile
                        },

                    },
                },
                {
                    .windows =
                    {
                        {
                            .origin = { .x = GUI_TYPICAL_TEXT_ORIGIN_X, .y = GUI_TYPICAL_TEXT_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_TEXT_SIZE_X, .y = GUI_TYPICAL_TEXT_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step2_sensitivity
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_SIZE_Y },
                            .use_border = 1,
                            .draw = draw_step2_description
                        },
                        {

                            .origin = { .x = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_ORIGIN_Y },
                            .size = { .x = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_X, .y = GUI_TYPICAL_DESCRIPTION_INNER_SIZE_Y },
                            .use_border = 0,
                            .draw = draw_step2_description_inner_sensitivity
                        },

                    },
                 },
            },
        },
        {    // MODE_STATUS
            .steps =
            {
                {
                    .windows =
                    {
                        {

                            .origin = { .x = 0, .y = 0 },
                            .size = { .x = 80, .y = 4 },
                            .use_border = 1,
                            .draw = draw_status_title
                        },
                        {

                            .origin = { .x = 0, .y = 6 },
                            .size = { .x = 28, .y = 10 },
                            .use_border = 1,
                            .draw = draw_status_menu
                        },
                        {

                            .origin = { .x = 34, .y = 6 },
                            .size = { .x = 46, .y = 15 },
                            .use_border = 1,
                            .draw = draw_status_description
                        },
                        {

                            .origin = { .x = 35, .y = 7 },
                            .size = { .x = 44, .y = 13 },
                            .use_border = 0,
                            .draw = draw_status_description_inner
                        },
                        {

                            .origin = { .x = 1, .y = 21 },
                            .size = { .x = 30, .y = 3 },
                            .use_border = 0,
                            .draw = draw_status_some_text
                        },
                    },
                },
            },
        },
    }
};




static WINDOW *lstick = NULL, *rstick = NULL, *wbuttons = NULL, *wcal __attribute__((unused)) = NULL;


static int cross[2][2] = { {STICK_X_L / 2, STICK_Y_L / 2}, {STICK_X_L / 2, STICK_Y_L / 2} };


void display_refresh(struct gcalibration *calibration) {
    // If mode or step were switched
    if (prev_mode != calibration->cal_mode || prev_step != calibration->cal_step) {
        // Clear windows from previous mode to prevent memory leak
        for (int i = 0; i < GWINDOWS_MAX; i++) {

            if (guid.gmodes[prev_mode].steps[prev_step].windows[i].window_handle != NULL) {
                delwin(guid.gmodes[prev_mode].steps[prev_step].windows[i].window_handle);
                guid.gmodes[prev_mode].steps[prev_step].windows[i].window_handle = NULL;
            }
        }

        prev_mode = calibration->cal_mode;
        prev_step = calibration->cal_step;
    }

    // Clear windows from current mode to allow redraw
    for (int i = 0; i < GWINDOWS_MAX; i++) {
        if (guid.gmodes[calibration->cal_mode].steps[calibration->cal_step].windows[i].window_handle != NULL) {

            delwin(guid.gmodes[calibration->cal_mode].steps[calibration->cal_step].windows[i].window_handle);
            guid.gmodes[prev_mode].steps[prev_step].windows[i].window_handle = NULL;
        }
    }

    clear(); // Clear whole screen (werase could leave some crap if window size changes)
    refresh(); // Update screen to blank

}

void display_calibration(struct gcalibration *calibration) {

    display_refresh(calibration);

    // Display curr step/mode/editMode on every mode except initial screen
    if (calibration->cal_mode != MODE_STATUS)
        draw_statuses(calibration);

    // Step through every window and execute draw function
    for (int i = 0; i < GWINDOWS_MAX; i++) {
        struct gwindow *curr_window = &guid.gmodes[calibration->cal_mode].steps[calibration->cal_step].windows[i];

        // Check if window element is empty, newwin would crash on empty
        if (curr_window->size.x != 0 && curr_window->size.y != 0)
            curr_window->draw(curr_window, calibration); // Run function inside that struct
    }

    doupdate(); // Update screen to reflect windows data


  /*
    char line[COLS];
    s_mouse_cal* mcal = cal_get_mouse(*calibration->mouse, *calibration->config);
    if(calibration->cal_mode == MODE_BASIC)
  {
      // List through windows for each step
      for( int i = 0; i < GWINDOWS_MAX; i++ )
      {
          // Check if window element is empty
          if(guid.gmodes[*calibration->cal_mode].steps[*calibration->cal_step].windows[i].size.x != 0 && guid.gmodes[*calibration->cal_mode].steps[*calibration->cal_step].windows[i].size.y != 0)
              guid.gmodes[*calibration->cal_mode].steps[*calibration->cal_step].windows[i].draw(&guid.gmodes[*calibration->cal_mode].steps[*calibration->cal_step].windows[i], calibration); // Run function inside that struct
      }


      //guid.gmodes[0].steps[0].windows[0].draw(&guid.gmodes[0].steps[0].windows[0], calibration);
      //guid.gmodes[0].steps[0].windows[1].draw(&guid.gmodes[0].steps[0].windows[1], calibration);
  }
  else
    {
      guid.gmodes[1].steps[0].windows[0].draw(&guid.gmodes[0].steps[0].windows[0], calibration);
      guid.gmodes[1].steps[0].windows[1].draw(&guid.gmodes[0].steps[0].windows[1], calibration);
    }*/
  //mvprintw(LINES-2, 1, "TEST x: %d", *calibration->mouse_cal->dzx);

  /*if(*calibration->current_cal == NONE)
  {
    mvaddstr(CAL_Y_P, CAL_X_P + 1, _("Mouse calibration (Ctrl+F1 to edit)"));
  }
  else
  {
    mvaddstr(CAL_Y_P, CAL_X_P + 1, _("Mouse calibration (Ctrl+F1 to save)(mouse wheel to change values)"));
  }
  clrtoeol();
  wmove(wcal, 1, 1);
  if(ginput_get_mk_mode() == GE_MK_MODE_MULTIPLE_INPUTS)
  {
    waddstr(wcal, "Mouse:");
    if(*calibration->current_cal == MC)
    {
      wattron(wcal, COLOR_PAIR(4));
    }
    snprintf(line, COLS, " %s (%d) (F1) ", ginput_mouse_name(*calibration->mouse), ginput_mouse_virtual_id(*calibration->mouse));
    waddstr(wcal, line);
    if(*calibration->current_cal == MC)
    {
      wattron(wcal, COLOR_PAIR(1));
    }
  }
  waddstr(wcal, _("Profile:"));
  if(*calibration->current_cal == CC)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  snprintf(line, COLS, " %d (F2)", *calibration->config + 1);
  waddstr(wcal, line);
  if(*calibration->current_cal == CC)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 2, 1, _("Dead zone:"));
  if(*calibration->current_cal == DZX)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " x=");
  if(mcal->dzx)
  {
    snprintf(line, COLS, "%d", *mcal->dzx);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F3)");
  if(*calibration->current_cal == DZX)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(*calibration->current_cal == DZY)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " y=");
  if(mcal->dzy)
  {
    snprintf(line, COLS, "%d", *mcal->dzy);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F4)");
  if(*calibration->current_cal == DZY)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(*calibration->current_cal == DZS)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" shape="));
  if(mcal->dzs)
  {
    if (*mcal->dzs == E_SHAPE_CIRCLE)
    {
      waddstr(wcal, _("circle"));
    }
    else
    {
      waddstr(wcal, _("rectangle"));
    }
  }
  else
  {
    waddstr(wcal, _(" N/A"));
  }
  waddstr(wcal, " (F5)");
  if(*calibration->current_cal == DZS)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 3, 1, _("Acceleration:"));
  if(*calibration->current_cal == TEST)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" test (F6)"));
  if(*calibration->current_cal == TEST)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(*calibration->current_cal == EX)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " x=");
  if(mcal->ex)
  {
    snprintf(line, COLS, "%.2f", *mcal->ex);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F7)");
  if(*calibration->current_cal == EX)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(*calibration->current_cal == EY)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " y=");
  if(mcal->ey)
  {
    snprintf(line, COLS, "%.2f", *mcal->ey);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F8)");
  if(*calibration->current_cal == EY)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 4, 1, _("Sensitivity:"));
  if(*calibration->current_cal == MX)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  if(mcal->mx)
  {
    snprintf(line, COLS, " %.2f", *mcal->mx);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _(" N/A"));
  }
  waddstr(wcal, " (F9)");
  if(*calibration->current_cal == MX)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 5, 1, "X/Y:");
  if(*calibration->current_cal == RD || *calibration->current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" circle test"));
  if(*calibration->current_cal == RD || *calibration->current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  waddstr(wcal, ", ");
  if(*calibration->current_cal == RD)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  snprintf(line, COLS, _("radius=%d (F10)"), mcal->rd);
  waddstr(wcal, line);
  if(*calibration->current_cal == RD)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  waddstr(wcal, ", ");
  if(*calibration->current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  snprintf(line, COLS, _("velocity=%d (F11)"), mcal->vel);
  waddstr(wcal, line);
  if(*calibration->current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(*calibration->current_cal == MY)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" ratio="));
  if(mcal->mx && mcal->my)
  {
    snprintf(line, COLS, "%.2f", *mcal->my / *mcal->mx);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F12)");
  if(*calibration->current_cal == MY)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  box(wcal, 0 , 0);*/


}


void display_init() {
    initscr();

    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, COLOR_RED);
    init_pair(4, COLOR_RED, COLOR_BLACK);

    wrefresh(stdscr); //first call clears the screen
  
  /*mvaddstr(1, LSTICK_X_P + 1, _("Left stick"));
  mvaddstr(1, RSTICK_X_P + 1, _("Right stick"));
  mvaddstr(1, BUTTON_X_P + 1, _("Buttons"));

  lstick = newwin(STICK_Y_L, STICK_X_L, LSTICK_Y_P, LSTICK_X_P);
  box(lstick, 0 , 0);
  wnoutrefresh(lstick);

  rstick = newwin(STICK_Y_L, STICK_X_L, RSTICK_Y_P, RSTICK_X_P);
  box(rstick, 0 , 0);
  wnoutrefresh(rstick);

  wbuttons = newwin(BUTTON_Y_L, BUTTON_X_L, BUTTON_Y_P, BUTTON_X_P);
  box(wbuttons, 0 , 0);
  wnoutrefresh(wbuttons);

  mvaddstr(CAL_Y_P, CAL_X_P + 1, _("Mouse calibration (Ctrl+F1 to edit)"));

  wcal = newwin(7, COLS-3, CAL_Y_P + 1, CAL_X_P);
  box(wcal, 0 , 0);
  wnoutrefresh(wcal);*/

  //mvprintw(LINES-1, 1, "TEST x: %s", gcal_data.gmodes[0].steps[0].windows[0].useBorder); // Test here
  //mvprintw(LINES-1, 1, "TEST x: %d", gcal_data.current_cal); // Test here
  //mvprintw(LINES-2, 1, "TEST x: %d", *gcal_data.mouse_cal->dzx);

  //struct gwindow *currWindow = &guid.gmodes[3].steps[0].windows[0];
  //currWindow->draw(currWindow, NULL);

  //draw_status_window1
  //mvaddstr(LINES-1, COLS-strlen(SHIFT_ESC), SHIFT_ESC);
    wnoutrefresh(stdscr);
    doupdate();
}

void display_end()
{
    endwin();
}

static void show_stats()
{
  /*char rate[COLS];

  int freq = stats_get_frequency(0);

  if(freq >= 0)
  {
    snprintf(rate, sizeof(rate), _("Refresh rate: %4dHz  "), freq);
    mvaddstr(LINES-1, 1, rate);
  }*/

    //mvprintw(LINES-2, 1, "TEST x: %d", *gcal_data.mouse_cal->dzx);
}

static int last_button_nb = 0;

static void show_axes(e_controller_type type, int axis[])
{
  char label[BUTTON_LENGTH];
  int d = 0;
  int i;

  for(i=rel_axis_rstick_y+1; i<AXIS_MAX; ++i)
  {
    if(axis[i])
    {
      snprintf(label, sizeof(label), "%15s: %5d", controller_get_axis_name(type, i), axis[i]);

      mvwaddstr(wbuttons, 1 + d, 1, label);
      d++;
      if(d == BUTTON_Y_L - 3)
      {
        break;
      }
    }
  }
  memset(label, ' ', sizeof(label));
  label[sizeof(label)-1] = '\0';
  for(i=d; i<last_button_nb; ++i)
  {
    mvwaddstr(wbuttons, 1 + i, 1, label);
  }
  last_button_nb = d;
  wnoutrefresh(wbuttons);

  mvwaddch(lstick, cross[0][1], cross[0][0], ' ');
  cross[0][0] = STICK_X_L / 2 + (double)axis[rel_axis_lstick_x] / controller_get_max_signed(adapter_get(0)->ctype, rel_axis_lstick_x) * (STICK_X_L / 2 - 1);
  cross[0][1] = STICK_Y_L / 2 + (double)axis[rel_axis_lstick_y] / controller_get_max_signed(adapter_get(0)->ctype, rel_axis_lstick_y) * (STICK_Y_L / 2 - 1);
  if(cross[0][0] <= 0 || cross[0][0] >= STICK_X_L-1 || cross[0][1] <= 0 || cross[0][1] >= STICK_Y_L-1)
  {
    mvwaddch(lstick, cross[0][1], cross[0][0], CROSS_CHAR | COLOR_PAIR(3));
  }
  else
  {
    mvwaddch(lstick, cross[0][1], cross[0][0], CROSS_CHAR);
  }
  wnoutrefresh(lstick);

  mvwaddch(rstick, cross[1][1], cross[1][0], ' ');
  cross[1][0] = STICK_X_L / 2 + (double)axis[rel_axis_rstick_x] / controller_get_max_signed(adapter_get(0)->ctype, rel_axis_rstick_x) * (STICK_X_L / 2 - 1);
  cross[1][1] = STICK_Y_L / 2 + (double)axis[rel_axis_rstick_y] / controller_get_max_signed(adapter_get(0)->ctype, rel_axis_rstick_y) * (STICK_Y_L / 2 - 1);
  if(cross[1][0] <= 0 || cross[1][0] >= STICK_X_L-1 || cross[1][1] <= 0 || cross[1][1] >= STICK_Y_L-1)
  {
    mvwaddch(rstick, cross[1][1], cross[1][0], CROSS_CHAR | COLOR_PAIR(3));
  }
  else
  {
    mvwaddch(rstick, cross[1][1], cross[1][0], CROSS_CHAR);
  }
  wnoutrefresh(rstick);
}

void display_run(e_controller_type type, int axis[])
{
  /*show_stats();

  if (axis != NULL)
  {
    show_axes(type, axis);
  }

  move(LINES-1, COLS-1);*/
  wnoutrefresh(stdscr);
  doupdate();
}

/*int main(int argc, char* argv[])
{
  int i;
  display_init();
  int axes[4] = {16,16,0,0};
  int buttons[BUTTON_NB] = {1, 0, 1};

  for(i=0; i<10000; ++i)
  {
    display_run(axes, 32, buttons, 1);
    usleep(10000);
  }

  display_end();

  return 0;
}*/
