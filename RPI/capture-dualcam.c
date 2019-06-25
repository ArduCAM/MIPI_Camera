#include "arducam_mipicamera.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

#define SOFTWARE_AE_AWB

void save_image(CAMERA_INSTANCE camera_instance, const char *name) {
    IMAGE_FORMAT fmt = {IMAGE_ENCODING_JPEG, 50};
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 3000);
    if (!buffer) {
        LOG("capture timeout.");
        return;
    }
    FILE *file = fopen(name, "wb");
    fwrite(buffer->data, buffer->length, 1, file);
    fclose(file);
    arducam_release_buffer(buffer);
}
int capture(int camera_num){
    CAMERA_INSTANCE camera_instance;
    int width = 0, height = 0;
    char file_name[100];

    LOG("Open camera...");
    // more information: https://www.raspberrypi.org/documentation/hardware/computemodule/cmio-camera.md
    struct camera_interface cam_interface = {
        .i2c_bus = 0,           // /dev/i2c-0  or /dev/i2c-1   
        .camera_num = camera_num,        // mipi interface num
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
#if defined(SOFTWARE_AE_AWB)
    LOG("Enable Software Auto Exposure...");
    arducam_software_auto_exposure(camera_instance, 1);
    LOG("Enable Software Auto White Balance...");
    arducam_software_auto_white_balance(camera_instance, 1);
    LOG("Waiting for automatic adjustment to complete...");
    usleep(1000 * 1000 * 1);
#endif
    sprintf(file_name, "camera%d-%dx%d.jpg",camera_num, width, height);
    LOG("Capture image %s...", file_name);
    save_image(camera_instance, file_name);


    LOG("Close camera...");
    res = arducam_close_camera(camera_instance);
    if (res) {
        LOG("close camera status = %d", res);
    }
    return 0;
}
int main(int argc, char **argv) {
   
    capture(0);
    capture(1);
    return 0;
}