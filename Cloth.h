// Cloth.hpp
/*
  This cloth simulation is an implementation of the algorithm
   defined by Thomas Jakobsen of IO Interactive in the GDC course
   'Advanced Character Physics in 2001
*/

#pragma once

#include "ClothUtil.h"
#include "Constraint.h"

#include <vector>

#define NUM_ITERATIONS 3
#define KDAMPING .9
//#define CURTAIN

enum ClothStyle { TABLECLOTH, CURTAIN };

enum DrawMode { DRAW_POINTS = 1, DRAW_LINES = 2, DRAW_TRIS = 4 };

struct CollisionQuad {
    f3vec kV_[4];
    f3vec kNormal_;
};

class Cloth {
public:
    Cloth();
    Cloth(int nx, int ny,              // Number of grid points in x,y
          float dx, float dy,          // Spacing between grid points
          const f3vec& p,              // Cloth center
          double timestep, int style); // Timestep and style of cloth
    ~Cloth();
    void TimeStep(); // Update cloth
    void Reset();    // Move cloth back to original position
    void Display(int mode = DRAW_TRIS);
    void MoveSphere(const f3vec& dx); // Interaction with cloth by moving sphere
    void MoveQuads(const f3vec& dx);
    void SetCollideMode(bool isSphereMode); // true = spheremode, false = polymode
    void WriteTriModel(const char* filename);

private:
    void Verlet();
    void SatisfyConstraints();
    void AccumulateForces();

    void ReadTexture(const char*);

    int nX_;           // Grid points in x-dimension
    int nY_;           // Grid points in y-dimension
    int numParticles_; // Number of total particles
    int numTriangles_; // Number of triangles for rendering
    float restDX_;     // Resting length of particle-particle constraints
    float restDY_;
    float restDDiag_;

    f3vec startPos_;                       // Upper left hand corner of cloth
    f3vec* x_;                             // Current particle positions
    f3vec* oldx_;                          // Old positions
    f3vec* a_;                             // Force accumulators
    std::vector<Constraint*> constraints_; // Constraints - use stl vector for on-the-fly constraint creation
    int* triangles_;                       // Triangles for rendering
    float* colors_;                        // Vertex colors
    float* texCoords_;
    f3vec gravity_;   // Gravity
    double timeStep_; // Time step
    int clothStyle_;  // Type of cloth we are simulating

    unsigned char* texColors_; // Texels
    int texWidth_;
    int texHeight_;
    unsigned int texID_;

    bool isSphereMode_;
    void CreateSpheres(); // Collision sphere
    void CollisionWithSphere();
    int numSpheres_;
    float* sphereRadius_;
    f3vec* spherePos_;

    void CreateQuads();
    void CollisionWithQuad();
    int numQuads_;
    CollisionQuad* collisionQuads_;
};
