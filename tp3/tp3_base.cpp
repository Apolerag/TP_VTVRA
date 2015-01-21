
#include <stdio.h>   // Always a good idea.
#include <string.h>   // Always a good idea.
#include <locale.h>

#include <GL/gl.h>   // OpenGL itself.
#include <GL/glu.h>  // GLU support library.
#include <GL/glut.h> // GLUT support library.


// Some global variables.

int Window_ID;

// camera resolution
int windowWidth = 640;
int windowHeight = 480;


void calculerFrustum( const float *A, const float *K, float w, float h, float *frustum)
{
	frustum[0] = -0.1f; // left
	frustum[1] =  0.1f; // right
	frustum[2] = -0.1f; // bottom
	frustum[3] =  0.1f; // top
	frustum[4] =  0.1f; // near
	frustum[5] = 100.0f; // far
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

	// exemple : dessine un carré rouge
	/*glColor3f( 1.0f, 0.0f, 0.0f);
	glVertex3f( 0.0f, 0.5f, 0.0f);
	glVertex3f( 0.5f, 1.0f, 0.0f);
	glVertex3f( 1.0f, 0.5f, 0.0f);
	glVertex3f( 0.5f, 0.0f, 0.0f);*/
	
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


// calcule la transformation GtoC
// GtoC = Global to Camera = Mire to Camera
void calculerTransformation( const float *R, const float *T, float *GtoC)
{
}


void glDrawFromCamera( const float *A, const float *K, const float *R, const float *T) 
{
	float frustum[6], GtoC[16];
	calculerFrustum( A, K, windowWidth, windowHeight, frustum);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum( frustum[0], frustum[1], frustum[2], frustum[3], frustum[4], frustum[5]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -1.0);
	dessineAxes(1.0);
	dessineMire( 5, 4, 0.05);
}


// ------
// Drawing function
void cbRenderScene(void)
{
	// desactive textures
	glDisable(GL_TEXTURE_2D);

	// active z-buffering pour masquage partie cachee
	glEnable(GL_DEPTH_TEST); 

	// Clear the color and depth buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// dessine dans le point de vue de la caméra
	float dummy;
	glDrawFromCamera( &dummy, &dummy, &dummy, &dummy);

	// All done drawing.  Let's show it.
	glutSwapBuffers();
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

	// Pass off control to OpenGL.
	// Above functions are called as appropriate.
	glutMainLoop();

	return 1;
}

