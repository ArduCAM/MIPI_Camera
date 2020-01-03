#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
pthread_t manualFocusPthreadID;

#define SET_CONTROL 0
#define WHITE_BALANCE 1
#define AUTO_EXPOSURE 0
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
int get_key_board_from_termios()
{
    int key_value;
    struct termios new_config;
    struct termios old_config;

    tcgetattr(0, &old_config);
    new_config = old_config;
    new_config.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &new_config);
    key_value = getchar();
    tcsetattr(0, TCSANOW, &old_config);
    return key_value;
}
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

void setFocus(CAMERA_INSTANCE camera_instance,int val){
    arducam_set_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE, val);
}

int manualFocusThread(CAMERA_INSTANCE camera_instance){
   int focusVal  = 0;
   printf("Please press up and down key\r\n");
    while(1){
    
        usleep(100);
        int KeyVal = 0;
        KeyVal = get_key_board_from_termios();
        if(KeyVal == 27){
            KeyVal = get_key_board_from_termios();
            if(KeyVal == 91){
                KeyVal = get_key_board_from_termios();      
                switch (KeyVal)
                {
                    case 65/* up */:
                    focusVal += 5;
                    if(focusVal >= 1024){
                        focusVal = 1024;
                    }
                    setFocus(camera_instance, focusVal);
                        printf("focus val:%d\r\n",focusVal);
                    break;
                    case 66/* down */:
                    focusVal -= 5;
                    if(focusVal <= 0){
                        focusVal = 0;
                    }
                    setFocus(camera_instance, focusVal);
                    printf("focus val:%d\r\n",focusVal);
                    break;
                    default:
                    break;
                }
            }
        }
    }
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
int ret = pthread_create(&manualFocusPthreadID, NULL, (int *)manualFocusThread(camera_instance),NULL);

   if(ret){
       printf("pthread create failed");
       return 1;
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