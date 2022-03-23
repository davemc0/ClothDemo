// ---------------------------------------------------
// Cloth demo
// ---------------------------------------------------

#include <glvu.hpp>
#include <GL/glut.h>
#include <vec3f.hpp>
#include "Cloth.hpp"

// Globals
bool paused = true;
bool wireMode = false;
bool sphereMode = false;
GLVU glvu;
int WW = 512, WH = 512;
Cloth * pCloth;
int CallingDefaultMouseFuncsOn=1;

// Display Function
void userDisplayFunc0()
{
  //glEnable(GL_LIGHTING);
  //glEnable(GL_LIGHT0);
  //glShadeModel(GL_SMOOTH);
  glvu.BeginFrame();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pCloth->Display();
    glvu.DrawFPS();
  glvu.EndFrame();
}

// Idle loop
void userIdleFunc0()
{
  // Update cloth
  if (!paused) 
  {
    pCloth->TimeStep();
  }
  glutPostRedisplay();
}

//----------------------------------------------------------------------------
// USER-PROVIDED MOUSE HANDLING ROUTINE
//----------------------------------------------------------------------------
void userMouseFunc0(int button, int state, int x, int y)
{
  int Modifiers = glutGetModifiers();
  if (Modifiers & GLUT_ACTIVE_CTRL)
    printf("CTRL-");
  if (Modifiers & GLUT_ACTIVE_SHIFT) 
    printf("SHIFT-");
  if (Modifiers & GLUT_ACTIVE_ALT) 
    printf("ALT-");

  if (button==GLUT_LEFT_BUTTON)
    if (state==GLUT_DOWN) printf("LEFT-DOWN\n"); else printf("LEFT-UP\n");
  if (button==GLUT_RIGHT_BUTTON)
    if (state==GLUT_DOWN) printf("RIGHT-DOWN\n"); else printf("RIGHT-UP\n");

  // OPTIONAL: CALL THE DEFAULT GLVU MOUSE HANDLER
  if (CallingDefaultMouseFuncsOn) glvu.Mouse(button,state,x,y);
}

void userMotionFunc0(int x, int y)
{
  printf("%d %d\n", x,y);

  // OPTIONAL: CALL THE DEFAULT GLVU MOTION HANDLER
  if (CallingDefaultMouseFuncsOn) glvu.Motion(x,y);
}

//----------------------------------------------------------------------------
// USER-PROVIDED KEYBOARD HANDLING ROUTINE
//----------------------------------------------------------------------------
void userKeyboardFunc0(unsigned char Key, int x, int y)
{
  switch(Key)
  {
    case ' ':
      paused = !paused;
      break;
    case 'r':
      if (pCloth) pCloth->Reset();
      break;
    case 'w':
      if (wireMode) {
        glPolygonMode(GL_FRONT, GL_FILL);
        glPolygonMode(GL_BACK, GL_FILL);
        wireMode = false;
      }
      else {
        glPolygonMode(GL_FRONT, GL_LINE);
        glPolygonMode(GL_BACK, GL_LINE);
        wireMode = true;
      }
      break;
    case 's':
      pCloth->WriteTriModel("tablecloth.tri");
      break;
    case 'm':
      sphereMode = !sphereMode;
      pCloth->SetCollideMode(sphereMode);
      break;
  };

  // OPTIONAL: CALL THE DEFAULT GLVU KEYBOARD HANDLER
  glvu.Keyboard(Key,x,y);
}

void userSpecialKeyFunc0(int Key, int x, int y)
{
  float dx = 2;
  int mod = glutGetModifiers();

  switch(Key)
  {
  case GLUT_KEY_LEFT:
    pCloth->MoveSphere(Vec3f(-dx,0,0));
    break;
  case GLUT_KEY_RIGHT:
    pCloth->MoveSphere(Vec3f(dx,0,0));
    break;
  case GLUT_KEY_UP:
    if (mod == GLUT_ACTIVE_CTRL)
      pCloth->MoveSphere(Vec3f(0,dx,0));
    else
      pCloth->MoveSphere(Vec3f(0,0,dx));
    break;
  case GLUT_KEY_DOWN:
    if (mod == GLUT_ACTIVE_CTRL)
      pCloth->MoveSphere(Vec3f(0,-dx,0));
    else
      pCloth->MoveSphere(Vec3f(0,0,-dx));
    break;
  }
}

int main()
{
  //--------------------------------------------------------------------------
  // TO USE THE VIEWER:
  // (1) Instantiate some global GLVUs (one for each window). See top of file.
  //--------------------------------------------------------------------------

  //--------------------------------------------------------------------------
  // (2) Init each viewer by specifying menu string, visual mode mask, 
  //     and starting position and extents of the window.
  //     After Init, perform any OpenGL initializations and
  //     initialize the viewer cameras.
  //--------------------------------------------------------------------------

  glvu.Init("GLVU Basic Example",
            GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA ,
            50,50,WW,WH);
  glvu.StartFPSClock();
  //glEnable(GL_LIGHTING);
  //glEnable(GL_LIGHT0);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glutDisplayFunc(userDisplayFunc0);
  glutIdleFunc(userIdleFunc0);
  glutMouseFunc(userMouseFunc0);
  glutMotionFunc(userMotionFunc0);
  glutKeyboardFunc(userKeyboardFunc0);
  glutSpecialFunc(userSpecialKeyFunc0);

  // Set up view
  Vec3f m(-20, -20, -20), M(20, 20, 20), LookAtCntr(0, 0, 0);
  Vec3f Eye(0,30,80), Up(0,1,0);
  float Yfov = 45;
  float Aspect = 1;   // WIDTH OVER HEIGHT
  float Near = 0.1f;  // NEAR PLANE DISTANCE RELATIVE TO MODEL DIAGONAL LENGTH
  float Far = 100.0f;  // FAR PLANE DISTANCE (ALSO RELATIVE)
  glvu.SetAllCams(m,M, Eye,LookAtCntr,Up, Yfov,Aspect, Near,Far);

  // Create cloth with starting point = P, timestep DT
  double dt = .03;
  Vec3f P(0,10,0);
//  pCloth = new Cloth(190, 190, 0.5f, 0.5f, P,dt,TABLECLOTH);
  pCloth = new Cloth(90, 90, 0.5f, 0.5f, P,dt,TABLECLOTH);

  GLfloat lightPos[] = {2.0, 0.0, 5.0, 0.0};
 
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  //--------------------------------------------------------------------------
  // (3) start the viewer event loop.
  //--------------------------------------------------------------------------
  glutMainLoop();

  return 0;
}
