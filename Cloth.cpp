// Cloth.cpp

#include "Cloth.h"

#include "Image/tImage.h"
#include "Math/Random.h"

// OpenGL
#include "GL/glew.h"

// This needs to come after GLEW
#include "GL/freeglut.h"

Cloth::Cloth() { Cloth(40, 40, 1.0f, 1.0f, f3vec(0, 0, 0), .01f, 0.9f, TABLECLOTH); }

Cloth::Cloth(int nx, int ny, float dx, float dy, const f3vec& clothCenter_, float timestep, float damping, ClothStyle clothStyle) :
    m_nx(nx), m_ny(ny), m_restDX(dx), m_restDY(dy), m_initClothCenter(clothCenter_), m_timeStep(timestep), m_damping(damping)
{
    m_numParticles = nx * ny;
    m_numTris = 2 * (nx - 1) * (ny - 1);
    restDDiag = sqrt(dx * dx + dy * dy);

    // Create cloth node points and constraints
    m_pos = new f3vec[m_numParticles];
    m_oldPos = new f3vec[m_numParticles];
    m_forceAcc = new f3vec[m_numParticles];
    m_triInds = new int[m_numTris * 3];
    m_texCoords = new f2vec[m_numParticles];
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

    // CREATE CONSTRAINTS or cloth will fall apart
    for (int j = 0; j < m_ny - 1; j++) {
        for (int i = 0; i < m_nx - 1; i++) {
            // Index points
            // p1---p2
            //  |    |
            // p3---p4
            f3vec* p1 = &m_pos[i + m_nx * j];
            f3vec* p2 = &m_pos[i + 1 + m_nx * j];
            f3vec* p3 = &m_pos[i + m_nx * (j + 1)];
            f3vec* p4 = &m_pos[i + 1 + m_nx * (j + 1)];
            // Horizontal springs
            Constraint* hC = new RodConstraint(p1, p2, m_restDX);
            m_constraints.push_back(hC);

            // Diagonal springs
            Constraint* dC = new RodConstraint(p1, p4, restDDiag);
            // XXX: What about a p2, p3 diagonal?
            m_constraints.push_back(dC);

            // Vertical springs
            Constraint* vC = new RodConstraint(p1, p3, m_restDY);
            m_constraints.push_back(vC);
        }
    }
    // Last row
    for (int i = 0; i < m_nx - 1; i++) {
        f3vec* p1 = &m_pos[i + m_nx * (m_ny - 1)];
        f3vec* p2 = &m_pos[i + 1 + m_nx * (m_ny - 1)];
        Constraint* hC = new RodConstraint(p1, p2, m_restDX);
        m_constraints.push_back(hC);
    }
    // Last column
    for (int j = 0; j < m_ny - 1; j++) {
        f3vec* p1 = &m_pos[m_nx - 1 + j * m_nx];
        f3vec* p2 = &m_pos[m_nx - 1 + (j + 1) * m_nx];
        Constraint* vC = new RodConstraint(p1, p2, m_restDY);
        m_constraints.push_back(vC);
    }

    // CONSTRAINTS FOR CURTAIN-LIKE BEHAVIOR
    if (clothStyle == CURTAIN || clothStyle == SLIDING_CURTAIN) {
        f3vec* p1;
        Constraint* pC;

#if 1
        // Fix two corner particles to their initial positions
        p1 = &m_pos[0];
        pC = new PointConstraint(p1, *p1);
        m_constraints.push_back(pC);

        p1 = &m_pos[m_nx - 1];
        pC = new PointConstraint(p1, *p1);
        m_constraints.push_back(pC);
#endif

        // Constrain top of cloth to X axis
        for (int i = 1; i < m_nx; i++) {
            if (i % 4 == 0) {
                p1 = &m_pos[i];
                if (clothStyle == SLIDING_CURTAIN)
                    pC = new SlideConstraint(p1, *p1, (ConstrainAxis)(CY_AXIS | CZ_AXIS));
                else
                    pC = new PointConstraint(p1, *p1);
                m_constraints.push_back(pC);
            }
        }
    }

    // Shuffle constraints
    for (int i = m_constraints.size() - 1; i >= 0; i--) {
        int j = LRand(i + 1);
        std::swap(m_constraints[i], m_constraints[j]);
    }

    // Create triangles
    int index = 0;
    for (int j = 0; j < m_ny - 1; j++) {
        for (int i = 0; i < m_nx - 1; i++) {
            m_triInds[3 * index] = i + j * m_nx;
            m_triInds[3 * index + 1] = i + (j + 1) * m_nx;
            m_triInds[3 * index + 2] = i + 1 + (j + 1) * m_nx;
            index++;
            m_triInds[3 * index] = i + j * m_nx;
            m_triInds[3 * index + 1] = i + 1 + (j + 1) * m_nx;
            m_triInds[3 * index + 2] = i + 1 + j * m_nx;
            index++;
        }
    }
}

void Cloth::VerletIntegration()
{
    for (int i = 0; i < m_numParticles; i++) {
        f3vec& x = m_pos[i];
        f3vec temp = x;
        f3vec& oldx = m_oldPos[i];
        f3vec& a = m_forceAcc[i];

        // Verlet integration: x - oldx is an approximation of velocity.
        x += (x - oldx) * m_damping + a * m_timeStep * m_timeStep;
        oldx = temp;
    }
}

void Cloth::SatisfyConstraints()
{
    // Apply all the constraints several times per time step to try find a mutually satisfactory position for each particle
    // More iterations makes the simulation much more accurate, such as making the cloth pleat properly.
    for (int j = 0; j < m_constraintItersPerTimeStep; j++) {
        if (m_collisionObj == COLLIDE_SPHERES)
            CollisionWithSpheres();
        else if (m_collisionObj == COLLIDE_BOXES)
            CollisionWithBoxes();

        for (int i = 0; i < m_constraints.size(); i++) {
            Constraint* c = m_constraints[i];
            c->Apply();
        }
    }
}

void Cloth::AccumulateForces()
{
    // All particles are affected by gravity; could put other forces here, too
    for (int i = 0; i < m_numParticles; i++) { m_forceAcc[i] = m_gravity; }
}

void Cloth::CollisionWithSpheres()
{
    // Collide with sphere with center m_spherePos, radius m_sphereRadii
    for (int i = 0; i < m_numParticles; i++) {
        for (int j = 0; j < m_numSpheres; j++) {
            f3vec V = m_pos[i] - m_spherePos[j];
            float lengthV = V.length();

            // If the particle is inside the sphere push it to the nearest point outside the sphere
            if (lengthV < m_sphereRadii[j]) m_pos[i] = m_spherePos[j] + V * (m_sphereRadii[j] / lengthV);
        }
    }
}

void Cloth::CollisionWithBoxes()
{
    for (int i = 0; i < m_numParticles; i++) {
        for (size_t j = 0; j < m_collisionBoxes.size(); j++) {
            // If the particle is inside the box push it to the nearest point outside the box
            if (m_collisionBoxes[j].contains(m_pos[i])) m_pos[i] = m_collisionBoxes[j].nearestOnSurface(m_pos[i]);
        }
    }
}

void Cloth::SetCollideObjectType(CollisionObjects collObj) { m_collisionObj = collObj; }
void Cloth::SetConstraintIters(int iters) { m_constraintItersPerTimeStep = iters; }

void Cloth::CreateSpheres()
{
    m_numSpheres = 3;
    m_spherePos = new f3vec[m_numSpheres];
    m_sphereRadii = new float[m_numSpheres];
    m_spherePos[0] = f3vec(-7.5, 0, 0);
    m_sphereRadii[0] = 10.0;
    m_spherePos[1] = f3vec(7.5, 0, 0);
    m_sphereRadii[1] = 10.0;
    m_spherePos[2] = f3vec(7.5, 0, 15);
    m_sphereRadii[2] = 10.0;
}

void Cloth::CreateBoxes()
{
    float hxz = 15.f, hy = 10.0f;

    m_collisionBoxes.clear();
    Aabb box(f3vec(-hxz, -hy, -hxz), f3vec(hxz, hy, hxz));
    m_collisionBoxes.emplace_back(box);
}

void Cloth::MoveColliders(const f3vec& delta)
{
    if (m_collisionObj == COLLIDE_SPHERES)
        for (int i = 0; i < m_numSpheres; i++) { m_spherePos[i] += delta; }
    else if (m_collisionObj == COLLIDE_BOXES) {
        for (int i = 0; i < m_collisionBoxes.size(); i++) { m_collisionBoxes[i] = m_collisionBoxes[i] + delta; }
    }
}

// Add up forces, advance system, satisfy constraints
void Cloth::TimeStep()
{
    AccumulateForces();
    VerletIntegration();
    SatisfyConstraints();
}

void Cloth::Display(DrawMode drawMode)
{
    glEnable(GL_DEPTH_TEST);

    if (drawMode == DRAW_POINTS) {
        glPointSize(3.0);
        glColor3f(0, 1, 1);
        glBegin(GL_POINTS);
        for (int i = 0; i < m_numParticles; i++) glVertex3fv(m_pos[i].getPtr());
        glEnd();
    } else if (drawMode == DRAW_LINES) {
        glColor3f(1, 1, 0);
        std::cerr << "Line mode not implemented\n";
    } else if (drawMode == DRAW_TRIS) {
        glColor3f(1, 1, 1);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, m_texID);

        for (int i = 0; i < m_numTris; i++) {
            f3vec &a = m_pos[m_triInds[3 * i]], &b = m_pos[m_triInds[3 * i + 1]], &c = m_pos[m_triInds[3 * i + 2]];
            f3vec n1 = Cross((c - b), (a - b));
            n1.normalize();
            glBegin(GL_TRIANGLES);
            glNormal3fv(n1.getPtr());
            glTexCoord2fv(m_texCoords[m_triInds[3 * i]].getPtr());
            glVertex3fv(a.getPtr());
            glTexCoord2fv(m_texCoords[m_triInds[3 * i + 1]].getPtr());
            glVertex3fv(b.getPtr());
            glTexCoord2fv(m_texCoords[m_triInds[3 * i + 2]].getPtr());
            glVertex3fv(c.getPtr());
            glEnd();
        }

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
    }
    GL_ASSERT();

    // Draw collision objects
    glColor3f(1, 0, 1);
    if (m_collisionObj == COLLIDE_SPHERES) {
        // Draw spheres
        for (int i = 0; i < m_numSpheres; i++) {
            glPushMatrix();
            glTranslatef(m_spherePos[i].x, m_spherePos[i].y, m_spherePos[i].z);
            glutWireSphere(.99 * m_sphereRadii[i], 20, 20);
            glPopMatrix();
        }
    } else if (m_collisionObj == COLLIDE_BOXES) {
        // Draw boxes
        for (size_t i = 0; i < m_collisionBoxes.size(); i++) {
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

    unsigned int HexColor;

    FILE* fp = fopen(FileName, "w");
    if (fp == NULL) {
        printf("ERROR: unable to open TriObj [%s]!\n", FileName);
        exit(1);
    }

    fprintf(fp, "%d\n", m_numTris);
    for (int i = 0; i < m_numTris; i++) {
        f3vec &a = m_pos[m_triInds[3 * i]], &b = m_pos[m_triInds[3 * i + 1]], &c = m_pos[m_triInds[3 * i + 2]];
        fprintf(fp, "%f %f %f ", a.x, a.y, a.z);
        fprintf(fp, "%f %f %f ", b.x, b.y, b.z);
        fprintf(fp, "%f %f %f ", c.x, c.y, c.z);

        HexColor = (((unsigned int)255) << 16) + (((unsigned int)255) << 8) + ((unsigned int)255);
        fprintf(fp, "%x\n", HexColor);
    }
    fclose(fp);
}
