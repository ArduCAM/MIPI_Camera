#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define SET_CONTROL 1


void list_modes(CAMERA_INSTANCE camera_instance){
    LOG("Available mode:");
    struct format support_fmt;
    int index = 0;
    char fourcc[5];
    fourcc[4] = '\0';
    while (!arducam_get_support_formats(camera_instance, &support_fmt, index++)) {
        strncpy(fourcc, (char *)&support_fmt.pixelformat, 4);
        LOG("   mode: %d, width: %d, height: %d, pixelformat: %s, desc: %s", 
            support_fmt.mode, support_fmt.width, support_fmt.height, fourcc, 
            support_fmt.description);
    }
    LOG("");
}
int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance;
    LOG("Open camera...");
    int res = arducam_init_camera(&camera_instance);
    if (res) {
        LOG("init camera status = %d", res);
        return -1;
    }
    list_modes(camera_instance);
    struct format fmt;
    LOG("Setting the mode...");
    //You can use the list_format program to view the supported modes.

// #define CAM_OV2311      //Monochrome
#define CAM_0G02B10     //Color
#if defined(CAM_OV2311)
    res = arducam_set_mode(camera_instance, 0); 
#else
    res = arducam_set_mode(camera_instance, 1); 
#endif
    arducam_get_format(camera_instance, &fmt);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current resolution is %dx%d", fmt.width, fmt.height);
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
    LOG("Start preview...");
    PREVIEW_PARAMS preview_params = {
        .fullscreen = 0,             // 0 is use previewRect, non-zero to use full screen
        .opacity = 255,              // Opacity of window - 0 = transparent, 255 = opaque
        .window = {0, 0, 1280, 720}, // Destination rectangle for the preview window.
    };
    res = arducam_start_preview(camera_instance, &preview_params);
    if (res) {
        LOG("start preview status = %d", res);
        return -1;
    }
#if SET_CONTROL
    LOG("Reset the focus...");
    if (arducam_reset_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE)) {
        LOG("Failed to set focus, the camera may not support this control.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Setting the exposure...");
    if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 0x10)) {
        LOG("Failed to set exposure, the camera may not support this control.");
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Setting the exposure...");
    if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 3000)) {
        LOG("Failed to set exposure, the camera may not support this control.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Setting the hfilp...");
    if (arducam_set_control(camera_instance, V4L2_CID_HFLIP, 1)) {
        LOG("Failed to set hflip, the camera may not support this control.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Enable Auto Exposure...");
    arducam_software_auto_exposure(camera_instance, 1);

    usleep(1000 * 1000 * 2);
    LOG("Enable Auto White Balance...");
    if (arducam_software_auto_white_balance(camera_instance, 1)) {
        LOG("Mono camera does not support automatic white balance.");
    }
#endif
    usleep(1000 * 1000 * 10);
    LOG("Stop preview...");
    res = arducam_stop_preview(camera_instance);
    if (res) {
        LOG("stop preview status = %d", res);
    }

    LOG("Close camera...");
    res = arducam_close_camera(camera_instance);
    if (res) {
        LOG("close camera status = %d", res);
    }
    return 0;
}