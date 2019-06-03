#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
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
    LOG("Setting the 0x7251 exposure to 0x10...");
    if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 0x0A)) {
        LOG("Failed to set exposure, the camera may not support this control.");
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }

    int ctrl_value = 0;
    if (arducam_get_control(camera_instance, V4L2_CID_EXPOSURE, &ctrl_value)) {
        LOG("Failed to get exposure value, the camera may not support this control.");
    } else {
        LOG("Read exposure value = 0x%02X", ctrl_value);
    }

    uint16_t reg_val = 0;
    // note The upper four bits of the 0x3502 register are the lower four bits of the ov7251 exposure register.
    if (arducam_read_sensor_reg(camera_instance, 0x3502, &reg_val)) {
        LOG("Failed to read sensor register.");
    } else {
        LOG("Read 0x3502 value = 0x%02X", reg_val);
    }

    // note The upper four bits of the 0x3502 register are the lower four bits of the ov7251 exposure register.
    LOG("Write 0x7251 exposure register to 0x0F...");
    if (arducam_write_sensor_reg(camera_instance, 0x3502, 0xF0)) {
        LOG("Failed to write sensor register.");
    }

    if (arducam_get_control(camera_instance, V4L2_CID_EXPOSURE, &ctrl_value)) {
        LOG("Failed to get exposure value, the camera may not support this control.");
    } else {
        LOG("Read exposure value = 0x%02X", ctrl_value);
    }

    if (arducam_close_camera(camera_instance)) {
        LOG("close camera status = %d", res);
    }

    return 0;
}