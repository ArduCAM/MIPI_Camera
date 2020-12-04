#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <zbar.h>   //sudo apt-get install libzbar-dev 
using namespace std;
using namespace zbar;
#define VCOS_ALIGN_DOWN(p,n) (((ptrdiff_t)(p)) & ~((n)-1))
#define VCOS_ALIGN_UP(p,n) VCOS_ALIGN_DOWN((ptrdiff_t)(p)+(n)-1,(n))


#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

void QRCodeDecode(ImageScanner &scanner, cv::Mat &frame);
// #define SOFTWARE_AE_AWB
int frame_count = 0;
cv::Mat *get_image(CAMERA_INSTANCE camera_instance, int width, int height) {
    IMAGE_FORMAT fmt = {IMAGE_ENCODING_RAW_BAYER, 50};
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 3000);
    if (!buffer) 
        return NULL;
    
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.
    width = VCOS_ALIGN_UP(width, 32);
    height = VCOS_ALIGN_UP(height, 16);
    cv::Mat *image = new cv::Mat(cv::Size(width,height), CV_8UC1, buffer->data);
    cv::cvtColor(*image, *image, cv::COLOR_GRAY2BGR);
    arducam_release_buffer(buffer);
    return image;
}

void show_help(){
    LOG(
        "Usage:\n"
        "   ./qrcode_detection <exposure_value>\n"
        "example:\n"
        "   ./qrcode_detection 10"
    );
}

int main(int argc, char **argv) {
    if(argc < 2){
        show_help();
        exit(-1);
    }
    

    CAMERA_INSTANCE camera_instance;
    int width = 0, height = 0, exposure_value = 1000;
    exposure_value = atoi(argv[1]);

    LOG("Open camera...");
    int res = arducam_init_camera(&camera_instance);
    if (res) {
        LOG("init camera status = %d", res);
        return -1;
    }

    width = 640;
    height = 480;
    LOG("Setting the resolution...");
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current resolution is %dx%d", width, height);
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
    LOG("Setting the exposure...");
    if (arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, exposure_value)) {
        LOG("Failed to set exposure, the camera may not support this control.");
    }


    ImageScanner scanner;
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);
    while(1){
        cv::Mat *image = get_image(camera_instance, width, height);
        // cv::Mat frame = *image;
        QRCodeDecode(scanner, *image);
        if(!image)
            continue;
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


void QRCodeDecode(ImageScanner &scanner, cv::Mat &frame)
{
	cv::Mat grey;
	cv::cvtColor(frame, grey, cv::COLOR_BGR2GRAY);

	int width = frame.cols;
	int height = frame.rows;
	uchar *raw = (uchar *)grey.data;
	// wrap image data
	Image image(width, height, "Y800", raw, width * height);
	// scan the image for barcodes
	int n = scanner.scan(image);
	// extract results
	for (Image::SymbolIterator symbol = image.symbol_begin();
		 symbol != image.symbol_end();
		 ++symbol)
	{
		vector<cv::Point> vp;
		// do something useful with results
		cout << "decoded " << symbol->get_type_name() << " symbol \"" << symbol->get_data() << '"' << " " << endl;
		int n = symbol->get_location_size();
		for (int i = 0; i < n; i++)
		{
			vp.push_back(cv::Point(symbol->get_location_x(i), symbol->get_location_y(i)));
		}
		cv::RotatedRect r = minAreaRect(vp);
		cv::Point2f pts[4];
		r.points(pts);
		for (int i = 0; i < 4; i++)
		{
			line(frame, pts[i], pts[(i + 1) % 4], cv::Scalar(255, 0, 0), 3);
		}
	}

}
