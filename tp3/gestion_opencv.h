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
#include "Calibration/src/chessboardcalibration.h"
#include "Apicamera/src/cameraOPENCV.h"
#include <iostream>
#include <fstream>

// functions section

bool readVideo( cv::VideoCapture *capture, cv::Mat *out)
{
	bool bOk = capture->grab();
	capture->retrieve(*out);

	return bOk;
}

void cameraUVC_getFrame( apicamera::CameraUVC *camera, cv::Mat *out1)
{
	cv::Mat(camera->get1Frame()).copyTo(*out1);
}

/**
 * Encapsulate ChessboardCalibration class to made it easier to use.
 */
class ExtrinsicChessboardCalibrator
{
public:
	ExtrinsicChessboardCalibrator( unsigned int _cbWidth, unsigned int _cbHeight, float _squareSize, const char *_intrinsicFileName, const char *_extrinsicFileName)
	{
		// load intrinsic parameters
		camera = new apicamera::CameraOPENCV();
		camera->loadIntrinsicParameters(_intrinsicFileName);

		// initialize calibration
		calibrator = new ChessboardCalibration( camera, 1, _cbWidth, _cbHeight, _squareSize);
		extrinsicFileName = _extrinsicFileName;
		done = false;
	}

	~ExtrinsicChessboardCalibrator()
	{
		delete calibrator;
		delete camera;
	}

	void processFrame( const cv::Mat *inImg, const cv::Mat *intrinsicA, const cv::Mat *intrinsicK, cv::Mat *translation, cv::Mat *rotation, cv::Mat *error, cv::Mat *outImg)
	{
		if( (! inImg) || (! inImg->data) )
			return;

		inImg->copyTo(*outImg);
		IplImage currentImage(*outImg);

		// set intrinsic parameters if provided through block inputs
		if( intrinsicA )
		{
			cv::Mat A( 3, 3, CV_32FC1, camera->intrinsicA);
			intrinsicA->copyTo(A);
		}
		if( intrinsicK )
		{
			cv::Mat K( 1, 4, CV_32FC1, camera->intrinsicK);
			intrinsicK->copyTo(K);
		}

		// compute extrinsic parameters
		camera->extrinsicError = calibrator->findExtrinsicParameters( 0.0f, 0.0f, &currentImage);

		// save extrinsic parameters to file
		camera->saveExtrinsicParameters(extrinsicFileName);

		// copy extrinsic parameters and error to outputs
		cv::Mat T( 1, 3, CV_32FC1, camera->extrinsicT);
		T.copyTo(*translation);
		cv::Mat R( 3, 3, CV_32FC1, camera->extrinsicR);
		R.copyTo(*rotation);
		cv::Mat E( 1, 1, CV_32FC1, &(camera->extrinsicError));
		E.copyTo(*error);
		
		done = true;
	}
	
	bool getDone()
	{
		return done;
	}

public:
	// camera is used only to store/load/save intrinsic/extrinsic parameters
	apicamera::CameraOPENCV *camera;

	ChessboardCalibration *calibrator;
	const char *extrinsicFileName;	
	bool done;
};

void printMat( const cv::Mat *in, const char *printingMode, const char *outputMode, const char *outputFile)
{
	std::streambuf * buf;
	std::ofstream output_file;
	std::string output = outputMode;
	// Use the file to write the matrix.
	if( output == "file" )
	{
		output_file.open( outputFile );
		if( !output_file.is_open() )
		{
			std::cerr << "Unable to open \"" << outputFile << "\" to write matrice." << std::endl;
		}
		std::cout << "Writing in \"" << outputFile << "\" file." << std::endl;
		buf = output_file.rdbuf();
	}
	// Use default stdout to print the matrix.
	else if( output == "stdout" )
	{
		buf = std::cout.rdbuf();
	}
	std::ostream out( buf );

	std::string cv_format = printingMode;
	if( cv_format != "default" )
		out << format(*in, printingMode) << std::endl;
	else
		out << *in << std::endl;
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
