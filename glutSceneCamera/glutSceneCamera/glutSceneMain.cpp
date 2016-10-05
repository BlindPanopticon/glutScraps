#include<iostream>
#include<glut.h>
#include<math.h>
#define PI 3.1415926f		//internal PI
#define FPSLOCK 0.25f		//Idle FPS Max fps
#define SMALLNUMBER 0.0001f	//Number Used For Offseting Values Slightly
#define GRIDLINECOUNT 400	//Number of Lines in Ground Grid
#define DEBUGOUTPUT true	//Pretoggle Output

#define ZOOMSTEPSIZE 1		//Size of a zoom step on camera radius				lu
#define CARTESIANSTEPSIZE 1	//Size of a cartesian step in						lu
#define POLARSTEPSIZE 1		//Size of a polar step in							degrees
#define FINESTEPMULT .1		//Multiplier for key event when alt is held down	lu

//Program Internal Data
int		windowIdentifier;
char	 *type = "NULL";

//FPS Data
	int
	//FPS Estimate Timers			ms
		timeLastFPSUpdate = 0,
		timeNowFPSUpdate = 0,
	//Idle FPS Limit Timers			ms
		timeLast = 0,
		timeNow = 0;
	//FPS Limit before redisplay	ms
		float	fpsLimit = 1000.0f / FPSLOCK;

//Camera Data
	float
	//Cartesian						lu
		camPos[] = {-20.0f, -20.0f, 20.0f},
		camTargetPos[] = {0.0f, 0.0f, 0.0f},
	//Polar							degrees/degrees/lu
		camTheta = 45.0f,
		camPhi = 45.0f,
		camRadius = 30.0f;
//Toggles
	bool
	//Viewport
		polarTarget = true,
		cartesianCamera = false,
		cartesianTarget = false,
	//Toggles
		targetToggle = true,
		axisToggle = true,
		gridToggle = true,
	//Debug
		fpsToggle = DEBUGOUTPUT,
		camPosToggle = DEBUGOUTPUT,
		eventLogToggle = DEBUGOUTPUT;

void printGlutCam(){
	//Outputs current camera cartesian, target polar and target cartesian

	std::cout << "Camera:\t" << camPos[0] << "\t\t" << camPos[1] << "\t\t" << camPos[2] << '\n';
	std::cout << "Target:\t" << camTargetPos[0] << "\t\t" << camTargetPos[1] << "\t\t" << camTargetPos[2] << '\n';
	std::cout << "Polar:\t" << camRadius << "\t\t" << camTheta << "\t\t" << camPhi << '\n';
}
void updateGlutFPS(){
	//Makes and outputs FPS Estimate

	timeNowFPSUpdate = glutGet(GLUT_ELAPSED_TIME);
	std::cout << "FPS:\t" << 1000.0 / (timeNowFPSUpdate - timeLastFPSUpdate) << '\n';
	timeLastFPSUpdate = timeNowFPSUpdate;
}

void startGlutWindow(int xWinPos = 0, int yWinPos = 0, int xWinLength = 100, int yWinLength = 100){
	//Places, sizes and initializes GLUT window

	glutInitWindowPosition(xWinPos, yWinPos);
	glutInitWindowSize(xWinLength, yWinLength);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	windowIdentifier = glutCreateWindow("GLUT Scene");

	if(eventLogToggle) std::cout << "Made a GLUT Window With ID:\t" << windowIdentifier << '\n';
}

void axisObject(float length = 100.0f){
	//Builds positive XYZ axis

	glTranslatef(0.0f, 0.0f, 0.0f);
	//X axis
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(length, 0.0f, 0.0f);
	glEnd();

	//Y axis
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, length, 0.0f);
	glEnd();

	//Z axis
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, length);
	glEnd();
}
void gridObject(){
	//builds XY grid centered at 0,0,0

	float length = GRIDLINECOUNT / 2;
	glColor3f(0.15f, 0.15f, 0.15f);
	glTranslatef(0.0f, 0.0f, 0.0f);
	for(int x = -(GRIDLINECOUNT / 2); x < (GRIDLINECOUNT / 2) + 1; x++){
		//X Parallel
		glBegin(GL_LINES);
			glVertex3f(-length, (float)x, 0.0f);
			glVertex3f(length, (float)x, 0.0f);
		glEnd();
		
		//Y Parallel
		glBegin(GL_LINES);
			glVertex3f((float)x, -length, 0.0f);
			glVertex3f((float)x, length, 0.0f);
		glEnd();
	}
}
void targetObject(float radius = 0.1f){
	//builds a yellow sphere indicating position of camera's target

	glColor3f(1.0f, 1.0f, 0.0f);
	glPushMatrix();
		glTranslatef(camTargetPos[0], camTargetPos[1], camTargetPos[2]);
		glutSolidSphere(radius, 6, 6);
	glPopMatrix();
}

void getCartesianFromPolar(){
	//Generates a new cartesian point for camera from polar data and camera target point

	//Limit Phi (1 to 360), Theta (-90 to 90) and Radius (0 to inf)
	if(camTheta > 90.0f) camTheta = 90.0f - SMALLNUMBER;
	if(camTheta < -90.0f) camTheta = -90.0f + SMALLNUMBER;
	if(camPhi > 360.0f) camPhi -= 360.0f;
	if(camPhi < 0) camPhi += 360.0f;
	if(camRadius < 0) camRadius = 0.0f;

	//Get a XY radius
	float temp = camRadius * cos((camTheta * PI) / 180);

	//Push calculated XYZ to camera cartesian position
	camPos[0] = (temp * cos((camPhi * PI) / 180)) + camTargetPos[0];
	camPos[1] = (temp * sin((camPhi * PI) / 180)) + camTargetPos[1];
	camPos[2] = (camRadius * sin((camTheta * PI) / 180)) + camTargetPos[2];
}
void getPolarFromCartesian(){
	//Generates a polar coordinate set centered at camera target from camera and target cartesian points
	float deltaX = camPos[0] - camTargetPos[0];
	float deltaY = camPos[1] - camTargetPos[1];
	float deltaZ = camPos[2] - camTargetPos[2];
	float temp = sqrt(pow(deltaX, 2) + pow(deltaY, 2));

	//Avoid 0 divide
	if(deltaX == 0) deltaX += SMALLNUMBER;
	if(temp == 0) temp += SMALLNUMBER;

	//Get Radius
	camRadius = sqrt(pow(deltaX, 2) + pow(deltaY, 2) + pow(deltaZ, 2)); 

	//Get Phi
	camPhi = (180.0f / PI) * atan(deltaY / deltaX);
	//Correct for location (-90 to 90 >> 0 to 360)
	if((deltaX <= 0) & (deltaY >= 0)) camPhi = 180 + camPhi;
	if((deltaX <= 0) & (deltaY <= 0)) camPhi = 180 + abs(camPhi);
	if((deltaX >= 0) & (deltaY <= 0)) camPhi = 360 + camPhi;

	//Get Theta
	camTheta = (180.0f / PI) * atan(deltaZ / temp );
}
void setGlutCamera(float *cam = camPos, float *target = camTargetPos){
	//Build camera at a polar or cartesian point torwards cartesian camera target point

	//Get cartesian point if in polar mode
	if(polarTarget) 
		getCartesianFromPolar();

	//Made to remain 'upright'
	if((cam[0] == 0) & (cam[1] == 0)) cam[0] = SMALLNUMBER;

	//Print camera data if enabled
	if(camPosToggle) printGlutCam();

	//Generate/Update polar data for smooth transition between polar and cartesian viewports
	getPolarFromCartesian();

	//Call a redisplay
	glutPostRedisplay();
}

void clearGlutBuffer(){
	//Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void renderGlutView(){
	//Erase
	clearGlutBuffer();

	glLoadIdentity();

	//Setup Camera
	gluLookAt(camPos[0], camPos[1], camPos[2], camTargetPos[0], camTargetPos[1], camTargetPos[2], 0.0f, 0.0f, 1.0f);

	//Draw a Ground Grid
	if(gridToggle) gridObject();
	//Draw a 3-Axis
	if(axisToggle) axisObject();
	//Draw a Target
	if(targetToggle) targetObject();

	//Finalize
	glutSwapBuffers();

	//Estimate FPS
	if(fpsToggle) updateGlutFPS();
}
void limitGlutFPS(){
	//Locks Maximum FPS, redisplays if idle and under FPS limit
	timeNow = glutGet(GLUT_ELAPSED_TIME);
	if( timeNow - timeLast >= fpsLimit){
		timeLast = timeNow;
		glutPostRedisplay();
	}
}

void resizeGlutView(int xWinLength, int yWinLength){
	//Resolves change in window reosolution/ratio

	//Prep Values
	if(yWinLength == 0) yWinLength = 1;
	float ratio = 1.0f * xWinLength / yWinLength;

	//Prep GLUT
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//Set View Parameters
	glViewport(0, 0, xWinLength, yWinLength);
	gluPerspective(45, ratio, 0.1, 100);

	//Finalize
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();

	if(eventLogToggle) std::cout << "Window Resized to\n\tX:\t" << xWinLength << "\n\tY:\t" << yWinLength << "\nRatio is Now:\t" << ratio <<  '\n';
}

void specialGlutKey(int key, int x, int y){
	//Catches special key events

	//Reduces step size on alt held down
	float stepModifier = 1;
	if(glutGetModifiers() == GLUT_ACTIVE_ALT){
		stepModifier = FINESTEPMULT;
	}

	//Polar Target
	if(polarTarget==true){
		switch(key){
			//Zoom using radius
			case GLUT_KEY_HOME:
				camRadius -= ZOOMSTEPSIZE * stepModifier;
				type = "ZOOM_INC";
				break;
			case GLUT_KEY_END:
				camRadius += ZOOMSTEPSIZE * stepModifier;
				type = "ZOOM_DEC";
				break;

			//Elevation using theta
			case GLUT_KEY_UP:
				camTheta += POLARSTEPSIZE * stepModifier;
				type = "TAR_THETA_INC";
				break;
			case GLUT_KEY_DOWN:
				camTheta -= POLARSTEPSIZE * stepModifier;
				type = "TAR_THETA_DEC";
				break;

			//Rotation using phi
			case GLUT_KEY_LEFT:
				camPhi -= POLARSTEPSIZE * stepModifier;
				type = "TAR_PHI_DEC";
				break;
			case GLUT_KEY_RIGHT:
				camPhi += POLARSTEPSIZE * stepModifier;
				type = "TAR_PHI_INC";
				break;
		}
		
	}
	//Cartesian Camera
	if(cartesianCamera==true){
		switch(key){
			//X
			case GLUT_KEY_LEFT:
				camPos[0] -= CARTESIANSTEPSIZE * stepModifier;
				type = "CAM_X_DEC";
				break;
			case GLUT_KEY_RIGHT:
				camPos[0] += CARTESIANSTEPSIZE * stepModifier;
				type = "CAM_X_INC";
				break;

			//Y
			case GLUT_KEY_UP:
				camPos[1] += CARTESIANSTEPSIZE * stepModifier;
				type = "CAM_Y_INC";
				break;
			case GLUT_KEY_DOWN:
				camPos[1] -= CARTESIANSTEPSIZE * stepModifier;
				type = "CAM_Y_DEC";
				break;

			//Z
			case GLUT_KEY_PAGE_UP:
				camPos[2] += CARTESIANSTEPSIZE * stepModifier;
				type = "CAM_Z_INC";
				break;
			case GLUT_KEY_PAGE_DOWN:
				camPos[2] -= CARTESIANSTEPSIZE * stepModifier;
				type = "CAM_Z_DEC";
				break;
		}
	}
	//Cartesian Target
	if(cartesianTarget==true){
		switch(key){
			//X
			case GLUT_KEY_LEFT:
				camTargetPos[0] -= CARTESIANSTEPSIZE * stepModifier;
				type = "TAR_X_DEC";
				break;
			case GLUT_KEY_RIGHT:
				camTargetPos[0] += CARTESIANSTEPSIZE * stepModifier;
				type = "TAR_X_INC";
				break;

			//Y
			case GLUT_KEY_UP:
				camTargetPos[1] += CARTESIANSTEPSIZE * stepModifier;
				type = "TAR_Y_INC";
				break;
			case GLUT_KEY_DOWN:
				camTargetPos[1] -= CARTESIANSTEPSIZE * stepModifier;
				type = "TAR_Y_DEC";
				break;

			//Z
			case GLUT_KEY_PAGE_UP:
				camTargetPos[2] += CARTESIANSTEPSIZE * stepModifier;
				type = "TAR_Z_INC";
				break;
			case GLUT_KEY_PAGE_DOWN:
				camTargetPos[2] -= CARTESIANSTEPSIZE * stepModifier;
				type = "TAR_Z_DEC";
				break;
		}
	}
	if(eventLogToggle) std::cout << "Special Key Event:\t" << key << " | " << type << '\n';
	
	//Interaction has occured, redisplay
	setGlutCamera();
}

void mainMenuEvent(int choice){}
void viewportMenuEvent(int choice){
	//Viewport menu events

	switch(choice){
		case 1:
			polarTarget = true;
			cartesianCamera = false;
			cartesianTarget = false;
			type = "POLAR_TAR";
			break;
		case 2:
			polarTarget = false;
			cartesianCamera = true;
			cartesianTarget = false;
			type = "CARTE_CAM";
			break;
		case 3:
			polarTarget = false;
			cartesianCamera = false;
			cartesianTarget = true;
			type = "CARTE_TAR";
			break;
	}
	std::cout << "Viewport Menu Event:\t" << choice << " | " << type << '\n';
}
void cameraMenuEvent(int choice){
	//Camera display menu events

	switch(choice){
		case 0:
			targetToggle = !targetToggle;
			type = "TAR_TOG";
			break;
		case 1:
			axisToggle = !axisToggle;
			type = "AXIS_TOG";
			break;
		case 2:
			gridToggle = !gridToggle;
			type = "GRID_TOG";
			break;
	}
	//glutPostRedisplay();
	std::cout << "Camera Menu Event:\t" << choice << " | " << type << '\n';
}
void debugMenuEvent(int choice){
	//Debug menu events

	switch(choice){
		case 0:
			fpsToggle = !fpsToggle;
			type = "FPS_TOG";
			break;
		case 1:
			camPosToggle = !camPosToggle;
			type = "CAM_POS_TOG";
			break;
		case 2:
			eventLogToggle = !eventLogToggle;
			type = "EVENT_LOG_TOG";
			break;
	}
	std::cout << "Debug Menu Event:\t" << choice << " | " << type << '\n';
}
void buildMainGlutMenu(){
	//Builds main menu and submenus

	int menuViewport = glutCreateMenu(viewportMenuEvent);
	glutAddMenuEntry("Polar Target", 1);
	glutAddMenuEntry("Cartesian Camera", 2);
	glutAddMenuEntry("Cartesian Target", 3);

	int menuCamera = glutCreateMenu(cameraMenuEvent);
	glutAddSubMenu("Viewport", menuViewport);
	glutAddMenuEntry("Toggle Target Icon", 0);
	glutAddMenuEntry("Toggle Axis Display", 1);
	glutAddMenuEntry("Toggle Ground Grid", 2);

	int menuDebug = glutCreateMenu(debugMenuEvent);
	glutAddMenuEntry("Toggle FPS", 0);
	glutAddMenuEntry("Toggle Camera Position", 1);
	glutAddMenuEntry("Toggle Event Logging", 2);

	glutCreateMenu(mainMenuEvent);
	glutAddSubMenu("Camera", menuCamera);
	glutAddSubMenu("Debug", menuDebug);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv){
	//Start Glut
	glutInit(&argc, argv);

	//Start Window
	startGlutWindow(100, 100, 800, 450);
	
	//Callbacks
	glutDisplayFunc(renderGlutView);
	glutReshapeFunc(resizeGlutView);
	glutIdleFunc(limitGlutFPS);

	//Key Events
	glutSpecialFunc(specialGlutKey);

	//Menu
	buildMainGlutMenu();

	//Preload Polar Camera
	setGlutCamera();

	//Run
	glutMainLoop();

	return 1;
}