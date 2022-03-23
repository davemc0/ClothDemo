// Cloth.h - An implementation of the algorithm defined by Thomas Jakobsen of IO Interactive in the GDC course 'Advanced Character Physics in 2001

#pragma once

#include "Constraint.h"

#include <vector>

#define NUM_ITERATIONS 3

enum ClothStyle { TABLECLOTH, CURTAIN, NUM_CLOTH_STYLES };

enum DrawMode { DRAW_POINTS, DRAW_LINES, DRAW_TRIS, NUM_DRAW_MODES };

struct CollisionQuad {
    f3vec kV_[4];
    f3vec kNormal_;
};

class Cloth {
public:
    Cloth();
    Cloth(int nx, int ny,                                    // Number of grid points in x,y
          float dx, float dy,                                // Spacing between grid points
          const f3vec& clothCenter,                          // Cloth center
          double timestep, float damping, ClothStyle style); // Timestep and style of cloth
    ~Cloth();
    void TimeStep();                        // Update cloth
    void Reset(ClothStyle clothStyle);      // Move cloth to original position
    void Display(DrawMode mode);            // Emit OpenGL commands
    void MoveSpheres(const f3vec& dx);      // Interact with cloth by moving spheres
    void MoveQuads(const f3vec& dx);        // Interact with cloth by moving quads
    void SetCollideMode(bool isSphereMode); // true = spheremode, false = polymode
    void WriteTriModel(const char* filename);

private:
    void Verlet();
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
    double m_timeStep;                      // Time step

    // Rendering data
    int m_numTris;            // Number of triangles for rendering
    int* m_triInds;           // Triangle indices for rendering
    f2vec* m_texCoords;       // Texture coordinates
    float m_texRepeats = 3.f; // Times the texture image repeats across the cloth
    unsigned int m_texID;     // OpenGL texture ID

    bool m_collisionModeSpheres;
    void CreateSpheres();
    void CollisionWithSpheres();
    int m_numSpheres;
    float* m_sphereRadii;
    f3vec* m_spherePos;

    void CreateQuads();
    void CollisionWithQuads();
    int m_numQuads;
    CollisionQuad* m_collisionQuads;
};
