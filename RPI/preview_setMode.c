#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h> //add support atoi
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define SET_CONTROL 0
#define WHITE_BALANCE 1
#define AUTO_EXPOSURE 0
int SAMPLE_Preview_Usage(char* sPrgNm)
{
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
    LOG("Please check and choose the mode your sensor support", res);
    printf("Usage : %s <mode>\n", sPrgNm);
    return 0;
}

int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance;
    if (argc < 2)
    {
        
        SAMPLE_Preview_Usage(argv[0]);
        return -1;
    }
    int mode = atoi(argv[1]);
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

    

    
    
    LOG("Setting the resolution...");
   // res = arducam_set_resolution(camera_instance, &width, &height);
    res= arducam_set_mode(camera_instance, mode); 
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current resolution mode is %d", mode);
         //LOG("Current resolution  is %dx%d", width, height);
      //  LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
#if WHITE_BALANCE
         LOG("Enable Auto White Balance...");
    if (arducam_software_auto_white_balance(camera_instance, 1)) {
        LOG("Mono camera does not support automatic white balance.");
    }
#endif
#if AUTO_EXPOSURE
	 LOG("Enable Software Auto Exposure...");
    arducam_software_auto_exposure(camera_instance, 1);
#endif
    LOG("Start preview...");
    PREVIEW_PARAMS preview_params = {
        .fullscreen = 0,             // 0 is use previewRect, non-zero to use full screen
        .opacity = 255,              // Opacity of window - 0 = transparent, 255 = opaque
        .window = {0, 0, 1280, 720}, // Destination rectangle for the preview window.
    };
    res = arducam_start_preview(camera_instance, &preview_params);

    if (res) {
        LOG("start preview status = %d", res);
        return -1;
    }
#if SET_CONTROL
    LOG("Reset the focus...");
    if (arducam_reset_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE)) {
        LOG("Failed to set focus, the camera may not support this control.");
    }
	arducam_set_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE, 0x10)
	
    usleep(1000 * 1000 * 2);
    LOG("Setting the exposure...");
    if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 0x10)) {
        LOG("Failed to set exposure, the camera may not support this control.");
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Setting the exposure...");
    if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 3000)) {
        LOG("Failed to set exposure, the camera may not support this control.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Setting the hfilp...");
    if (arducam_set_control(camera_instance, V4L2_CID_HFLIP, 1)) {
        LOG("Failed to set hflip, the camera may not support this control.");
    }
    usleep(1000 * 1000 * 2);
    LOG("Enable Auto Exposure...");
    arducam_software_auto_exposure(camera_instance, 1);

    usleep(1000 * 1000 * 2);
    LOG("Enable Auto White Balance...");
    if (arducam_software_auto_white_balance(camera_instance, 1)) {
        LOG("Mono camera does not support automatic white balance.");
    }
#endif
uint32_t r_gain_compensation, b_gain_compensation;
uint32_t minus = 0;
while(1){
    uint32_t temp, r_temp, b_temp;
    printf("\r\nR_gain_compensation: ");
    temp = (int)(getchar()-'0');  r_temp = 0;
    while(temp != -38){  //enter
        if(temp == -3){  //-
          minus = 1;
        }else{
            r_temp = r_temp*10 + temp;
        }
       temp = (int)(getchar()-'0');
    }
    if(minus){
        r_gain_compensation = -r_temp;
    }
    else{
        r_gain_compensation = r_temp;
    }
    
    printf("Set r_gain_compensation to %d\r\n",r_gain_compensation);
    arducam_manual_set_awb_compensation(r_gain_compensation,b_gain_compensation);

    printf("\r\nB_gain_compensation:  ");
    temp = (int)(getchar()-'0');  b_temp = 0;minus = 0;
    while(temp != -38){  //enter
        if(temp == -3){  //-
          minus = 1;
        }else{
            b_temp = b_temp*10 + temp;
        }
       
        temp = (int)(getchar()-'0');
    }
    if(minus){
        b_gain_compensation = -b_temp;
    }
    else{
       b_gain_compensation = b_temp;
    }
    printf("Set b_gain_compensation to %d\r\n",b_gain_compensation);
    arducam_manual_set_awb_compensation(r_gain_compensation,b_gain_compensation);
}
    usleep(1000 * 1000 * 10);
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
    return 0;
}


