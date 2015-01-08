
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

void difference_absolue( const cv::Mat& src1, const cv::Mat& src2, cv::Mat& dest)
{
	dest = src1.clone();
   	int nPixels = src1.cols * src2.rows;
	uchar *im1 = src1.data;
	uchar *im2 = src2.data;
	uchar *res = dest.data;
	
	for( int i = 0; i < nPixels; i++/* = i+3*/)
	{
		res[i] = abs(im1[i] - im2[i]);
		/*res[i + 1] = abs(im1[i + 1] - im2[i + 1]);
		res[i + 2] = abs(im1[i + 2] - im2[i + 2]);*/
	}  
	
}

float somme_normalisee(const cv::Mat& src)
{
	int nPixels = src.cols * src.rows;
	float somme = 0.f;
	uchar *im1 = src.data;

	for (int i = 0; i < nPixels; ++i)
	{
		somme += im1[i]/256;
	}

	return somme / nPixels;
}

int main(int argc, char *argv[])
{	
	if(argc < 2)
		return -1;
	// disable buffer on stdout to make 'printf' outputs
	// immediately displayed in GUI-console
	setbuf(stdout, NULL);

	// initializations section
	
	char * nom_video = argv[1];

	//cv::VideoCapture capture_block8("/Shared/TP_VTDVR/resources/video.avi");
	cv::VideoCapture capture_block8(nom_video);
	if( ! capture_block8.isOpened() )
	{
		printf( "Failed to open %s video file.\n", nom_video);
		return -1;
	}

	//cv::VideoWriter videoWriter_block6( "output0.mpeg", "PIM1", 25.0);
	
	cv::Mat video_NB, video_NB_precedent;
	cv::Mat diff_Video;
	cv::Mat video;
	float somme = 0.f, new_somme = 0.f;
	int scene = 0;
	int key = 0;
	float Tr = 100;
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
	//	readImage( "/Shared/TP_VTDVR/LIRIS-VISION/Applications/Starling/resource/guardians.jpg", &video);
		showImage( "block2 (ESC to stop, SPACE to pause)", &block8_out1);
		cv::cvtColor( *(&block8_out1), *(&video_NB), CV_BGR2GRAY, 0);

		difference_absolue(video_NB, video_NB_precedent, diff_Video);
		new_somme = somme_normalisee(diff_Video);

		showImage( "block10 (ESC to stop, SPACE to pause)", &diff_Video);
/*
		if(fabs(somme - new_somme) > Tr) {
			printf("New Scene !\n");
			scene++;
			//std::string s = SSTR(scene);
			s = "output" + s + ".mpeg";
			videoWriter_block6.setFileName(s.c_str());
		}

		if( ! saveToVideo( &videoWriter_block6, &video_NB_precedent) ) {
			printf("Failed to write frame to video file.\n");
			break;
		}*/

		if( paused )
			key = cv::waitKey(0);
		else
			key = cv::waitKey(25);
		if( (key & 255) == ' ' )
			paused = ! paused;
		else if( key != -1 )
			goOn = false;

		video_NB_precedent = video_NB.clone();
	}

	return 0;
}

