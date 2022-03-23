// Cloth.hpp
/* 
  This cloth simulation is an implementation of the algorithm
   defined by Thomas Jakobsen of IO Interactive in the GDC course
   'Advanced Character Physics in 2001 
*/

#ifndef __CLOTH_H__
#define __CLOTH_H__

#include "ClothUtil.hpp"
#include "Constraint.hpp"
#include <vec3f.hpp>
#include <GL/glut.h>
#include <ppm.hpp>
#include <vector>
using namespace std;

#define NUM_ITERATIONS 3
#define KDAMPING .9
//#define CURTAIN


enum ClothStyle {
  TABLECLOTH,
  CURTAIN
};

enum DrawMode {
  DRAW_POINTS = 1,
  DRAW_LINES = 2,
  DRAW_TRIS = 4
};

struct CollisionQuad
{
  Vec3f kV_[4];
  Vec3f kNormal_;
};

class Cloth 
{
public:
  Cloth();
  Cloth::Cloth(int nx, int ny,                // number of grid points in x,y
               float dx, float dy,            // spacing between grid points
               const Vec3f& p,                // cloth center
               double timestep, int style);   // timestep and style of cloth
  ~Cloth();
  void TimeStep();                 // Update cloth
  void Reset();                    // Move cloth back to original position
  void Display(int mode=DRAW_TRIS);
  void MoveSphere(const Vec3f& dx);// Interaction with cloth by moving sphere
  void MoveQuads(const Vec3f& dx);
  void SetCollideMode(bool isSphereMode);  // true = spheremode, false = polymode
  void WriteTriModel(const char * filename);

private:

  void Verlet();
  void SatisfyConstraints();
  void AccumulateForces();
    
  void ReadTexture(char *);


  int nX_;                         // Grid points in x-dimension
  int nY_;                         // Grid points in y-dimension
  int numParticles_;               // Number of total particles
  int numTriangles_;               // Number of triangles for rendering
  float restDX_;                   // Resting length of particle-particle constraints
  float restDY_;
  float restDDiag_;

  Vec3f startPos_;                 // Upper left hand corner of cloth
  Vec3f *x_;                       // Current particle positions
  Vec3f *oldx_;                    // Old positions
  Vec3f *a_;                       // Force accumulators
  vector<Constraint *> constraints_; // Constraints - use stl vector for on the fly constraint creation
  int *triangles_;                // triangles for rendering
  float *colors_;                 // vertex colors
  float *texCoords_;
  Vec3f gravity_;                  // Gravity
  double timeStep_;                // Time step
  int clothStyle_;                 // Type of cloth we are simulating

  unsigned char * texColors_;      // Texels
  int texWidth_;
  int texHeight_;     
  GLuint texID_;

  bool isSphereMode_;
  void CreateSpheres();             // Collision sphere
  void CollisionWithSphere(); 
  int numSpheres_;
  float *sphereRadius_;
  Vec3f *spherePos_;

  void CreateQuads();
  void CollisionWithQuad();
  int numQuads_;
  CollisionQuad *collisionQuads_;
};


#endif