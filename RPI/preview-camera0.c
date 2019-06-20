#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define SET_CONTROL 1

int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance;
    int width = 0, height = 0;
    LOG("Open camera...");
    // more information: https://www.raspberrypi.org/documentation/hardware/computemodule/cmio-camera.md
    struct camera_interface cam_interface = {
        .i2c_bus = 0,           // /dev/i2c-0  or /dev/i2c-1   
        .camera_num = 0,        // mipi interface num
        .sda_pins = {28, 0},    // enable sda_pins[camera_num], disable sda_pins[camera_num ? 0 : 1]
        .scl_pins = {29, 1},    // enable scl_pins[camera_num], disable scl_pins[camera_num ? 0 : 1]
        .led_pins = {30, 2},
        .shutdown_pins ={31, 3},
    };
    int res = arducam_init_camera2(&camera_instance, cam_interface);
    if (res) {
        LOG("init camera status = %d", res);
        return -1;
    }
    width = 1920;
    height = 1080;
    LOG("Setting the resolution...");
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current resolution is %dx%d", width, height);
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

    width = 3280;
    height = 2464;
    LOG("Setting the resolution...");
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current resolution is %dx%d", width, height);
    }
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