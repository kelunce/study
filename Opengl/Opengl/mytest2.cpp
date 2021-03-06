/***************************************************************************/
/* This is a simple demo program written for CS 184 by Ravi Ramamoorthi    */
/* This program corresponds to the first OpenGL lecture.                   */
/*                                                                         */
/* Successive lectures/iterations make this program more complex.          */
/* This is the first simple program to draw a ground plane allowing zooming*/  
/* The intent is to show how to draw a simple scene.                       */
/***************************************************************************/


// I start with the modified first opengl program to do shaders
// I am now trying to do vertices/pixels in a modern way. 
// I am going to add cubes and teapots to my scene.

#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "shaders.h"
#include "geometry.h"

int mouseoldx, mouseoldy ; // For mouse motion
GLdouble eyeloc = 2.0 ; // Where to look from; initially 0 -2, 2
GLfloat teapotloc = -0.5 ; // ** NEW ** where the teapot is located
GLint animate = 0 ; // ** NEW ** whether to animate or not
GLuint vertexshader, fragmentshader, shaderprogram ; // shaders

const int DEMO = 4  ; // ** NEW ** To turn on and off features

void display(void)
{
	// clear all pixels  
	// If (DEMO >= 2) also clear the depth buffer 

	if (DEMO >= 2) glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ; 
	else glClear (GL_COLOR_BUFFER_BIT);

	// draw white polygon (square) of unit length centered at the origin
	// Note that vertices must generally go counterclockwise
	// Change from the first program, in that I just made it white.
	// The old OpenGL code of using glBegin... glEnd no longer appears. 
	// The new version uses vertex buffer objects from init.   

	// Does the order of drawing matter?  What happens if I draw the ground 
	// after the pillars?  I will show this in class 


	drawobject(FLOOR) ;

	// Now draw several cubes with different transforms, colors
	// I continue to use the deprecated push-pop and matrix mode 
	// Since it is convenient (or you have to write your own stack).  

	if (DEMO > 0) {

		glMatrixMode(GL_MODELVIEW) ; 

		// 1st pillar 
		glPushMatrix() ; 
		glTranslatef(-0.4,-0.4,0.0) ; 
		drawcolor(CUBE, 0) ; 
		glPopMatrix() ; 

		// 2nd pillar 
		glPushMatrix() ; 
		glTranslatef(0.4,-0.4,0.0) ; 
		drawcolor(CUBE, 1) ; 
		glPopMatrix() ; 

		// 3rd pillar 
		glPushMatrix() ; 
		glTranslatef(0.4,0.4,0.0) ; 
		drawcolor(CUBE, 2) ; 
		glPopMatrix() ; 

		// 4th pillar 
		glPushMatrix() ; 
		glTranslatef(-0.4,0.4,0.0) ; 
		drawcolor(CUBE, 3) ; 
		glPopMatrix() ; 
	}

	// Draw the glut teapot 
	// This is using deprecated old-style OpenGL certainly   
	if (DEMO >= 3) {
		//  ** NEW ** Put a teapot in the middle that animates 
		glColor3f(0.0,1.0,1.0) ; // Deprecated command to set the color 
		glPushMatrix() ;
		//  I now transform by the teapot translation for animation */
		glTranslatef(teapotloc, 0.0, 0.0) ;

		//  The following two transforms set up and center the teapot 
		//  Remember that transforms right-multiply the stack 

		glTranslatef(0.0,0.0,0.1) ;
		glRotatef(90.0,1.0,0.0,0.0) ;
		glutSolidTeapot(0.15) ; 
		glPopMatrix() ;
	}

	// Does order of drawing matter? 
	// What happens if I draw the ground after the pillars? 
	// I will show this in class.

	// drawobject(FLOOR) ; 

	// don't wait!  
	// start processing buffered OpenGL routines 

	if (DEMO >= 2) glutSwapBuffers() ; 
	glFlush ();
}

// ** NEW ** in this assignment, is an animation of a teapot 
// Hitting p will pause this animation; see keyboard callback

void animation(void) {
	teapotloc = teapotloc + 0.005 ;
	if (teapotloc > 0.5) teapotloc = -0.5 ;
	glutPostRedisplay() ;  
}


// Defines a Mouse callback to zoom in and out 
// This is done by modifying gluLookAt         
// The actual motion is in mousedrag           
// mouse simply sets state for mousedrag       
void mouse(int button, int state, int x, int y) 
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_UP) {
			// Do Nothing ;
		}
		else if (state == GLUT_DOWN) {
			mouseoldx = x ; mouseoldy = y ; // so we can move wrt x , y 
		}
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) 
	{ // Reset gluLookAt
		eyeloc = 2.0 ; 
		glMatrixMode(GL_MODELVIEW) ;
		glLoadIdentity() ;
		gluLookAt(0,-eyeloc,eyeloc,0,0,0,0,1,1) ;
		glutPostRedisplay() ;
	}
}

void mousedrag(int x, int y) {
	int yloc = y - mouseoldy  ;    // We will use the y coord to zoom in/out
	eyeloc  += 0.005*yloc ;         // Where do we look from
	if (eyeloc < 0) eyeloc = 0.0 ;
	mouseoldy = y ;

	/* Set the eye location */
	glMatrixMode(GL_MODELVIEW) ;
	glLoadIdentity() ;
	gluLookAt(0,-eyeloc,eyeloc,0,0,0,0,1,1) ;

	glutPostRedisplay() ;
}

// Defines what to do when various keys are pressed 
void keyboard (unsigned char key, int x, int y) 
{
	switch (key) {
	case 27:  // Escape to quit
		exit(0) ;
		break ;
	case 'p': // ** NEW ** to pause/restart animation
		animate = !animate ;
		if (DEMO >= 3) {
			if (animate) glutIdleFunc(animation) ;
			else glutIdleFunc(NULL) ;
		}
		break ;
	default:
		break ;
	}
}

/* Reshapes the window appropriately */
void reshape(int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Think about the rationale for this choice for gluPerspective 
	// What would happen if you changed near and far planes? 
	gluPerspective(30.0, (GLdouble)w/(GLdouble)h, 1.0, 10.0) ;

}


void init (void) 
{
	/* select clearing color 	*/
	glClearColor (0.0, 0.0, 0.0, 0.0);

	/* initialize viewing values  */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Think about this.  Why is the up vector not normalized?
	glMatrixMode(GL_MODELVIEW) ;
	glLoadIdentity() ;
	gluLookAt(0,-eyeloc,eyeloc,0,0,0,0,1,1) ;


	// Set up the Geometry for the scene.  
	// From OpenGL book pages 103-109



	glGenBuffers(numperobj*numobjects+ncolors, buffers) ; 
	initcolorscube() ; 

	initobject(FLOOR, (GLfloat *) floorverts, sizeof(floorverts), (GLfloat *) floorcol, sizeof (floorcol), (GLubyte *) floorinds, sizeof (floorinds), GL_POLYGON) ; 
	//       initobject(CUBE, (GLfloat *) cubeverts, sizeof(cubeverts), (GLfloat *) cubecol, sizeof (cubecol), (GLubyte *) cubeinds, sizeof (cubeinds), GL_QUADS) ; 
	initobjectnocol(CUBE, (GLfloat *) cubeverts, sizeof(cubeverts), (GLubyte *) cubeinds, sizeof (cubeinds), GL_QUADS) ; 

	if (DEMO >= 2) {
		// Enable the depth test
		glEnable(GL_DEPTH_TEST) ;
		glDepthFunc (GL_LESS) ; // The default option
	}

	vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/nop.vert") ;
	fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/nop.frag") ;
	shaderprogram = initprogram(vertexshader, fragmentshader) ; 

}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	// Requests the type of buffers (Single, RGB).
	// Think about what buffers you would need...

	// Request the depth if needed, later swith to double buffer 
	if (DEMO >= 2) glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	else glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB) ;

	glutInitWindowSize (500, 500); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ("Simple Demo with Shaders");
	GLenum err = glewInit() ; 
	if (GLEW_OK != err) { 
		std::cerr << "Error: " << glewGetString(err) << std::endl; 
	} 
	init (); // Always initialize first

	// Now, we define callbacks and functions for various tasks.
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape) ;
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse) ;
	glutMotionFunc(mousedrag) ;

	glutMainLoop(); // Start the main code
	return 0;   /* ANSI C requires main to return int. */
}
