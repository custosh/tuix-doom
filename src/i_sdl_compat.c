// SDL backend compatibility stubs
#include "i_video.h"
#include "i_input.h"
#include "doomtype.h"

boolean screenvisible = true;
int vanilla_keyboard_mapping = 0;
boolean screensaver_mode = false;
int usegamma = 0;
pixel_t *I_VideoBuffer = NULL;
int screen_width = SCREENWIDTH;
int screen_height = SCREENHEIGHT;
int fullscreen = 0;
int aspect_ratio_correct = 0;
int integer_scaling = 0;
int smooth_pixel_scaling = 0;
int vga_porch_flash = 0;
int force_software_renderer = 0;

int png_screenshots = 0;
char *video_driver = NULL;
char *window_position = NULL;

unsigned int joywait = 0;
int usemouse = 1;

float mouse_acceleration = 1.0f;
int mouse_threshold = 4;

void I_GetWindowPosition(int *x, int *y, int w, int h)
{
    (void)w; (void)h;
    if (x) *x = 0;
    if (y) *y = 0;
}

void I_SetGrabMouseCallback(grabmouse_callback_t func)
{
    (void)func;
}
