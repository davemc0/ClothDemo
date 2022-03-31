// Cloth.cpp

#include "Cloth.h"

#include "Image/tImage.h"
#include "Math/Random.h"

// OpenGL
#include "GL/glew.h"

// This needs to come after GLEW
#include "GL/freeglut.h"

#include <execution>

Cloth::Cloth() { Cloth(40, 40, 1.0f, 1.0f, f3vec(0, 0, 0), .01f, 0.9f, TABLECLOTH); }

Cloth::Cloth(int nx, int ny, float dx, float dy, const f3vec& clothCenter_, float timestep, float damping, ClothStyle clothStyle) :
    m_nx(nx), m_ny(ny), m_restDX(dx), m_restDY(dy), m_initClothCenter(clothCenter_), m_timeStep(timestep), m_damping(damping)
{
    int numParticles = nx * ny;
    m_numTris = 2 * (nx - 1) * (ny - 1);
    restDDiag = sqrt(dx * dx + dy * dy);

    // Create cloth node points and constraints
    m_pos.resize(numParticles);
    m_oldPos.resize(numParticles);
    m_forceAcc.resize(numParticles);
    m_triInds.resize(m_numTris);
    m_texCoords.resize(numParticles);
    m_normals.resize(numParticles);
    Reset(clothStyle);

    // Create colliders
    CreateSpheres();
    CreateBoxes();

    // Read texture
    ReadTexture("PatternCloth.jpg");
}

Cloth::~Cloth() {}

void Cloth::Reset(ClothStyle clothStyle)
{
    m_constraints.clear();

    // Find width and height of cloth
    float width = m_nx * m_restDX;
    float height = m_ny * m_restDY;
    f3vec clothGridCorner(-width / 2.f, 0, -height / 2.f);

    // Create grid of particles
    for (int j = 0; j < m_ny; j++) {
        for (int i = 0; i < m_nx; i++) {
            int index = i + m_nx * j;
            m_pos[index] = m_oldPos[index] = m_initClothCenter + clothGridCorner + f3vec(m_restDX * i, 0, m_restDY * j);
            m_texCoords[index] = f2vec((float)i / m_nx, (float)j / m_ny) * m_texRepeats;
        }
    }

    // Constraints to hold the cloth together
    for (int j = 0; j < m_ny; j++) {
        for (int i = 0; i < m_nx; i++) {
            f3vec* p1 = m_pos.data() + i + m_nx * j;           // Index point
            f3vec* p2 = m_pos.data() + i + 1 + m_nx * j;       // P1---p2
            f3vec* p3 = m_pos.data() + i + m_nx * (j + 1);     //  |    |
            f3vec* p4 = m_pos.data() + i + 1 + m_nx * (j + 1); // P3---p4

            if (i < m_nx - 1) m_constraints.push_back(new RodConstraint(p1, p2, m_restDX));                  // Horizontal springs
            if (j < m_ny - 1) m_constraints.push_back(new RodConstraint(p1, p3, m_restDY));                  // Vertical springs
            if (i < m_nx - 1 && j < m_ny - 1) m_constraints.push_back(new RodConstraint(p1, p4, restDDiag)); // Diagonal springs are faster with
            if (i < m_nx - 1 && j < m_ny - 1) m_constraints.push_back(new RodConstraint(p2, p3, restDDiag)); // Only one but it sags to the left
        }
    }

    // Stiffening constraints
    const int ST = m_stiffening;
    if (ST > 1)
        for (int j = 0; j < m_ny; j++) {
            for (int i = 0; i < m_nx; i++) {
                f3vec* p1 = m_pos.data() + i + m_nx * j;             // Index point
                f3vec* p2 = m_pos.data() + i + ST + m_nx * j;        // P1---p2
                f3vec* p3 = m_pos.data() + i + m_nx * (j + ST);      //  |    |
                f3vec* p4 = m_pos.data() + i + ST + m_nx * (j + ST); // P3---p4

                if (i < m_nx - ST) m_constraints.push_back(new RodConstraint(p1, p2, m_restDX * ST));                   // Horizontal springs
                if (j < m_ny - ST) m_constraints.push_back(new RodConstraint(p1, p3, m_restDY * ST));                   // Vertical springs
                if (i < m_nx - ST && j < m_ny - ST) m_constraints.push_back(new RodConstraint(p1, p4, restDDiag * ST)); // Diagonal springs are faster with
                if (i < m_nx - ST && j < m_ny - ST) m_constraints.push_back(new RodConstraint(p2, p3, restDDiag * ST)); // Only one but it sags to the left
            }
        }

    // Constraints for curtain-like behavior
    if (clothStyle == CURTAIN) {
        for (int i = 0; i < m_nx; i += 4) {
            f3vec* p1 = &m_pos[i];
            m_constraints.push_back(new PointConstraint(p1, *p1)); // Constrain top of cloth to X axis
        }
    } else if (clothStyle == SLIDING_CURTAIN) {
        for (int i = 0; i < m_nx; i += 4) {
            if (i == 0)
                m_constraints.push_back(new PointConstraint(&m_pos[i], m_pos[i])); // Fix top-left corner particle to initial position
            else
                m_constraints.push_back(new SlideConstraint(&m_pos[i], m_pos[i], (ConstrainAxis)(CY_AXIS | CZ_AXIS))); // Let top particles slide in X
        }
    } else if (clothStyle == PLEATED_CURTAIN) {
        for (int i = 0; i < m_nx; i += 10) {
            f3vec tgt = m_pos[i];
            tgt.x *= 0.7f;                                                // Shrink X coords to cause pleating
            m_constraints.push_back(new PointConstraint(&m_pos[i], tgt)); // Constrain top of cloth to X axis
        }
    }

    // Shuffle constraints by swapping each one with another random one
    for (int i = 0; i < m_constraints.size(); i++) std::swap(m_constraints[i], m_constraints[irand((int)m_constraints.size())]);

    // Create triangle indices for rendering
    int index = 0;
    for (int j = 0; j < m_ny - 1; j++) {
        for (int i = 0; i < m_nx - 1; i++) {
            m_triInds[index++] = {i + j * m_nx, i + (j + 1) * m_nx, i + 1 + (j + 1) * m_nx};
            m_triInds[index++] = {i + j * m_nx, i + 1 + (j + 1) * m_nx, i + 1 + j * m_nx};
        }
    }
}

void Cloth::VerletIntegration()
{
    // Parallelizing this didn't really help. Remove par_unseq if not C++17.
    std::for_each(std::execution::par_unseq, m_pos.begin(), m_pos.end(), [&](f3vec const& p) {
        int i = &p - &m_pos[0];
        f3vec& x = m_pos[i];
        f3vec temp = x;
        f3vec& oldx = m_oldPos[i];
        f3vec& a = m_forceAcc[i];

        // Verlet integration: x - oldx is an approximation of velocity.
        x += (x - oldx) * m_damping + a * m_timeStep * m_timeStep;
        oldx = temp;
    });
}

void Cloth::SatisfyConstraints()
{
    // Apply all the constraints several times per time step to try to find a mutually satisfactory position for each particle
    // More iterations makes the simulation much more accurate, such as making the cloth pleat properly.
    for (int j = 0; j < m_constraintItersPerTimeStep; j++) {
        if (m_collisionObj == COLLIDE_SPHERES)
            CollisionWithSpheres();
        else if (m_collisionObj == COLLIDE_BOXES || m_collisionObj == COLLIDE_INSIDE_BOXES)
            CollisionWithBoxes();

        // This parallelization has a race condition for Rod constraints, since multiple threads could touch the same particle at the same time,
        // but in practice it just doesn't matter.
        std::for_each(std::execution::par_unseq, m_constraints.begin(), m_constraints.end(), [&](Constraint* const& cc) { cc->Apply(); });

        std::for_each(std::execution::par_unseq, m_grabConstraints.begin(), m_grabConstraints.end(), [&](Constraint* const& cc) { cc->Apply(); });
    }
}

void Cloth::AccumulateForces()
{
    // All particles are affected by gravity; could put other forces here, too
    for (int i = 0; i < m_pos.size(); i++) { m_forceAcc[i] = m_gravity; }
}

void Cloth::CollisionWithSpheres()
{
    // Collide each particle with each sphere
    for (int i = 0; i < m_pos.size(); i++) {
        for (size_t j = 0; j < m_collisionSpheres.size(); j++) {
            f3vec spherePos(m_collisionSpheres[j]);
            float sphereRad = m_collisionSpheres[j].w;
            f3vec V = m_pos[i] - spherePos;
            float lengthV = V.length();

            // If the particle is inside the sphere push it to the nearest point outside the sphere
            if (lengthV < sphereRad) m_pos[i] = spherePos + V * (sphereRad / lengthV);
        }
    }
}

void Cloth::CollisionWithBoxes()
{
    bool checkInside = m_collisionObj == COLLIDE_INSIDE_BOXES;
    // Collide each particle with each AABB and make sure the particle is on the proper side of the box
    size_t st = 1, end = m_collisionBoxes.size();
    if (checkInside) {
        st = 0;
        end = 1;
    }

    for (size_t j = st; j < end; j++) {
        for (int i = 0; i < m_pos.size(); i++) {
            // If forcing outside and the particle is inside the box push it to the nearest point on the box surface
            if (m_collisionBoxes[j].contains(m_pos[i]) && !checkInside) m_pos[i] = m_collisionBoxes[j].nearestOnSurface(m_pos[i]);
            // If forcing inside and the particle is outside the box push it to the nearest point on the box surface
            if (!m_collisionBoxes[j].contains(m_pos[i]) && checkInside) m_pos[i] = m_collisionBoxes[j].nearest(m_pos[i]);
        }
    }
}

void Cloth::SetCollideObjectType(CollisionObjects collObj) { m_collisionObj = collObj; }
void Cloth::SetConstraintIters(int iters) { m_constraintItersPerTimeStep = iters; }
void Cloth::SetStiffening(int stif, ClothStyle clothStyle)
{
    m_stiffening = stif;
    Reset(clothStyle);
}

void Cloth::CreateSpheres()
{
    m_collisionSpheres.resize(3);
    m_collisionSpheres[0] = f4vec(-5, 0, -4, 10.f);
    m_collisionSpheres[1] = f4vec(5, 0, -4, 10.f);
    m_collisionSpheres[2] = f4vec(0, 0, 5, 10.f);
}

void Cloth::CreateBoxes()
{
    m_collisionBoxes.clear();

    // Create Inside Boxes
    float hxz = 35.f, hy = 30.0f;
    Aabb box = {f3vec(-hxz, -25, -hxz), f3vec(hxz, hy, hxz)};
    m_collisionBoxes.emplace_back(box);

    // Create Outside Boxes
    hxz = 15.f, hy = 10.0f;
    box = {f3vec(-hxz, -hy, -hxz), f3vec(hxz, hy, hxz)};
    m_collisionBoxes.emplace_back(box);
}

void Cloth::MoveColliders(const f3vec& delta)
{
    if (m_collisionObj == COLLIDE_SPHERES)
        for (int i = 0; i < m_collisionSpheres.size(); i++) { m_collisionSpheres[i] = f4vec(f3vec(m_collisionSpheres[i]) + delta, m_collisionSpheres[i].w); }
    else if (m_collisionObj == COLLIDE_BOXES) {
        for (int i = 1; i < m_collisionBoxes.size(); i++) { m_collisionBoxes[i] = m_collisionBoxes[i] + delta; }
    } else if (m_collisionObj == COLLIDE_INSIDE_BOXES) {
        for (int i = 0; i < 1; i++) { m_collisionBoxes[i] = m_collisionBoxes[i] + delta; }
    }
}

void Cloth::GrabParticles(const f3vec& pt)
{
    for (auto p : m_grabConstraints) delete p;
    m_grabConstraints.clear();

    for (size_t i = 0; i < m_pos.size(); i++) {
        f3vec& p = m_pos[i];
        if ((p - pt).length() < restDDiag) m_grabConstraints.emplace_back(new PointConstraint(&p, p));
    }
}

void Cloth::UngrabParticles()
{
    for (auto p : m_grabConstraints) delete p;
    m_grabConstraints.clear();
}

// Add up forces, advance system, satisfy constraints
void Cloth::TimeStep()
{
    // Cumulative times with no par_unseq: AccumulateForces: 0.0456733 VerletIntegration: 0.193743 SatisfyConstraints: 11.9977
    AccumulateForces();
    VerletIntegration();
    SatisfyConstraints();
}

void Cloth::MoveGrabbedParticles(const f3vec& delta)
{
    for (auto& c : m_grabConstraints) { c->setPos(c->getPos() + delta); }
}

void Cloth::Display(DrawMode drawMode)
{
    if (drawMode == DRAW_POINTS) {
        glPointSize(3.0);
        glColor3f(0, 1, 1);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, m_pos.data());
        glDrawArrays(GL_POINTS, 0, (GLsizei)m_pos.size());
        glDisableClientState(GL_VERTEX_ARRAY);
    } else if (drawMode == DRAW_LINES) {
        glLineWidth(2.5f);
        glColor3f(1, 1, 1);
        glEnableClientState(GL_VERTEX_ARRAY);
        for (int i = 0; i < m_nx - 1; i++) {
            glVertexPointer(3, GL_FLOAT, m_ny * sizeof(f3vec), &m_pos[i]);
            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)m_ny);
        }
        glDisableClientState(GL_VERTEX_ARRAY);
    } else if (drawMode == DRAW_TRIS) {
        glColor3f(1, 1, 1);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, m_texID);

        for (int i = 0; i < m_numTris; i++) {
            f3vec &a = m_pos[m_triInds[i][0]], &b = m_pos[m_triInds[i][1]], &c = m_pos[m_triInds[i][2]];
            f3vec n1 = cross((c - b), (a - b));
            n1.normalize();
            m_normals[m_triInds[i][0]] = n1; // Computing facet normals but using them as vertex normals because glDrawElements requires vertex normals
            m_normals[m_triInds[i][1]] = n1; // Most normals get written to three times, but no big deal.
            m_normals[m_triInds[i][2]] = n1; // Could average them to get higher quality normals but not worth it.
        }

        glTexCoordPointer(2, GL_FLOAT, 0, m_texCoords.data());
        glNormalPointer(GL_FLOAT, 0, m_normals.data());
        glVertexPointer(3, GL_FLOAT, 0, m_pos.data());

        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);

        glDrawElements(GL_TRIANGLES, (GLsizei)m_triInds.size() * 3, GL_UNSIGNED_INT, m_triInds.data());

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
    }
    GL_ASSERT();

    // Draw collision objects
    glLineWidth(1.5f);
    glColor3f(1, 0, 1);
    if (m_collisionObj == COLLIDE_SPHERES) {
        // Draw spheres
        for (int i = 0; i < m_collisionSpheres.size(); i++) {
            glPushMatrix();
            glTranslatef(m_collisionSpheres[i].x, m_collisionSpheres[i].y, m_collisionSpheres[i].z);
            glutWireSphere(.99 * m_collisionSpheres[i].w, 20, 20);
            glPopMatrix();
        }
    } else if (m_collisionObj == COLLIDE_BOXES || m_collisionObj == COLLIDE_INSIDE_BOXES) {
        // Draw boxes
        size_t st = 1, end = m_collisionBoxes.size();
        if (m_collisionObj == COLLIDE_INSIDE_BOXES) {
            st = 0;
            end = 1;
        }

        for (size_t i = st; i < end; i++) {
            Aabb box = m_collisionBoxes[i];
            glPushMatrix();
            glTranslatef(box.centroid().x, box.centroid().y, box.centroid().z);
            glScalef(box.extent().x, box.extent().y, box.extent().z);
            glutWireCube(1);
            glPopMatrix();
        }
    }
    GL_ASSERT();
}

void Cloth::ReadTexture(const char* texName)
{
    uc3Image texIm(texName);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &m_texID);
    glBindTexture(GL_TEXTURE_2D, m_texID);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 128.0f);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, texIm.w(), texIm.h(), GL_RGB, GL_UNSIGNED_BYTE, texIm.pp());

    GL_ASSERT();
}

void Cloth::WriteTriModel(const char* FileName)
{
    printf("Writing to %s (%d triangles). . .\n", FileName, m_numTris);

    FILE* fp = fopen(FileName, "w");
    if (fp == NULL) {
        printf("ERROR: unable to open TriObj [%s]!\n", FileName);
        exit(1);
    }

    fprintf(fp, "%d\n", m_numTris);
    for (int i = 0; i < m_numTris; i++) {
        f3vec &a = m_pos[m_triInds[i][0]], &b = m_pos[m_triInds[i][1]], &c = m_pos[m_triInds[i][2]];
        fprintf(fp, "%f %f %f ", a.x, a.y, a.z);
        fprintf(fp, "%f %f %f ", b.x, b.y, b.z);
        fprintf(fp, "%f %f %f ", c.x, c.y, c.z);

        fprintf(fp, "ffffff\n");
    }
    fclose(fp);
}
