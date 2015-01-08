
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

// functions section

void readImage( const char* fileName, cv::Mat *out)
{
	*out = cv::imread(fileName, -1);

	if( ! out->data )
		printf( "Failed to read image from file %s.\n", fileName);
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


int main(int argc, char *argv[])
{
	if(argc < 2)
		return -1;
	
	// disable buffer on stdout to make 'printf' outputs
	// immediately displayed in GUI-console
	setbuf(stdout, NULL);
	char * image = argv[1];

	// initializations section

	// processings section

	cv::Mat block1_out1;
	//readImage( "/Shared/TP_VTDVR/LIRIS-VISION/Applications/Starling/resource/guardians.jpg", &block1_out1);
	
	readImage( image, &block1_out1);
	cv::Mat block4_out1;
	cv::cvtColor( *(&block1_out1), *(&block4_out1), CV_BGR2GRAY, 0);
	showImage( "block2 (ESC to stop, SPACE to pause)", &block4_out1);

	cv::waitKey(0);

	// cleanings section

	return 0;
}

