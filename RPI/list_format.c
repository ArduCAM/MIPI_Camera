#include "arducam_mipicamera.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance;
    int res = arducam_init_camera(&camera_instance);
    if (res) {
        LOG("init camera status = %d", res);
        return -1;
    }
    struct format support_fmt;
    int index = 0;
    char fourcc[5];
    fourcc[4] = '\0';
    while (!arducam_get_support_formats(camera_instance, &support_fmt, index++)) {
        strncpy(fourcc, (char *)&support_fmt.pixelformat, 4);
        LOG("mode: %d, width: %d, height: %d, pixelformat: %s, desc: %s", 
            support_fmt.mode, support_fmt.width, support_fmt.height, fourcc, 
            support_fmt.description);
    }
    index = 0;
    struct camera_ctrl support_cam_ctrl;
    while (!arducam_get_support_controls(camera_instance, &support_cam_ctrl, index++)) {
        int value = 0;
        if (arducam_get_control(camera_instance, support_cam_ctrl.id, &value)) {
            LOG("Get ctrl %s fail.", support_cam_ctrl.desc);
        }
        LOG("index: %d, CID: 0x%08X, desc: %s, min: %d, max: %d, default: %d, current: %d",
            index - 1, support_cam_ctrl.id, support_cam_ctrl.desc, support_cam_ctrl.min_value,
            support_cam_ctrl.max_value, support_cam_ctrl.default_value, value);
    }
    res = arducam_close_camera(camera_instance);
    LOG("close camera status = %d", res);
    return 0;
}