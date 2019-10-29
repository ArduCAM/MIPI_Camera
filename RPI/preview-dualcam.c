#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define SET_CONTROL 1

int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance, camera_instance2;
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
        .window = {0, 0, 640, 480}, // Destination rectangle for the preview window.
    };
    res = arducam_start_preview(camera_instance, &preview_params);
    if (res) {
        LOG("start preview status = %d", res);
        return -1;
    }

    // Important: Using init_camera will invalidate the read/write I2C of another camera_instance, 
    // ie the control command is invalid and the resolution cannot be switched(autoexposure is also unusable).
    // This problem will be fixed in the future.
    LOG("Open camera...");
    cam_interface.camera_num = 1;
    res = arducam_init_camera2(&camera_instance2, cam_interface);
    if (res) {
        LOG("init camera status = %d", res);
        return -1;
    }

    width = 1920;
    height = 1080;
    LOG("Setting the resolution...");
    res = arducam_set_resolution(camera_instance2, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current resolution is %dx%d", width, height);
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
    LOG("Start preview...");
    PREVIEW_PARAMS preview_params2 = {
        .fullscreen = 0,             // 0 is use previewRect, non-zero to use full screen
        .opacity = 255,              // Opacity of window - 0 = transparent, 255 = opaque
        .window = {0, 480, 640, 480}, // Destination rectangle for the preview window.
    };
    res = arducam_start_preview(camera_instance2, &preview_params2);
    if (res) {
        LOG("start preview status = %d", res);
        return -1;
    }

    usleep(1000 * 1000 * 20);
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
    res = arducam_close_camera(camera_instance2);
    if (res) {
        LOG("close camera status = %d", res);
    }
    return 0;
}