
#include <stdio.h>   // Always a good idea.
#include <string.h>   // Always a good idea.
#include <locale.h>
#include <math.h>

#include <GL/gl.h>   // OpenGL itself.
#include <GL/glu.h>  // GLU support library.
#include <GL/glut.h> // GLUT support library.

#include "gestion_opencv.h"

// Some global variables.

int Window_ID;

// camera resolution
int windowWidth = 640;
int windowHeight = 480;

char* fichierIntrinseque;
unsigned int ligneMire;
unsigned int colonneMire;
double tailleCarreau;

ExtrinsicChessboardCalibrator *extCal;
apicamera::CameraUVC camera;

void unproject( const float *A,  float *pi,  float *pc)
{
	pc[0] = (pi[0] -A[2])/A[0] ;
	pc[1] = (pi[1] -A[5])/A[4];
	pc[2] = 1.0f;
}

float distance(const float *p, const float *q)
{
	return sqrt(pow(p[0]-q[0],2) + pow(p[1]-q[1],2) +pow(p[2]-q[2],2));
}

void calculerFrustum( const float *A, float w, float h, float *frustum)
{
	float p3Dm[3];
	float p3D1[3],p3D2[3],p3D3[3],p3D4[3];
	
	float p2Dm[2];
	float p2D1[2],p2D2[2],p2D3[2],p2D4[2];
	
	p2Dm[0] = w/2; p2Dm[1] = h/2; 
	p2D1[0] = 0; p2D1[1] = h/2;
	p2D2[0] = w; p2D2[1] = h/2;
	p2D3[0] = w/2; p2D3[1] = 0;
	p2D4[0] = w/2; p2D4[1] = h;
	
	unproject(A, p2Dm, p3Dm);
	
	unproject(A, p2D1, p3D1);
	unproject(A, p2D2, p3D2);
	unproject(A, p2D3, p3D3);
	unproject(A, p2D4, p3D4);
	
	//unproject(A,K,pi,pc);
	frustum[0] = -1.f * distance(p3Dm,p3D1);
	frustum[1] = 1.f * distance(p3Dm,p3D2);
	frustum[2] = -1.f * distance(p3Dm,p3D3);
	frustum[3] = 1.f * distance(p3Dm,p3D4);
	frustum[4] =  1.f; // near
	frustum[5] = 10000.0f; // far
}


void dessineAxes(float taille)
{
	glBegin(GL_LINES);

	// dessiner axe X en rouge
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f + taille, 0.0f, 0.0f);
	
	// dessiner axe Y en vert
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f + taille, 0.0f);
	
	// dessiner axe Z en bleu
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f , 0.0f, 0.0f+ taille);

	glEnd();
}

void dessineMire( int w, int h, float sz)
{
	glBegin(GL_QUADS);    	
	
	float initX, initY;
	initX = initY = 0.0f - sz;
	
	for(int i = 0; i < w; i++) {
		for(int j =0; j < h; j++) {
			if(i%2 == j%2 ) glColor3f(0.0f, 0.0f, 0.0f);
			else glColor3f(1.0f, 1.0f, 1.0f);
			
			glVertex3f(initX + i*sz, initY + j*sz, 0.0f);
			glVertex3f(initX + i*sz, initY + (j+1)*sz, 0.0f);
			glVertex3f(initX + (i+1)*sz, initY + (j+1)*sz, 0.0f);
			glVertex3f(initX + (i+1)*sz, initY + j*sz, 0.0f); 
		}
	}
	
	glEnd();
}

void dessineTeaPot(void)
{
	glColor3f(1.f, 0.f, 0.f); 
	glRotatef(-90.0,1.0,0.0,0.0);
	glutWireTeapot(5);
} 

// calcule la transformation GtoC
// GtoC = Global to Camera = Mire to Camera
void calculerTransformation( const float *R, const float *T, float *GtoC)
{
	// colonne 1
	GtoC[0] = R[0];
	GtoC[1] = R[3];
	GtoC[2] = R[6];
	GtoC[3] = 0;
	
	// colonne 2
	GtoC[4] =  R[1];
	GtoC[5] =	 R[4];
	GtoC[6] = R[7];
	GtoC[7] = 0;

	// colonne 3
	GtoC[8] = R[2];
	GtoC[9] =	R[5];
	GtoC[10] = R[8];		
	GtoC[11] = 0;

	// colonne 4
	GtoC[12] = T[0];
	GtoC[13] = T[1];
	GtoC[14] = T[2];
	GtoC[15] = 1;
}

void calculerDirection( const float *A, const float *K, const int w, const int h, float *direction)
{
	float a = w/2, b=h/2;
	direction[2] = 1/A[8];
	direction[1] = (1/A[4]) * (b - A[5]*direction[2]);
	direction[0] = (1/A[0]) * (a - A[1]*direction[1] - A[2]*direction[2]);
}

void glDrawFromCamera( const float *A, const float *K, const float *R, const float *T) 
{
	float frustum[6], GtoC[16], direction[3];
	calculerFrustum( A, windowWidth, windowHeight, frustum);
	
        calculerDirection( A, K, windowWidth, windowHeight, direction);
        calculerTransformation( R, T, GtoC);
 
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
	glFrustum( frustum[0], frustum[1], frustum[2], frustum[3], frustum[4], frustum[5]);
 
	glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
 
        // passe dans le repère caméra
	gluLookAt( 0., 0., 0., direction[0], direction[1], direction[2], 0., -1., 0.);
        // ou glMultMatrixf(transfoCameraAxisToGLAxis)
 
	// passe dans le repère global
	glMultMatrixf(GtoC);
	
	dessineAxes(30.0);
	dessineMire( ligneMire, colonneMire, tailleCarreau);
	dessineTeaPot();
}

GLuint matToTexture(cv::Mat &mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter)
{
	// Generate a number for our textureID's unique handle
	GLuint textureID;
	glGenTextures(1, &textureID);
	 
	// Bind to our texture handle
	glBindTexture(GL_TEXTURE_2D, textureID);
	 
	// Catch silly-mistake texture interpolation method for magnification
	if (magFilter == GL_LINEAR_MIPMAP_LINEAR  ||
	    magFilter == GL_LINEAR_MIPMAP_NEAREST ||
	    magFilter == GL_NEAREST_MIPMAP_LINEAR ||
	    magFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		std::cout << "You can't use MIPMAPs for magnification - setting filter to GL_LINEAR" << std::endl;
		magFilter = GL_LINEAR;
	}
	 
	// Set texture interpolation methods for minification and magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	 
	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);
	 
	// Set incoming texture format to:
	// GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
	// GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
	// Work out other mappings as required ( there's a list in comments in main() )
	GLenum inputColourFormat = GL_BGR;
	if (mat.channels() == 1)
	{
		inputColourFormat = GL_LUMINANCE;
	}
	 
	// Create the texture
	glTexImage2D(GL_TEXTURE_2D,     // Type of texture
		     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
		     GL_RGB,            // Internal colour format to convert to
		     mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
		     mat.rows,          // Image height i.e. 480 for Kinect in standard mode
		     0,                 // Border width in pixels (can either be 1 or 0)
		     inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
		     GL_UNSIGNED_BYTE,  // Image data type
		     mat.ptr());        // The actual image data itself
	 
	 
	return textureID;
}

void dessineTexture(cv::Mat &imageCamera)
{
	// desactive textures
	glEnable(GL_TEXTURE_2D);
	
	// Convert image and depth data to OpenGL textures
	GLuint imageTex = matToTexture(imageCamera,   GL_LINEAR_MIPMAP_LINEAR,   GL_LINEAR, GL_CLAMP);
	 
	// Draw the textures
	// Note: Window co-ordinates origin is top left, texture co-ordinate origin is bottom left.
	 
	// Front facing texture
	glBindTexture(GL_TEXTURE_2D, imageTex);
	glBegin(GL_QUADS);
	glTexCoord2f(1, 1);
	glVertex2f(-windowWidth/2,  windowHeight/2);
	glTexCoord2f(0, 1);
	glVertex2f( windowWidth/2,  windowHeight/2);
	glTexCoord2f(0, 0);
	glVertex2f( windowWidth/2, -windowHeight/2);
	glTexCoord2f(1, 0);
	glVertex2f(-windowWidth/2, -windowHeight/2);
	glEnd();
	 
	// Free the texture memory
	glDeleteTextures(1, &imageTex);
	 
	glDisable(GL_TEXTURE_2D);
}

// ------
// Drawing function
void cbRenderScene(void)
{
	// active z-buffering pour masquage partie cachee
	glEnable(GL_DEPTH_TEST); 

	// Clear the color and depth buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	cv::Mat image_camera;
	cameraUVC_getFrame( &camera, &image_camera);
	cv::Mat translation;
	cv::Mat rotation;
	cv::Mat erreur;
	cv::Mat image_extrin;
	extCal->processFrame( &image_camera, NULL, NULL, &translation, &rotation, &erreur, &image_extrin);

	showImage( " image_extrin (ESC to stop, SPACE to pause)", &image_extrin);
	
	dessineTexture(image_camera);

	
	glDrawFromCamera(extCal->camera->intrinsicA, extCal->camera->intrinsicK, extCal->camera->extrinsicR, extCal->camera->extrinsicT);
		
	// All done drawing.  Let's show it.
	glutSwapBuffers();
	cvWaitKey(5);
}


// ------
// Callback function called when a normal key is pressed.
void cbKeyPressed( unsigned char key, int x, int y)
{
	switch (key) 
	{
		case 113: case 81: case 27: // Q (Escape) - We're outta here.
			glutDestroyWindow(Window_ID);
			exit(1);
			break; // exit doesn't return, but anyway...

		case 'i': 
		case 'I':
			break;

		default:
			printf ("KP: No action for %d.\n", key);
			break;
	}
}

// ------
// Does everything needed before losing control to the main
// OpenGL event loop.  
void ourInit(void) 
{
	// Color to clear color buffer to.
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	// Depth to clear depth buffer to; type of test.
	glClearDepth(1.0);
	glDepthFunc(GL_LESS); 

	// Enables Smooth Color Shading; try GL_FLAT for (lack of) fun.
	glShadeModel(GL_SMOOTH);
}


// ------
// The main() function.  Inits OpenGL.  Calls our own init function,
// then passes control onto OpenGL.
int main(  int argc,  char **argv)
{
	if(argc < 5)
		return -1; 
	fichierIntrinseque = argv[1];
	ligneMire = atoi(argv[2]);
	colonneMire = atoi(argv[3]);
	tailleCarreau = atof(argv[4]);
	// disable buffer on stdout to make 'printf' outputs
	// immediately displayed in GUI-console
	setbuf(stdout, NULL);
	
	// initialize camera UVC
	apicamera::OpenParameters openParam_block3_;
	openParam_block3_.width = 640;
	openParam_block3_.height = 480;
	openParam_block3_.fRate = 30;
	if( camera.open( 0, &openParam_block3_) != 0 )
	{
		printf( "failed to init UVC Camera. Exiting ...\n");
		exit(1);
	}
	extCal = new ExtrinsicChessboardCalibrator( ligneMire, colonneMire, tailleCarreau, fichierIntrinseque, "extrinsics.txt");

	// pour eviter pb de . et , dans les floats
	setlocale(LC_NUMERIC, "C");

	// initialisation de glut ???
	glutInit(&argc, argv);

	// To see OpenGL drawing, take out the GLUT_DOUBLE request.
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);

	// Open a window 
	Window_ID = glutCreateWindow("OpenGL");

	// Register the callback function to do the drawing. 
	glutDisplayFunc(&cbRenderScene);

	// If there's nothing to do, draw.
	glutIdleFunc(&cbRenderScene);

	// And let's get some keyboard input.
	glutKeyboardFunc(&cbKeyPressed);

	// OK, OpenGL's ready to go.  Let's call our own init function.
	ourInit();
	
	glutMainLoop();

	// cleanings section

	//delete extCal;

	return 1;
}

