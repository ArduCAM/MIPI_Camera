#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance;
    int width = 0, height = 0;
    int res = arducam_init_camera(&camera_instance);
    if (res) {
        LOG("init camera status = %d", res);
        return -1;
    }
    width = 1920;
    height = 1080;
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    }
    res = arducam_start_preview(camera_instance, NULL);
    if (res) {
        LOG("start preview status = %d", res);
        return -1;
    }
    usleep(1000 * 1000 * 2);
    arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 0x10);
    usleep(1000 * 1000 * 2);
    arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 3000);
    usleep(1000 * 1000 * 2);
    arducam_set_control(camera_instance, V4L2_CID_HFLIP, 1);
    usleep(1000 * 1000 * 10);

    width = 3280;
    height = 2464;
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    }
    usleep(1000 * 1000 * 10);
    res = arducam_stop_preview(camera_instance);
    LOG("stop preview status = %d", res);
    res = arducam_close_camera(camera_instance);
    LOG("close camera status = %d", res);
    return 0;
}