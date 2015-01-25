/**
 * @file tp3.cpp
 * @author Aurélien CHEMIER
 * @date janvier 2015
 * @brief le fichier principale du TP
 */
#include <stdio.h>   // Always a good idea.
#include <string.h>   // Always a good idea.
#include <locale.h>
#include <math.h>

#include <GL/gl.h>   // OpenGL itself.
#include <GL/glu.h>  // GLU support library.
#include <GL/glut.h> // GLUT support library.

#include "gestion_opencv.h"


int Window_ID;

/**
 * @brief résolution de la caméra
 */ 
int windowWidth = 640;

/**
 * @brief résolution de la caméra
 */ 
intwindowHeight = 480;

/**
 * @brief nom du fichier contenant les paramètres intrinsèques de la caméra
 */
char* fichierIntrinseque;

/**
 * @brief le nombre de ligne de la mire
 */
unsigned int ligneMire;

/**
 * @brief le nombre de colonne de la mire
 */
unsigned int colonneMire;

/**
 * @brief la taille des carreaux de la mire
 */
double tailleCarreau;

/**
 * @brief calcul des paramètres extrinsèques de la caméra
 */
ExtrinsicChessboardCalibrator *extCal;

/**
 * @brief la caméra
 */
apicamera::CameraUVC camera;

/**
 * @brief recupère la projection d'un point du plan image dans le plan global
 * 
 * @param A  la matrice des paramètres intrinsèques
 * @param pi un point 2D du plan image
 * @param pc la projection de pi en 3D dans le plan global
 */
void unproject(const float *A,  const float *pi,  float *pc)
{
	pc[0] = (pi[0] -A[2])/A[0] ;
	pc[1] = (pi[1] -A[5])/A[4];
	pc[2] = 1.0f;
}

/**
 * @brief calcule la distance entre 2 points 3D
 * 
 * @param p un point 3D
 * @param q un point 3D
 * 
 * @warning p et q sont des tableaux de float de taille 3.
 * 
 * @return le distance entre p et q
 */
float distance(const float *p, const float *q)
{
	return sqrt(pow(p[0]-q[0],2) + pow(p[1]-q[1],2) +pow(p[2]-q[2],2));
}

/**
 * @brief calcul le frustrum de la fenêtre openGL
 * 
 * @param A la matrice des paramètres intrinsèques
 * @param w la largeur de la fenêtre
 * @param h la hauteur de la fenêtre
 * @param frustum le frustrum calculé
 */
void calculerFrustum(const float *A, const float w, const float h, float *frustum)
{
	float p3Dm[3];
	float p3D1[3],p3D2[3],p3D3[3],p3D4[3];
	
	float p2Dm[2] = {w/2, h/2};
	float p2D1[2] = {0.f, h/2}, p2D2[2] = {w, h/2}, p2D3[2] = {w/2, 0.f} ,p2D4[2] = {w/2, h};
	
	unproject(A, p2Dm, p3Dm);
	
	unproject(A, p2D1, p3D1);
	unproject(A, p2D2, p3D2);
	unproject(A, p2D3, p3D3);
	unproject(A, p2D4, p3D4);
	
	frustum[0] = -1.f * distance(p3Dm,p3D1);
	frustum[1] = 1.f * distance(p3Dm,p3D2);
	frustum[2] = -1.f * distance(p3Dm,p3D3);
	frustum[3] = 1.f * distance(p3Dm,p3D4);
	frustum[4] =  1.f; // near
	frustum[5] = 10000.0f; // far
}

/**
 * @brief dessine les axes dans la fenêtre openGL
 * 
 * @param taille la taille des axes à dessiner
 */
void dessineAxes(const float taille)
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

/**
 * @brief dessine la mire dans la fenêtre openGL
 * 
 * @param w la largeur de la fenêtre
 * @param h la hauteur de la fenêtre
 * @param sz la taille des carreaux de la mire
 */
void dessineMire(const int w, const int h, const float sz)
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

/**
 * @brief dessine une théire sur la mire
 * 
 */
void dessineTeaPot(void)
{
	glColor3f(1.f, 0.f, 0.f); 
	glRotatef(-90.0,1.0,0.0,0.0);
	glutWireTeapot(5);
} 

/**
 * @brief calcule la transformation GtoC
 * @details GtoC = Global to Camera = Mire to Camera
 * 
 * @param R la rotation repère global (repère de la mire) vers repère caméra
 * @param T la translation repère global vers repère caméra
 * @param GtoC la transformation Gtoc
 */
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

/**
 * @brief calule la direction de la camera
 * 
 * @param A la matrice des paramètres intrinsèques
 * @param K le vecteur des coefficients de distortion
 * @param w la largeur de la fenêtre
 * @param h la hauteur de la fenêtre
 * @param direction la direction de la caméra
 */
void calculerDirection( const float *A, const float *K, const int w, const int h, float *direction)
{
	float a = w/2, b=h/2;
	direction[2] = 1/A[8];
	direction[1] = (1/A[4]) * (b - A[5]*direction[2]);
	direction[0] = (1/A[0]) * (a - A[1]*direction[1] - A[2]*direction[2]);
}

/**
 * @brief dessine la fenêtre openGL.
 * @details Le fond correspond à l'image récupéré de la caméra.
 * 
 * La mire est dessinée à l'endroit et la position de la mire filmée.
 * 
 * La théière est posé sur la mire.
 * 
 * @param A la matrice des paramètres intrinsèques
 * @param K le vecteur des coefficients de distortion
 * @param R la rotation repère global (repère de la mire) vers repère caméra
 * @param T la translation repère global vers repère caméra
 */
void glDrawFromCamera( const float *A, const float *K, const float *R, const float *T) 
{

	//affichage de l'image de la caméra dans la fenêtre openGL
	glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,1 ,0 ,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLuint texture;	

	cv :: Mat img;
	cameraUVC_getFrame( &camera , &img);

	cv :: flip(img,img,0);
	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);

	glBegin(GL_QUADS);  
 	glTexCoord2d(0,0);  glVertex2f(0,0);
    glTexCoord2d(1,0);  glVertex2f(1,0);
    glTexCoord2d(1,1);  glVertex2f(1,1);
    glTexCoord2d(0,1);  glVertex2f(0,1);
    glEnd();

	glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

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
	dessineMire(ligneMire, colonneMire, tailleCarreau);
	dessineTeaPot();
}

/**
 * @brief affichage des fenêtres openGL et openCV
 */
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

/**
 * @brief gestion du clavier
 */
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
 
/**
 * @brief Does everything needed before losing control to the main
 * OpenGL event loop.  
 */
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

/**
 * @brief The main() function.  Inits OpenGL.  Calls our own init function,
 * @details [long description]
 * 
 * @param argc [description]
 * @param argv [description]
 * 
 * @return [description]
 */
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

