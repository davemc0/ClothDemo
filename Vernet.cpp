// ---------------------------------------------------
// Cloth demo
// ---------------------------------------------------

#include "Cloth.h"
#include "Util/Assert.h"

// OpenGL
#include "GL/glew.h"

// This needs to come after GLEW
#include "GL/freeglut.h"
#include "Math/Vector.h"

// Globals
bool paused = true;
bool wireMode = false;
bool sphereMode = false;
int WW = 512, WH = 512;
Cloth* pCloth;
int CallingDefaultMouseFuncsOn = 1;

void userReshapeFunc0(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float Yfov = 45;              // VERTICAL FIELD OF VIEW IN DEGREES
    float Aspect = w / double(h); // WIDTH OVER HEIGHT
    float Near = 0.1f;            // NEAR PLANE DISTANCE
    float Far = 1000.0f;          // FAR PLANE DISTANCE

    gluPerspective(Yfov, Aspect, Near, Far);
}

// Display Function
void userDisplayFunc0()
{
    // Set up view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    f3vec LookAtCntr(0, 0, 0);
    f3vec Eye(0, 30, 80), Up(0, 1, 0);
    gluLookAt(Eye.x, Eye.y, Eye.z, LookAtCntr.x, LookAtCntr.y, LookAtCntr.z, Up.x, Up.y, Up.z);

    // glEnable(GL_LIGHTING);
    // glEnable(GL_LIGHT0);
    // glShadeModel(GL_SMOOTH);
    glClearColor(0, 1, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1, 0, 1); // XXX
    GL_ASSERT();
    // pCloth->Display(DRAW_POINTS);
    // pCloth->Display(DRAW_LINES);
    pCloth->Display(DRAW_TRIS);
    GL_ASSERT();

    glutSwapBuffers();
}

// Idle loop
void userIdleFunc0()
{
    // Update cloth
    if (!paused) { pCloth->TimeStep(); }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------
// USER-PROVIDED MOUSE HANDLING ROUTINE
//----------------------------------------------------------------------------
void userMouseFunc0(int button, int state, int x, int y)
{
    int Modifiers = glutGetModifiers();
    if (Modifiers & GLUT_ACTIVE_CTRL) printf("CTRL-");
    if (Modifiers & GLUT_ACTIVE_SHIFT) printf("SHIFT-");
    if (Modifiers & GLUT_ACTIVE_ALT) printf("ALT-");

    if (button == GLUT_LEFT_BUTTON)
        if (state == GLUT_DOWN)
            printf("LEFT-DOWN\n");
        else
            printf("LEFT-UP\n");
    if (button == GLUT_RIGHT_BUTTON)
        if (state == GLUT_DOWN)
            printf("RIGHT-DOWN\n");
        else
            printf("RIGHT-UP\n");
}

void userMotionFunc0(int x, int y) { printf("%d %d\n", x, y); }

//----------------------------------------------------------------------------
// USER-PROVIDED KEYBOARD HANDLING ROUTINE
//----------------------------------------------------------------------------
void userKeyboardFunc0(unsigned char Key, int x, int y)
{
    switch (Key) {
    case ' ':
        paused = !paused;
        std::cerr << "paused: " << paused << '\n';
        break;
    case 'r':
        if (pCloth) pCloth->Reset();
        break;
    case 'w':
        if (wireMode) {
            glPolygonMode(GL_FRONT, GL_FILL);
            glPolygonMode(GL_BACK, GL_FILL);
            wireMode = false;
        } else {
            glPolygonMode(GL_FRONT, GL_LINE);
            glPolygonMode(GL_BACK, GL_LINE);
            wireMode = true;
        }
        break;
    case 's': pCloth->WriteTriModel("tablecloth.tri"); break;
    case 'm':
        sphereMode = !sphereMode;
        pCloth->SetCollideMode(sphereMode);
        break;
    };
}

void userSpecialKeyFunc0(int Key, int x, int y)
{
    float dx = 2;
    int mod = glutGetModifiers();

    switch (Key) {
    case GLUT_KEY_LEFT: pCloth->MoveSphere(f3vec(-dx, 0, 0)); break;
    case GLUT_KEY_RIGHT: pCloth->MoveSphere(f3vec(dx, 0, 0)); break;
    case GLUT_KEY_UP:
        if (mod == GLUT_ACTIVE_CTRL)
            pCloth->MoveSphere(f3vec(0, dx, 0));
        else
            pCloth->MoveSphere(f3vec(0, 0, dx));
        break;
    case GLUT_KEY_DOWN:
        if (mod == GLUT_ACTIVE_CTRL)
            pCloth->MoveSphere(f3vec(0, -dx, 0));
        else
            pCloth->MoveSphere(f3vec(0, 0, -dx));
        break;
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
    glutInitWindowSize(WW, WH);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Cloth");

    // glEnable(GL_LIGHTING);
    // glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(userDisplayFunc0);
    glutIdleFunc(userIdleFunc0);
    glutMouseFunc(userMouseFunc0);
    glutMotionFunc(userMotionFunc0);
    glutKeyboardFunc(userKeyboardFunc0);
    glutSpecialFunc(userSpecialKeyFunc0);
    glutReshapeFunc(userReshapeFunc0);

    // Create cloth with starting point = P, timestep DT
    double dt = 0.03; //.03;
    f3vec startPos(0, 10, 0);
    pCloth = new Cloth(190, 190, 0.25f, 0.25f, startPos, dt, TABLECLOTH);
    // pCloth = new Cloth(90, 90, 0.5f, 0.5f, startPos, dt, TABLECLOTH);
    // pCloth = new Cloth(90, 90, 0.5f, 0.5f, startPos, dt, CURTAIN);

    GLfloat lightPos[] = {2.0, 0.0, 5.0, 0.0};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glutMainLoop();

    return 0;
}
