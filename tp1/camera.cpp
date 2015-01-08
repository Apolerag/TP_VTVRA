
#include <opencv2/opencv.hpp>
#include <stdio.h>

#ifdef WIN32
	// unix to win porting
	#define	  __func__   __FUNCTION__
	#define	  __PRETTY_FUNCTION__   __FUNCTION__
	#define   snprintf   _snprintf
	#define	  sleep(n_seconds)   Sleep((n_seconds)*1000)
	#define   round(d)   floor((d) + 0.5)
#endif


// includes section

#include "Apicamera/src/cameraUVC.h"

// functions section

bool readVideo( cv::VideoCapture *capture, cv::Mat *out)
{
	bool bOk = capture->grab();
	capture->retrieve(*out);

	return bOk;
}

void readImage( const char* fileName, cv::Mat *out)
{
	*out = cv::imread(fileName, -1);

	if( ! out->data )
		printf( "Failed to read image from file %s.\n", fileName);
}

void cameraUVC_getFrame( apicamera::CameraUVC *camera, cv::Mat *out1)
{
	cv::Mat(camera->get1Frame()).copyTo(*out1);
}

void showImage( const char* windowName, const cv::Mat *in)
{
	if( in == NULL || ( in->cols == 0 && in->rows == 0 ) )
	{
		// invalid image, display empty image
		const int w = 200;
		const int h = 100;
		cv::Mat img = cv::Mat( h, w, CV_8UC3, cv::Scalar(0));
		cv::line( img, cv::Point( 0, 0), cv::Point( w-1, h-1), cv::Scalar(0,0,255), 2);
		cv::line( img, cv::Point( 0, h-1), cv::Point( w-1, 0), cv::Scalar(0,0,255), 2);
		cv::imshow( windowName, img);
		return;
	}
	else if( in->depth() == CV_32F  ||  in->depth() == CV_64F )
	{
		// float image must be normalized in [0,1] to be displayed
		cv::Mat img;
		cv::normalize( *in, img, 1.0, 0.0, cv::NORM_MINMAX);
		cv::imshow( windowName, img);
		return;
	}

	cv::imshow( windowName, *in);
}


int main(void)
{
	// disable buffer on stdout to make 'printf' outputs
	// immediately displayed in GUI-console
	setbuf(stdout, NULL);

	// initializations section

	cv::VideoCapture capture_block8("/Shared/TP_VTDVR/resources/video.avi");
	if( ! capture_block8.isOpened() )
	{
		printf( "Failed to open %s video file.\n", "/Shared/TP_VTDVR/resources/video.avi");
		return -1;
	}
	// initialize camera UVC
	apicamera::OpenParameters openParam_block9_;
	openParam_block9_.width = 640;
	openParam_block9_.height = 480;
	openParam_block9_.fRate = 30;
	apicamera::CameraUVC camera_block9_;
	if( camera_block9_.open( 0, &openParam_block9_) != 0 )
	{
		printf( "failed to init UVC Camera. Exiting ...\n");
		exit(1);
	}

	int key = 0;
	bool paused = false;
	bool goOn = true;
	while( goOn )
	{
		// processings section

		cv::Mat block8_out1;
		if( ! readVideo( &capture_block8, &block8_out1) )
		{
			printf("End of video file.\n");
			break;
		}
		cv::Mat block1_out1;
		readImage( "/Shared/TP_VTDVR/LIRIS-VISION/Applications/Starling/resource/guardians.jpg", &block1_out1);
		cv::Mat block9_out1;
		cameraUVC_getFrame( &camera_block9_, &block9_out1);
		showImage( "block2 (ESC to stop, SPACE to pause)", &block9_out1);

		if( paused )
			key = cv::waitKey(0);
		else
			key = cv::waitKey(25);
		if( (key & 255) == ' ' )
			paused = ! paused;
		else if( key != -1 )
			goOn = false;
	}

	// cleanings section

	return 0;
}

