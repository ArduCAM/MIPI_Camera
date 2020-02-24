/*
This is a demo application to showcase the trackbar component.
Authors: Pascal Thomet, Fernando Bevilacqua
*/
#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#define CVUI_IMPLEMENTATION
#include "cvui.h"
#include <pthread.h>
#define WINDOW_NAME	"Arducam mipi camera controller"

#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define SET_CONTROL 0
#ifndef vcos_assert
#define vcos_assert(cond) \
   ( (cond) ? (void)0 : (VCOS_ASSERT_BKPT, VCOS_ASSERT_MSG("%s", #cond)) )
#endif
pthread_t cameraThread;
int exposureValue = 45, focusValue = 15, rgainValue = 10, bgainValue = 5;
bool awbButtonPressFlag = 0;

bool aeButtonPressFlag  = 0;
typedef struct{
bool awbEnable;
bool aeEnable;
} STATE_T;
STATE_T controlState;
// The width of all trackbars used in this example.
int windowWidth = 350;
int windowHigh  = 600;
int barWidth 	= 300;
int buttonWidth  = 50;
int buttonHigh	 = 30;
int x = 10;
int y = 40;
void save_image(CAMERA_INSTANCE camera_instance, const char *name, uint32_t encoding, int quality);
void cameraControlStateInit(STATE_T controlState){
controlState.aeEnable  = 0;
controlState.awbEnable = 0;
}
char* itoa(int num,char* str,int radix)
{
    char index[]="0123456789ABCDEF";
    unsigned unum;
    int i=0,j,k;
    if(radix==10&&num<0)
    {
        unum=(unsigned)-num;
        str[i++]='-';
    }
    else unum=(unsigned)num;
    do{
        str[i++]=index[unum%(unsigned)radix];
        unum/=radix;
       }while(unum);
    str[i]='\0';
    if(str[0]=='-')
        k=1;
    else
        k=0;
     
    for(j=k;j<=(i-1)/2;j++)
    {       char temp;
        temp=str[j];
        str[j]=str[i-1+k-j];
        str[i-1+k-j]=temp;
    }
    return str;
}
int main(int argc, const char *argv[])
{   
    CAMERA_INSTANCE camera_instance;
	cameraControlStateInit(controlState);
	cv::Mat frame = cv::Mat(windowHigh, windowWidth, CV_8UC3);
	// Init cvui and tell it to create a OpenCV window, i.e. cv::namedWindow(WINDOW_NAME).
	cvui::init(WINDOW_NAME);
	//int ret = pthread_create(&cameraThread, NULL, cameraThreadProcess,NULL);
	int res = arducam_init_camera(&camera_instance);
    if (res) {
        LOG("init camera status = %d", res);
        return -1;
    }
	res = arducam_set_mode(camera_instance, 0);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } 
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
   if (arducam_reset_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE)) {
        LOG("Failed to set focus, the camera may not support this control.");
    }
	arducam_get_control(camera_instance, V4L2_CID_EXPOSURE, &exposureValue);
    arducam_get_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE, &focusValue);
    arducam_get_gain(camera_instance, &rgainValue, &bgainValue);
    
	while (true) {
		// Fill the frame with a nice color
		frame = cv::Scalar(49, 52, 49);
		cvui::text(frame, x, y-30, "To exit this app click the Quit or Esc");
		if (cvui::button(frame, x, y, buttonWidth, buttonHigh,"&Quit")) {
			break;
		}
		if (cvui::button(frame, x+1*(buttonWidth+10), y,buttonWidth,buttonHigh,"AWB")) {
			awbButtonPressFlag =!awbButtonPressFlag;
			if(awbButtonPressFlag){
				if (arducam_software_auto_white_balance(camera_instance, 1)) {
        			LOG("Mono camera does not support automatic white balance.");
        		}
			}else{
				if (arducam_software_auto_white_balance(camera_instance, 0)) {
        			LOG("Mono camera does not support automatic white balance.");
        		}
			}	
		}
		if(awbButtonPressFlag){
			cvui::text(frame, x+2*(buttonWidth+10), y+10, "ON");
		}
		else
		{
			cvui::text(frame, x+2*(buttonWidth+10), y+10, "OFF");
		}
		if (cvui::button(frame, x+3*(buttonWidth+10), y,buttonWidth,buttonHigh,"AE")) {
			aeButtonPressFlag =!aeButtonPressFlag;
			if(aeButtonPressFlag){
				if (arducam_software_auto_exposure(camera_instance, 1)) {
        			LOG("Mono camera does not support automatic white balance.");
        		}
			}else{
				if (arducam_software_auto_exposure(camera_instance, 0)) {
        			LOG("Mono camera does not support automatic white balance.");
        		}
			}	
		}
		if(aeButtonPressFlag){
			controlState.aeEnable = 1;
			cvui::text(frame, x+4*(buttonWidth+10), y+10, "ON");
		}
		else
		{
			controlState.aeEnable = 0;
			cvui::text(frame, x+4*(buttonWidth+10), y+10, "OFF");
		}

		if (cvui::button(frame, x, y+buttonHigh, buttonWidth, buttonHigh,"Reset")) {
			if (arducam_reset_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE)) {
        		LOG("Failed to set focus, the camera may not support this control.");
    		}
			arducam_get_control(camera_instance, V4L2_CID_EXPOSURE, &exposureValue);
    		arducam_get_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE, &focusValue);
    		arducam_get_gain(camera_instance, &rgainValue, &bgainValue);
		}
		if (cvui::button(frame, x+1*(buttonWidth+10), y+buttonHigh, buttonWidth, \
			buttonHigh,"Snapshot")) {
			static int k = 0;
			k++;
            char str[8];
            itoa(k, str, 10);
            strcat(str, ".jpg");
            save_image(camera_instance, str, IMAGE_ENCODING_JPEG, 80);
            printf("Image save OK\r\n");
		}
		// The trackbar component uses templates to guess the type of its arguments.
		// You have to be very explicit about the type of the value, the min and
		// the max params. For instance, if they are double, use 100.0 instead of 100.

		cvui::text(frame, x, 1*110+y-30, "exposure step 1 (default)");
		if(cvui::trackbar(frame, x, 1*110+y, barWidth, &exposureValue,0,200)){
			if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, (int)(exposureValue*0xFFFF/200.0))) {
            LOG("Failed to set exposure, the camera may not support this control.");
            }
		}
		cvui::text(frame, x, 2*110+y-30, "focus step 1 (default)");
		if(cvui::trackbar(frame, x, 2*110+y, barWidth, &focusValue,0,1024)){
			if (arducam_set_control(camera_instance, V4L2_CID_FOCUS_ABSOLUTE,focusValue)) {
             LOG("Failed to set focus, the camera may not support this control.");
            }
		}
		cvui::text(frame, x,  3*110+y-30, "awb rgain compensation, step 1 (default)");
		if(cvui::trackbar(frame, x, 3*110+y, barWidth, &rgainValue,0,200)){
			arducam_manual_set_awb_compensation(rgainValue,bgainValue); 
		}
		cvui::text(frame, x, 4*110+y-30, "awb bgain compensation step 1 (default)");

		if(cvui::trackbar(frame, x, 4*110+y, barWidth, &bgainValue,0,200)){
			arducam_manual_set_awb_compensation(rgainValue,bgainValue); 
		}
		cvui::update();
		// Show everything on the screen
		cv::moveWindow(WINDOW_NAME, 1280,0);
		cv::imshow(WINDOW_NAME, frame);
		// Check if ESC key was pressed
		if (cv::waitKey(20) == 27) {
			break;
		}
	}
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

void save_image(CAMERA_INSTANCE camera_instance, const char *name, uint32_t encoding, int quality) {
    IMAGE_FORMAT fmt = {encoding, quality};
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 352000);
    if (!buffer) {
        LOG("capture timeout.");
        return;
    }
    FILE *file = fopen(name, "wb");
    fwrite(buffer->data, buffer->length, 1, file);
    fclose(file);
    arducam_release_buffer(buffer);
}