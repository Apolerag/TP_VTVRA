
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

/**
 * Encapsulate cv::VideoWriter class to made it easier to use.
 */
class EasyVideoWriter
{
public:
	EasyVideoWriter( const char* filename, const char *codec, double fps = 30.0) 
	{
		fileName = filename;
		fourcc = CV_FOURCC( codec[0], codec[1], codec[2], codec[3]);
		frameRate = fps;
		isOpened = false;
	}

	bool writeFrame(const cv::Mat &img)
	{
		if( ! isOpened )
		{
			isOpened = vidWriter.open( fileName, fourcc, frameRate, img.size(), true);
			if( ! isOpened )
			{
				printf( "Failed to open video writer for %s file.\n", fileName);
				return false;
			}
		}

		vidWriter << img;
		return true;
	}

protected:
	const char *fileName;
	int fourcc;
	double frameRate;
	bool isOpened;
	cv::VideoWriter vidWriter;
};

/**
 * Save one frame to video file.
 */
bool saveToVideo( EasyVideoWriter *writer, const cv::Mat *in)
{
	bool success = false;

	if( in )
		success = writer->writeFrame(*in);

	return success;
}


int main(void)
{
	// disable buffer on stdout to make 'printf' outputs
	// immediately displayed in GUI-console
	setbuf(stdout, NULL);

	// initializations section

	cv::VideoCapture capture_block2("/Shared/TP_VTDVR/LIRIS-VISION/Applications/Starling/resource/kth_walkingd1_person01.mpg");
	if( ! capture_block2.isOpened() )
	{
		printf( "Failed to open %s video file.\n", "/Shared/TP_VTDVR/LIRIS-VISION/Applications/Starling/resource/kth_walkingd1_person01.mpg");
		return -1;
	}
	EasyVideoWriter videoWriter_block1( "/Shared/TP_VTDVR/LIRIS-VISION/Applications/Starling/output.mpeg", "DIVX", 25.0);

	int key = 0;
	bool paused = false;
	bool goOn = true;
	while( goOn )
	{
		// processings section

		cv::Mat block2_out1;
		if( ! readVideo( &capture_block2, &block2_out1) )
		{
			printf("End of video file.\n");
			break;
		}
		if( ! saveToVideo( &videoWriter_block1, &block2_out1) )
		{
			printf("Failed to write frame to video file.\n");
			break;
		}

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

