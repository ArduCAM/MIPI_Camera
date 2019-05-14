#include "arducam_mipicamera.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

void save_image(CAMERA_INSTANCE *camera_instance, const char *name) {
    IMAGE_FORMAT fmt = {IMAGE_ENCODING_JPEG, 50};
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
    int res = arducam_init_camera(&camera_instance);
    LOG("init camera status = %d", res);

    width = 1920;
    height = 1080;
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    }
    save_image(camera_instance, "1920x1080.jpg");

    width = 3280;
    height = 2464;
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    }
    save_image(camera_instance, "3280x2464.jpg");

    res = arducam_close_camera(camera_instance);
    LOG("close camera status = %d", res);
    return 0;
}