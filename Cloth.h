// Cloth.h - An implementation of the algorithm defined by Thomas Jakobsen of IO Interactive in the GDC course 'Advanced Character Physics in 2001

#pragma once

#include "Constraint.h"
#include "Math/AABB.h"

#include <vector>

enum ClothStyle { TABLECLOTH, CURTAIN, SLIDING_CURTAIN, NUM_CLOTH_STYLES };
enum DrawMode { DRAW_POINTS, DRAW_LINES, DRAW_TRIS, NUM_DRAW_MODES };
enum CollisionObjects { COLLIDE_SPHERES, COLLIDE_BOXES, NUM_COLLISION_OBJECTS };

class Cloth {
public:
    Cloth();
    Cloth(int nx, int ny,                                   // Number of grid points in x,y
          float dx, float dy,                               // Spacing between grid points
          const f3vec& clothCenter,                         // Cloth center
          float timestep, float damping, ClothStyle style); // Timestep, damping factor, and style of cloth
    ~Cloth();                                               // Destroy
    void TimeStep();                                        // Update cloth
    void Reset(ClothStyle clothStyle);                      // Move cloth to original position
    void Display(DrawMode mode);                            // Emit OpenGL commands
    void MoveColliders(const f3vec& delta);                 // Interact with cloth by moving collision objects
    void SetCollideObjectType(CollisionObjects collObj);    // What kind of objects to collide against
    void SetConstraintIters(int iters);                     // Set m_constraintItersPerTimeStep
    void WriteTriModel(const char* filename);               // Write current cloth mesh to geometry file

private:
    void VerletIntegration();
    void SatisfyConstraints();
    void AccumulateForces();

    void ReadTexture(const char*);

    // Simulation data
    int m_nx;                               // Grid points in x-dimension
    int m_ny;                               // Grid points in y-dimension
    int m_numParticles;                     // Number of total particles
    float m_restDX, m_restDY, restDDiag;    // Resting length of particle-particle constraints
    f3vec m_initClothCenter;                // Upper left hand corner of cloth
    f3vec* m_pos;                           // Current particle positions
    f3vec* m_oldPos;                        // Old positions
    f3vec* m_forceAcc;                      // Force accumulators
    std::vector<Constraint*> m_constraints; // Constraints
    f3vec m_gravity = {0, -40, 0};          // Gravity
    float m_damping;                        // Damping constant to improve stability
    float m_timeStep;                       // Time step
    int m_constraintItersPerTimeStep = 10;  // Iterating constraint satisfaction improves quality a lot

    // Rendering data
    int m_numTris;            // Number of triangles for rendering
    int* m_triInds;           // Triangle indices for rendering
    f2vec* m_texCoords;       // Texture coordinates
    float m_texRepeats = 3.f; // Times the texture image repeats across the cloth
    unsigned int m_texID;     // OpenGL texture ID

    CollisionObjects m_collisionObj = COLLIDE_SPHERES;
    void CreateSpheres();
    void CollisionWithSpheres();
    int m_numSpheres;
    float* m_sphereRadii;
    f3vec* m_spherePos;

    void CreateBoxes();
    void CollisionWithBoxes();
    std::vector<Aabb> m_collisionBoxes;
};
