#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

#define FOCUS_VAL  270  //range(0-65535)
#define SOFTWARE_AE_AWB
                   //IMAGE_ENCODING_JPEG
                   //IMAGE_ENCODING_BMP
                   //IMAGE_ENCODING_PNG
IMAGE_FORMAT fmt = {IMAGE_ENCODING_PNG, 50};
void save_image(CAMERA_INSTANCE camera_instance, const char *name) {
  
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.
    LOG("Setting the focus...");
        if (arducam_set_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE, FOCUS_VAL)) {
            LOG("Failed to set focus, the camera may not support this control.");
        }
        usleep(1000*10);
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

int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance;
    int width = 0, height = 0;
    char file_name[100];

    LOG("Open camera...");
    int res = arducam_init_camera(&camera_instance);
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
    if(fmt.encoding == IMAGE_ENCODING_JPEG){
        sprintf(file_name, "%dx%d.jpg", width, height);
    }
    if(fmt.encoding == IMAGE_ENCODING_BMP){
        sprintf(file_name, "%dx%d.bmp", width, height);
    }
    if(fmt.encoding == IMAGE_ENCODING_PNG){
        sprintf(file_name, "%dx%d.png", width, height);
    } 
    LOG("Capture image %s...", file_name);
    save_image(camera_instance, file_name);

    LOG("Close camera...");
    res = arducam_close_camera(camera_instance);
    if (res) {
        LOG("close camera status = %d", res);
    }
    return 0;
}