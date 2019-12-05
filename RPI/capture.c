#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

#define SOFTWARE_AE_AWB
                   //IMAGE_ENCODING_JPEG
                   //IMAGE_ENCODING_BMP
                   //IMAGE_ENCODING_PNG
IMAGE_FORMAT fmt = {IMAGE_ENCODING_JPEG, 50};
void save_image(CAMERA_INSTANCE camera_instance, const char *name) {
  
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.
#if defined(SOFTWARE_AE_AWB)
    LOG("Enable Software Auto Exposure...");
    arducam_software_auto_exposure(camera_instance, 1);
    LOG("Enable Software Auto White Balance...");
    arducam_software_auto_white_balance(camera_instance, 1);
   LOG("Waiting for automatic adjustment to complete...");
    usleep(1000 * 1000 * 1);
#endif
        usleep(1000*10);
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 12000);
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
    printf("Please choose sensor mode: ");
    int mode = (int)(getchar()-'0');
    
    LOG("Setting the mode...");
   // res = arducam_set_resolution(camera_instance, &width, &height);
    printf("choose the mode %d\r\n", mode );
    res= arducam_set_mode(camera_instance, mode);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current mode  is %d", mode);
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
    if (arducam_reset_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE)) {
               LOG("Failed to set focus, the camera may not support this control.");
           }

    if(fmt.encoding == IMAGE_ENCODING_JPEG){
        sprintf(file_name, "mode%d.jpg", mode);
    }
    if(fmt.encoding == IMAGE_ENCODING_BMP){
        sprintf(file_name, "mode%d.bmp", mode);
    }
    if(fmt.encoding == IMAGE_ENCODING_PNG){
        sprintf(file_name, "mode%d.png", mode);
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
