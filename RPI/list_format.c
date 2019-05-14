#include "arducam_mipicamera.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance;
    int res = arducam_init_camera(&camera_instance);
    LOG("init camera status = %d", res);
    struct format support_fmt;
    int index = 0;
    while (!arducam_get_support_formats(camera_instance, &support_fmt, index++)) {
        LOG("index: %d, width: %d, height: %d", index, support_fmt.width, support_fmt.height);
    }
    index = 0;
    struct camera_ctrl support_cam_ctrl;
    while (!arducam_get_support_controls(camera_instance, &support_cam_ctrl, index++)) {
        int value = 0;
        if (arducam_get_control(camera_instance, support_cam_ctrl.id, &value)) {
            LOG("Get ctrl %s fail.", support_cam_ctrl.desc);
        }
        LOG("index: %d, CID: 0x%08X, desc: %s, min: %d, max: %d, default: %d, current: %d",
            index, support_cam_ctrl.id, support_cam_ctrl.desc, support_cam_ctrl.min_value,
            support_cam_ctrl.max_value, support_cam_ctrl.default_value, value);
    }
    return 0;
}