#include "arducam_mipicamera.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

void save_image(CAMERA_INSTANCE camera_instance, const char *name, int width, int height) {
    IMAGE_FORMAT fmt = {IMAGE_ENCODING_RAW_BAYER, 0};
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 6000);
    if (!buffer) {
        LOG("capture timeout.");
        return;
    }
    if(0){
        BUFFER *buffer2 = arducam_unpack_raw10_to_raw8(buffer->data, width, height);
        arducam_release_buffer(buffer);
        buffer = buffer2;
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

   // width =2592;//2336;//4672;//1920;
    //height =1944;//1748;//3496;//1080;
    LOG("Setting the resolution...");
   // res = arducam_set_resolution(camera_instance, &width, &height);
    res = arducam_set_mode(camera_instance, 0);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
      //  LOG("Current resolution is %dx%d", width, height);
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }

    sprintf(file_name, "%dx%d.raw", width, height);
    LOG("Capture image %s...", file_name);
    save_image(camera_instance, file_name, width, height);

    width = 1280;
    height = 720;
    LOG("Setting the resolution...");
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current resolution is %dx%d", width, height);
    }

    sprintf(file_name, "%dx%d.raw", width, height);
    LOG("Capture image %s...", file_name);
    save_image(camera_instance, file_name, width, height);

    LOG("Close camera...");
    res = arducam_close_camera(camera_instance);
    if (res) {
        LOG("close camera status = %d", res);
    }
    return 0;
}
