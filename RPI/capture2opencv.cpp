#include "arducam_mipicamera.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>


#include <opencv2/core/core.hpp>  
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include <vector>
#include <opencv2/xphoto/white_balance.hpp>
//#include "whiteBalance.hpp"

cv::Mat dynamicThreWihteBalance(cv::Mat image);
int getWhitePointThre(cv::Mat whiteRegion);
void WhitePointMask(cv::Mat Cr, cv::Mat Cb, cv::Mat RL);
cv::Mat choiceWhitePoint(cv::Mat YCrCb, int mBlocks, int nBlocks);

#define VCOS_ALIGN_DOWN(p,n) (((ptrdiff_t)(p)) & ~((n)-1))
#define VCOS_ALIGN_UP(p,n) VCOS_ALIGN_DOWN((ptrdiff_t)(p)+(n)-1,(n))

using namespace cv;
using namespace std;


#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)

// #define SOFTWARE_AE_AWB
int frame_count = 0;
cv::Mat *get_image(CAMERA_INSTANCE camera_instance, int width, int height) {
    IMAGE_FORMAT fmt = {IMAGE_ENCODING_I420, 50};
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 3000);
    if (!buffer) 
        return NULL;
    
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.
    width = VCOS_ALIGN_UP(width, 32);
    height = VCOS_ALIGN_UP(height, 16);
    cv::Mat *image = new cv::Mat(cv::Size(width,(int)(height * 1.5)), CV_8UC1, buffer->data);
    cv::cvtColor(*image, *image, cv::COLOR_YUV2BGR_I420);
    arducam_release_buffer(buffer);
    return image;
}

//估计图像result的色温为Temperature，阈值phi默认为180
void CountTemperature(const Mat result,Mat &Temperature ,const short phi=180)
{
    Mat_<Vec3f>::const_iterator rit=result.begin<Vec3f>();
    Mat_<Vec3f>::const_iterator ritend=result.end<Vec3f>();
    //遍历所有像素 估计色温
    float Y=0,Cb=0,Cr=0,n=0,Z=0,Y1=0,Cb1=0,Cr1=0;
    //const int phi=180;
    for(;rit!=ritend;++rit,++n)
    {
        Y1=(*rit)[0];
        Cb1=(*rit)[1];
        Cr1=(*rit)[2];
        Z=Y1-abs(Cb1)-abs(Cr1);
        if (Z>phi)
            {Y+=Y1;
             Cb+=Cb1;
             Cr+=Cr1;
            }
    }
    //所有像素处理完了
    Temperature.at<Vec3f>(0,0)[0]=Y/n;
    Temperature.at<Vec3f>(0,0)[1]=Cb/n;
    Temperature.at<Vec3f>(0,0)[2]=Cr/n;
}
//增益计算
void gain(Mat Temperature,float &u,float&v,float lamda=0.05)
{
    float Y_T=Temperature.at<Vec3f>(0,0)[0];
    float Cb_T=Temperature.at<Vec3f>(0,0)[1];
    float Cr_T=Temperature.at<Vec3f>(0,0)[2];
    //增益计算
//  float u=1,lamda=0.05,v=1;//u是B通道增益，v是R通道增益
    if(abs(Cb_T)>abs(Cr_T))
        if(Cb_T>0)
            u-=lamda;
        else
           u+=lamda;
    else
        if(Cr_T>0)
            v-=lamda;
        else
           v+=lamda;

}
//色温校正

void correctionImage(const Mat image,Mat &result,const float u ,const float v)
{
    Mat_<Vec3b>::const_iterator it=image.begin<Vec3b>();
    Mat_<Vec3b>::const_iterator itend=image.end<Vec3b>();

    Mat_<Vec3b>::iterator rit=result.begin<Vec3b>();
    Mat_<Vec3b>::iterator ritend=result.end<Vec3b>();
    //遍历所有像素 改变B和R通道的值
    for(;it!=itend;++it,++rit)
    {
        (*rit)[0]=saturate_cast<uchar>(u*(*it)[0]);//B
        (*rit)[1]=saturate_cast<uchar>((*it)[1]);//G
        (*rit)[2]=saturate_cast<uchar>(v*(*it)[2]);//R

    }


}

Mat choiceWhitePoint(Mat YCrCb, int mBlocks, int nBlocks) {
	std::vector<cv::Mat> channels;
	cv::split(YCrCb, channels);
	Mat Cr = channels[1];
	Mat Cb = channels[2];
	int height = Cr.rows, width = Cr.cols;
	int blockWidth = width / nBlocks;
	int blockHeight = height / mBlocks;
	Mat RL = cv::Mat::zeros(Cr.rows, Cr.cols, CV_8UC1);
	for (int i = 0; i < mBlocks - 1; i++) {
		int startRow = blockHeight * i;
		int endRow = blockHeight * (i + 1);
		for (int j = 0; j < nBlocks - 1; j++) {
			int startCol = blockWidth * j;
			int endCol = blockWidth * (j + 1);
			WhitePointMask(Cr(Range(startRow, endRow), Range(startCol, endCol)), 
					Cb(Range(startRow, endRow), Range(startCol, endCol)),
					RL(Range(startRow, endRow), Range(startCol, endCol)));
		}
		WhitePointMask(Cr(Range(startRow, endRow), Range(blockWidth * (nBlocks-1), width)), 
				Cb(Range(startRow, endRow), Range(blockWidth * (nBlocks-1), width)), 
				RL(Range(startRow, endRow), Range(blockWidth * (nBlocks-1), width)));
	}
	WhitePointMask(Cr(Range(blockHeight * (mBlocks-1), height), Range(blockWidth * (nBlocks-1), width)), 
			Cb(Range(blockHeight * (mBlocks-1), height), Range(blockWidth * (nBlocks-1), width)), 
			RL(Range(blockHeight * (mBlocks-1), height), Range(blockWidth * (nBlocks-1), width)));
	return RL;
}
void WhitePointMask(Mat Cr, Mat Cb, Mat RL) {
	//imshow("Cr", Cr);
	//imshow("Cb", Cb);
	//waitKey(0);
	Mat savg,sfangcha;
	meanStdDev(Cb, savg, sfangcha);
	double Mb=savg.at<double>(0);
	double Db=sfangcha.at<double>(0);//求出第一部分cb的均值和均方差
	meanStdDev(Cr,savg,sfangcha);
	double Mr=savg.at<double>(0);
	double Dr=sfangcha.at<double>(0);;
	double b,r;
	if (Mb<0)//计算mb+db*sign（mb）
	{ 
		b=Mb+Db*(-1);
	}
	else
		b=Mb+Db;

	
	if (Mr<0)//计算1.5*mr+dr*sign（mb）；
	{
		r=1.5*Mr+Dr*(-1);
	}
	else
		r=1.5*Mr+Dr;
	//Mat mask =  abs(Cb - b) < 1.5 * Db;
	Mat mask;
	//bitwise_and(mask, abs(Cr - r) < 1.5 *Dr, mask);
	bitwise_and(abs(Cb - b) < 1.5 * Db, abs(Cr - b) < 1.5 * Dr, mask);
	RL.setTo(255, mask);
	//imshow("Cbmask", abs(Cb - b) < 1.5 * Db);
	//imshow("Crmask", abs(Cr - b) < 1.5 * Dr);
   	//imshow("and", mask);	
	//waitKey(0);

}

int getWhitePointThre(Mat whiteRegion) {
	int value[256] = {0};
	int width = whiteRegion.cols;
	int height = whiteRegion.rows;
	int counter = 0;
	for (int i = 0; i < height; i++) {
		uchar *rowDataPtr = whiteRegion.ptr<uchar>(i);
		for (int j = 0; j < width; j++) {
			if (rowDataPtr[j] != 0) {
				value[rowDataPtr[j]]++;
				counter++;
			}
		}
	}
	int sum = 0;
	int thre;
	for (thre = 255; thre > 0; thre--) {
		sum += value[thre];
		if (sum >= counter * 0.1)
			break;
	}
	return thre;
}


Mat dynamicThreWihteBalance(Mat image) {
	Mat YCrCb;
	cv::cvtColor(image, YCrCb, cv::COLOR_BGR2YCrCb);
	int mBlocks=3, nBlocks = 4;
	Mat RLMask = choiceWhitePoint(YCrCb, mBlocks, nBlocks);
	//imshow("RLMask", RLMask);
	std::vector<cv::Mat> channels;
	cv::split(YCrCb, channels);
	Mat RL = channels[0];
	Mat Yimg = channels[0].clone();
	double Ymin, Ymax;
	cv::minMaxLoc(Yimg, &Ymin, &Ymax);
	Mat notRLMask;
	cv::bitwise_not(RLMask, notRLMask);
	//imshow("notRL mask", notRLMask);
	RL.setTo(0, notRLMask);
	//imshow("RL", RL);
	int thre = getWhitePointThre(RL);
	cout << "thre:" << thre << endl;
	RLMask.setTo(0, RL < thre);
	cv::split(image, channels);
	Mat B, G, R;
	B = channels[0].clone();
	G = channels[1].clone();
	R = channels[2].clone();
	cv::bitwise_not(RLMask, notRLMask);
	//imshow("notRLMask", notRLMask);
	//waitKey(0);
	float meanB = sum(B.setTo(0, notRLMask))[0] / float(countNonZero(RLMask));
	float meanG = sum(G.setTo(0, notRLMask))[0] / float(countNonZero(RLMask));
	float meanR = sum(R.setTo(0, notRLMask))[0] / float(countNonZero(RLMask));
	Ymax /= 3;
	float gainB = Ymax / meanB;
	float gainG = Ymax / meanG;
	float gainR = Ymax / meanR;
	cout << "gain:" << gainB << " " << gainG << " " << gainR << " " << Ymax << endl;
	channels[0] = channels[0] * gainB;	
	channels[1] = channels[1] * gainG;	
	channels[2] = channels[2] * gainR;
	cv::merge(channels, image);

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

    width = 1920;
    height = 1080;
    LOG("Setting the resolution...");
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current resolution is %dx%d", width, height);
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }

#if defined(SOFTWARE_AE_AWB)
    LOG("Enable Software Auto Exposure...");
    arducam_software_auto_exposure(camera_instance, 1);
    LOG("Enable Software Auto White Balance...");
    arducam_software_auto_white_balance(camera_instance, 1);
    LOG("Waiting for automatic adjustment to complete...");
    usleep(1000 * 1000 * 1);
#endif

    while(1){
        cv::Mat *image = get_image(camera_instance, width, height);
        if(!image)
            continue;
#if 0
        Mat dynamicImg = dynamicThreWihteBalance(*image);
        cv::imshow("Arducam", dynamicImg);
#endif
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