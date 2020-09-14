#include<gl/Freeglut.h>  

bool gbIsFullScreen = true;

int main(int argc, char *argv[])
{
	//function declarations
	void initialize(void);
	void uninitialize(void);
	void reshape(int, int);
	void display(void);
	void keyboard(unsigned char, int, int);
	void mouse(int, int, int, int);

	// code
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(0, 100);
	glutCreateWindow("First OpenGL Window by Abhijit A Gurav");

	initialize();

	// callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutCloseFunc(uninitialize);
	
	glutMainLoop();

}

void initialize()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void uninitialize()
{

}

void reshape(int iWidth, int iHeight)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

void display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glBegin(GL_TRIANGLES);
	
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(1.0f, -1.0f);

	glEnd();

	glFlush();

}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
			glutLeaveMainLoop();
			break;

		case 'F':
		case 'f':
			if (false == gbIsFullScreen)
			{
				glutFullScreen();
				gbIsFullScreen = true;
			}
			else
			{
				glutLeaveFullScreen();
				gbIsFullScreen = false;
			}
			break;
	}
}

void mouse(int iButton, int iState, int x, int y)
{
	switch (iButton)
	{
		case GLUT_LEFT_BUTTON:
			break;

		case GLUT_RIGHT_BUTTON:
			glutLeaveMainLoop();
			break;
	}
}

