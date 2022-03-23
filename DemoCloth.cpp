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

// User Interface Globals
bool paused = false, fullScreen = false;
int WW = 512, WH = 512, constraintIters = 10;
DrawMode drawMode = DRAW_TRIS;
ClothStyle clothStyle = TABLECLOTH;
CollisionObjects collisionObjects = COLLIDE_SPHERES;
Cloth* pCloth;

void userReshapeFunc0(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float Yfov = 45;             // VERTICAL FIELD OF VIEW IN DEGREES
    float Aspect = w / float(h); // WIDTH OVER HEIGHT
    float Near = 0.1f;           // NEAR PLANE DISTANCE
    float Far = 1000.0f;         // FAR PLANE DISTANCE

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
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pCloth->Display(drawMode);
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

void userKeyboardFunc0(unsigned char Key, int x, int y)
{
    switch (Key) {
    case ' ':
        paused = !paused;
        std::cerr << "paused: " << paused << '\n';
        break;
    case 'f':
        fullScreen = !fullScreen;
        if (fullScreen) {
            glutFullScreen();
        } else {
            glutReshapeWindow(WW, WH);
            glutPositionWindow(0, 0);
        }
        break;
    case '+':
    case '=':
        constraintIters++;
        std::cerr << "constraintIters: " << constraintIters << '\n';
        pCloth->SetConstraintIters(constraintIters);
        break;
    case '-':
    case '_':
        constraintIters = max(constraintIters - 1, 0);
        std::cerr << "constraintIters: " << constraintIters << '\n';
        pCloth->SetConstraintIters(constraintIters);
        break;
    case 'c':
        clothStyle = static_cast<ClothStyle>((clothStyle + 1) % NUM_CLOTH_STYLES);
        std::cerr << "clothStyle: " << clothStyle << '\n';
        pCloth->Reset(clothStyle);
        break;
    case 'r':
        if (pCloth) pCloth->Reset(clothStyle);
        break;
    case 'w':
        drawMode = static_cast<DrawMode>((drawMode + 1) % NUM_DRAW_MODES);
        std::cerr << "drawMode: " << drawMode << '\n';
        break;
    case 's': pCloth->WriteTriModel("tablecloth.tri"); break;
    case 'm':
        collisionObjects = static_cast<CollisionObjects>((collisionObjects + 1) % NUM_COLLISION_OBJECTS);
        std::cerr << "collisionObjects: " << collisionObjects << '\n';
        pCloth->SetCollideObjectType(collisionObjects);
        break;
    case 'q':
    case '\033': /* ESC key: quit */ exit(0); break;
    };
}

void userSpecialKeyFunc0(int Key, int x, int y)
{
    float dx = 2;
    int mod = glutGetModifiers();

    switch (Key) {
    case GLUT_KEY_LEFT: pCloth->MoveColliders(f3vec(-dx, 0, 0)); break;
    case GLUT_KEY_RIGHT: pCloth->MoveColliders(f3vec(dx, 0, 0)); break;
    case GLUT_KEY_UP:
        if (mod == GLUT_ACTIVE_CTRL)
            pCloth->MoveColliders(f3vec(0, dx, 0));
        else
            pCloth->MoveColliders(f3vec(0, 0, -dx));
        break;
    case GLUT_KEY_DOWN:
        if (mod == GLUT_ACTIVE_CTRL)
            pCloth->MoveColliders(f3vec(0, -dx, 0));
        else
            pCloth->MoveColliders(f3vec(0, 0, dx));
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
    f3vec startPos(0, 25, 0);
    float dt = 0.03f;
    float damping = 0.9f;
    // pCloth = new Cloth(200, 200, 0.25f, 0.25f, startPos, dt, damping, clothStyle);
    pCloth = new Cloth(100, 100, 0.5f, 0.5f, startPos, dt, damping, clothStyle);
    pCloth->SetCollideObjectType(collisionObjects);
    pCloth->SetConstraintIters(constraintIters);

    GLfloat lightPos[] = {2.0, 30.0, 5.0, 1.0};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glutMainLoop();

    return 0;
}
