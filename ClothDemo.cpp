// ---------------------------------------------------
// Cloth demo
// ---------------------------------------------------

#include "Cloth.h"
#include "Math/Vector.h"
#include "Util/Assert.h"
#include "Util/Timer.h"

// OpenGL
#include "GL/glew.h"

// This needs to come after GLEW
#include "GL/freeglut.h"

// User Interface Globals
bool paused = false, fullScreen = false;
int WW = 1024, WH = 1024, constraintIters = 50; // If it runs slow reduce constraintIters first.
int stiffening = 1;                             // If > 1, sdd stiffening constraints that span this many particles
int nParticlesXY = 110;                         // Num particles in each dimension
f3vec grabPtWorld, grabPtWin;                   // The point being dragged around by a mouse click and drag
DrawMode drawMode = DRAW_TRIS;
ClothStyle clothStyle = TABLECLOTH;
CollisionObjects collisionObjects = COLLIDE_SPHERES;
Cloth* pCloth;
Timer FrameRateTimer;

// Given x,y,z window location compute 3D point clicked on; z should be like 0.9999
// Window y coord needs to be reversed for 0 at bottom before calling this.
f3vec unproject(f3vec winPt)
{
    double modelmat[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelmat);
    double projmat[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projmat);
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    d3vec worldPt;
    bool ok = gluUnProject(winPt.x, winPt.y, winPt.z, modelmat, projmat, viewport, &worldPt.x, &worldPt.y, &worldPt.z);
    ASSERT_R(ok);

    return f3vec(worldPt.x, worldPt.y, worldPt.z);
}

void userReshapeFunc0(int w, int h)
{
    WW = w;
    WH = h;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float Yfov = 45;             // VERTICAL FIELD OF VIEW IN DEGREES
    float Aspect = w / float(h); // WIDTH OVER HEIGHT
    float Near = 1.f;            // NEAR PLANE DISTANCE
    float Far = 500.0f;          // FAR PLANE DISTANCE

    gluPerspective(Yfov, Aspect, Near, Far);
}

// Display Function
void userDisplayFunc0()
{
    static int frameCount = 0;
    if (frameCount++ == 600) {
        double time = frameCount / FrameRateTimer.Reset();
        std::cerr << "Avg. frame rate: " << time << '\n';
        frameCount = 0;
    }

    // Set up view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    f3vec LookAtCntr(0, 0, 0);
    f3vec Eye(0, 30, 80), Up(0, 1, 0);
    gluLookAt(Eye.x, Eye.y, Eye.z, LookAtCntr.x, LookAtCntr.y, LookAtCntr.z, Up.x, Up.y, Up.z);

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
        if (state == GLUT_DOWN) {
            grabPtWin = f3vec(x, WH - y - 1, 1.f); // Invert y because GLUT uses window coords and the OpenGL viewport uses upside-down coords
            glReadPixels(grabPtWin.x, grabPtWin.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &grabPtWin.z); // Get depth value of clicked point
            if (grabPtWin.z < 0 || grabPtWin.z >= 1.f) return;

            grabPtWorld = unproject(grabPtWin);
            pCloth->GrabParticles(grabPtWorld);
        } else {
            pCloth->UngrabParticles();
        }
    if (button == GLUT_RIGHT_BUTTON)
        if (state == GLUT_DOWN)
            printf("RIGHT-DOWN\n");
        else
            printf("RIGHT-UP\n");
}

void userMotionFunc0(int x, int y)
{
    if (grabPtWin.z < 0 || grabPtWin.z >= 1.f) return;
    grabPtWin.x = x;
    grabPtWin.y = WH - y - 1;

    f3vec newGrabPtWorld = unproject(grabPtWin);
    f3vec delta = newGrabPtWorld - grabPtWorld;
    pCloth->MoveGrabbedParticles(delta);
    grabPtWorld = newGrabPtWorld;
}

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
    case '-':
        stiffening--;
        if (stiffening < 1) stiffening = nParticlesXY - 1;
        std::cerr << "stiffening: " << stiffening << '\n';
        pCloth->SetStiffening(stiffening, clothStyle);
        break;
    case '=':
        stiffening++;
        if (stiffening >= nParticlesXY) stiffening = 1;
        std::cerr << "stiffening: " << stiffening << '\n';
        pCloth->SetStiffening(stiffening, clothStyle);
        break;
    case '+':
        constraintIters++;
        std::cerr << "constraintIters: " << constraintIters << '\n';
        pCloth->SetConstraintIters(constraintIters);
        break;
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
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glutDisplayFunc(userDisplayFunc0);
    glutIdleFunc(userIdleFunc0);
    glutMouseFunc(userMouseFunc0);
    glutMotionFunc(userMotionFunc0);
    glutKeyboardFunc(userKeyboardFunc0);
    glutSpecialFunc(userSpecialKeyFunc0);
    glutReshapeFunc(userReshapeFunc0);

    // Create cloth with these initial parameters
    float clothWid = 60.f;
    f3vec startPos(0, clothWid / 2, 0);
    float dt = 0.03f;
    float damping = 0.95f;
    float partStep = clothWid / nParticlesXY;
    pCloth = new Cloth(nParticlesXY, nParticlesXY, partStep, partStep, startPos, dt, damping, clothStyle);
    pCloth->SetCollideObjectType(collisionObjects);
    pCloth->SetConstraintIters(constraintIters);

    GLfloat lightPos[] = {2.0, 30.0, 5.0, 1.0};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glutMainLoop();

    return 0;
}
