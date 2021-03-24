#include "arducam_mipicamera.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>

#define VCOS_ALIGN_DOWN(p,n) (((ptrdiff_t)(p)) & ~((n)-1))
#define VCOS_ALIGN_UP(p,n) VCOS_ALIGN_DOWN((ptrdiff_t)(p)+(n)-1,(n))

using namespace cv;
using namespace std;
#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

int frame_count = 0;
cv::Mat *get_image(CAMERA_INSTANCE camera_instance, int width, int height) {
    IMAGE_FORMAT fmt = {IMAGE_ENCODING_RAW_BAYER, 0};
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 3000);
    if (!buffer) 
        return NULL;
    
    BUFFER *buffer2 = arducam_unpack_raw10_to_raw8(buffer->data, width, height);
    arducam_release_buffer(buffer);
    buffer = buffer2;

    BUFFER *rgb = arducam_rgbir_to_rgb24(buffer->data, width, height);
    arducam_release_buffer(buffer);

    cv::Mat *image = new cv::Mat(cv::Size(width, height * 2), CV_8UC3);
    memcpy(image->data, rgb->data, width * height * 3 * 2);
    arducam_release_buffer(rgb);
    return image;
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

    LOG("Setting the resolution...");
    res = arducam_set_mode(camera_instance, 0);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        struct format fmt;
        arducam_get_format(camera_instance, &fmt);
        width = fmt.width;
        height = fmt.height;
        LOG("Current resolution is %dx%d", width, height);
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }

    float scale = 640.0 / width;
    while(1)
	{
        cv::Mat *image = get_image(camera_instance, width, height);
        cv::resize(*image, *image, cv::Size(), scale, scale);
        if(!image)
          return 0;// continue;
        cv::imshow("Arducam", *image);
        cv::waitKey(10);
        delete image;
    }
    
	
    LOG("Close camera...");
    res = arducam_close_camera(camera_instance);
    if (res) {
        LOG("close camera status = %d", res);
    }
    return 0;
}