//Author: Aven Bross
//Date: 12-13-2013
//
//Demonstration of sphere tree collision detection

//OpenGL/GLUT includes - DO THESE FIRST
#include <cstdlib>       //Do this before GL/GLUT includes
using std::exit;
#ifndef __APPLE__
# include <GL/glew.h>
# include <GL/glut.h>    //Includes OpenGL headers as well
#else
# include <GLEW/glew.h>
# include <GLUT/glut.h>  //Apple puts glut.h in a different place
#endif
#ifdef _MSC_VER          //Tell MS-Visual Studio about GLEW lib
# pragma comment(lib, "glew32.lib")
#endif

//Other includes
#include "lib/lib381/bitmapprinter.h"
                         //For class BitmapPrinter
#include "lib/lib381/glslprog.h"
                         //For GLSL code-handling functions
#include "lib/lib381/globj.h"
                         //For class Tex2D
#include "lib/lib381/tshapes.h"
                         //For shape-drawing functions
#include "lib/lib381/whereami.h"
                         //For function whereAmI
#include <iomanip>
using std::setprecision;
using std::fixed;
#include <sstream>
using std::ostringstream;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iostream>
using std::cerr;
using std::endl;

#include "lib/object_w_tree.h"

//Global variables
//Keyboard
const int ESCKEY = 27;         //ASCII value of Escape

//Window/viewport
const int startwinsize = 600;  //Start window width & height (pixels)
int winw = 1, winh = 1;        //Window width, height (pixels)
                               //(Initialize to avoid spurious errors)

//Objects
GLdouble cammat[16];           //Saved camera transformation

double savetime = 0.;

//Keeps track of texure size
const int h = 7;

vector<Object*> objects;

bool drawSolid;
int sphereTreeDepth;
bool pause;

double nextSampleTime;

GLhandleARB theprog;

//Draw all objects in the scene
void drawObjects()
{
	for(int i=0; i<objects.size(); i++){
		glPushMatrix();
		if(drawSolid) objects[i]->draw();
		else objects[i]->drawSphereTree();
		glPopMatrix();
	}
}

//The GLUT display function
void myDisplay()
{
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Set up 3-D
	glEnable(GL_DEPTH_TEST);
	glLoadMatrixd(cammat);

	//Make light source at player position
	glUseProgramObjectARB(theprog);
	glColor3d(0.,0.,1.);
	drawObjects();

	//Draw documentation
	glUseProgramObjectARB(0);
	glDisable(GL_DEPTH_TEST);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);  //Set up simple ortho projection
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0., double(winw), 0., double(winh));
	glColor3d(.25,.5,.75);
	glColor3d(0., 0., 0.);        //Black text
	BitmapPrinter p(20., winh-20., 20.);

    // Print text
	ostringstream ostr;
	ostr << "Sphere Tree Depth: " << sphereTreeDepth;
	p.print(ostr.str());
	p.print("Q, W    Decrease, increase tree depth");
	p.print("M       Show/hide mesh");
	p.print("P       Pause");
	p.print("Space   Reset");
	p.print("Esc     Exit");
	
	glPopMatrix();                //Restore previous projection
	glMatrixMode(GL_MODELVIEW);

	glutSwapBuffers();
}

//Rough calculation of how long we can wait before checking for calculations
double getNextSampleTime(){
	double minDist=-1.;
	double maxVel=0.;
	for(int i=0; i<objects.size(); i++){
		double vel = objects[i]->getVel();
		if(vel>maxVel) maxVel=vel;

		if(i==objects.size()-1) break;
		for(int j=i+1; j<objects.size(); j++){
			double dist=objects[i]->getDist(*objects[j]);
			if(dist<minDist || minDist<0.)	minDist=dist;
		}
	}
	return (minDist/maxVel)/2.;
}

//Checks for objects collisions
double checkCollisions(){
	for(int i=0; i<objects.size()-1; i++){
		for(int j=i+1; j<objects.size(); j++){
			//Only check for collisions if one of the objects have moved
			if((objects[i])->getVel() != 0. || (objects[j])->getVel() != 0){
				if(objects[i]->checkCollision(*objects[j])){
					if(objects[i]->_colliding == false){
						objects[i]->_colliding = true;
						//Have objects bounce if they are colliding for the first time
						objects[i]->bounce(*objects[j]);
					}
				}
				else objects[i]->_colliding = false;
			}
		}		
	}
}

//The GLUT idle function
void myIdle()
{
	if(pause) return;
        //Compute elapsed time since last update
	double currtime = glutGet(GLUT_ELAPSED_TIME) / 1000.;
	double elapsedTime = currtime - savetime;
	savetime = currtime;

	if(elapsedTime>.1) elapsedTime = .1;
	
	if(currtime>=nextSampleTime){
		//Check for collisions between objects
		checkCollisions();
		nextSampleTime = getNextSampleTime();
	}
	
	//Update each object
	for(int i=0; i<objects.size(); i++)
		objects[i]->update(elapsedTime);

	glutPostRedisplay();
}

void reset(){
	objects.clear();

	objects.push_back(new Sphere(2., 0., 8., 1.));
	objects.push_back(new Cube(-2., 0., 8., 1., sphereTreeDepth));

	//Make one object move so we have a collision
	(objects[0])->setVel(-1., 0., 0.);

	glutPostRedisplay();
}

//The GLUT special function
void myKeyboard(unsigned char key, int x, int y)
{
	switch(key){
	case 'm':
	case 'M': 
		drawSolid=!drawSolid;
		break;
	case ' ': 
		reset();
		break;
	case 27:
		exit(0);
		break;
	case 'w':
	case 'W':
	    if(sphereTreeDepth < 4)
		    sphereTreeDepth++;
		break;
	case 'q':
	case 'Q':
	    if(sphereTreeDepth > 0)
		    sphereTreeDepth--;
		break;

	case 'p':
	case 'P':
		pause = !pause;
		break;
	}
}

//The GLUT reshape function
void myReshape(int w, int h)
{
    //Set viewport & save window dimensions in globals
    glViewport(0, 0, w, h);
    winw = w;
    winh = h;

    //Set up projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60., double(w)/h, 0.5, 100.);
    glMatrixMode(GL_MODELVIEW);  //Always go back to model/view mode
}

//Initialize GL states & global data
//Called by main after window creation
void init()
{
	savetime = glutGet(GLUT_ELAPSED_TIME)/1000.;
	nextSampleTime = savetime;
	
	sphereTreeDepth = 1;

	drawSolid = true;
	pause = false;

	reset();

	theprog = makeProgramObjectFromFiles("lib/shaders/twoside_v.glsl", "lib/shaders/twoside_f.glsl");

	glLoadIdentity();
	glTranslated(0., 0., 0.);
	glRotated(180., 0., 1., 0.);
	glGetDoublev(GL_MODELVIEW_MATRIX, cammat);
}

//Main Function
int main(int argc, char ** argv)
{
    //Initialize OpenGL/GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    //Make a window
    glutInitWindowSize(startwinsize, startwinsize);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Sphere Map Collision Detection");

    // Init GLEW & check status
    if (glewInit() != GLEW_OK)
    {
        cerr << "glewInit failed" << endl;
        exit(1);
    }

    //Initialize GL states & register GLUT callbacks
    init();
    glutDisplayFunc(myDisplay);
    glutIdleFunc(myIdle);
    glutKeyboardFunc(myKeyboard);
    //glutSpecialFunc(mySpecial);
    glutReshapeFunc(myReshape);

    //Do something
    glutMainLoop();

    return 0;
}

