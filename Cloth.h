// Cloth.h - An implementation of the algorithm defined by Thomas Jakobsen of IO Interactive in the GDC course 'Advanced Character Physics in 2001

#pragma once

#include "Constraint.h"
#include "Math/AABB.h"

#include <vector>

enum ClothStyle { TABLECLOTH, CURTAIN, SLIDING_CURTAIN, PLEATED_CURTAIN, NUM_CLOTH_STYLES };
enum DrawMode { DRAW_POINTS, DRAW_LINES, DRAW_TRIS, NUM_DRAW_MODES };
enum CollisionObjects { COLLIDE_SPHERES, COLLIDE_BOXES, COLLIDE_INSIDE_BOXES, NUM_COLLISION_OBJECTS };

class Cloth {
public:
    Cloth();
    Cloth(int nx, int ny, float dx, float dy,               // Number of grid points in x,y, and Spacing between grid points
          const f3vec& clothCenter,                         // Cloth center
          float timestep, float damping, ClothStyle style); // Timestep, damping factor, and style of cloth
    ~Cloth();                                               // Destroy
    void TimeStep();                                        // Update cloth
    void Reset(ClothStyle clothStyle);                      // Move cloth to original position
    void Display(DrawMode mode);                            // Emit OpenGL commands
    void MoveColliders(const f3vec& delta);                 // Interact with cloth by moving collision objects
    void SetCollideObjectType(CollisionObjects collObj);    // What kind of objects to collide against
    void SetConstraintIters(int iters);                     // Set m_constraintItersPerTimeStep
    void SetStiffening(int stif, ClothStyle clothStyle);    // Set stiffening constraint span width
    void WriteTriModel(const char* filename);               // Write current cloth mesh to geometry file
    void GrabParticles(const f3vec& nPt);                   // Grab particles on projective mouse click line
    void UngrabParticles();                                 // Ungrab particles on mouse-up
    void MoveGrabbedParticles(const f3vec& delta);          // Interact with cloth by moving clicked-on particles

private:
    void VerletIntegration();
    void SatisfyConstraints();
    void AccumulateForces();
    void CreateSpheres();
    void CollisionWithSpheres();
    void CreateBoxes();
    void CollisionWithBoxes();

    void ReadTexture(const char*);

    // Simulation data
    int m_nx;                                          // Grid points in x-dimension
    int m_ny;                                          // Grid points in y-dimension
    float m_restDX, m_restDY, restDDiag;               // Resting length of particle-particle constraints
    f3vec m_initClothCenter;                           // Upper left hand corner of cloth
    std::vector<f3vec> m_pos;                          // Current particle positions
    std::vector<f3vec> m_oldPos;                       // Old positions
    std::vector<f3vec> m_forceAcc;                     // Force accumulators
    std::vector<Constraint*> m_constraints;            // Constraints
    std::vector<PointConstraint*> m_grabConstraints;   // Constraints for particles that were grabbed for moving around
    f3vec m_gravity = {0, -40, 0};                     // Gravity
    float m_damping;                                   // Damping constant to improve stability
    float m_timeStep;                                  // Time step
    int m_constraintItersPerTimeStep = 10;             // Iterating constraint satisfaction improves quality a lot
    int m_stiffening = 1;                              // Add stiffening constraints that span this many particles
    CollisionObjects m_collisionObj = COLLIDE_SPHERES; // What kind of objects to collide against
    std::vector<f4vec> m_collisionSpheres;             // List of spheres to collide against
    std::vector<Aabb> m_collisionBoxes;                // List of boxes to collide against

    // Rendering data
    int m_numTris;                  // Number of triangles for rendering
    std::vector<i3vec> m_triInds;   // Triangle indices for rendering and saving
    std::vector<f2vec> m_texCoords; // Texture coordinates per vertex for rendering
    std::vector<f3vec> m_normals;   // Normals per vertex for rendering
    float m_texRepeats = 3.f;       // Times the texture image repeats across the cloth
    unsigned int m_texID;           // OpenGL texture ID
};
