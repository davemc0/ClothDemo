// Cloth.cpp
#include "Cloth.h"

#include "Image/tImage.h"
#include "Math/AABB.h"
#include "Math/Random.h"

// OpenGL
#include "GL/glew.h"

// This needs to come after GLEW
#include "GL/freeglut.h"

#include <cstdlib>

// CONSTRUCTOR
Cloth::Cloth() { Cloth(20, 20, 2.0f, 2.0f, f3vec(0, 0, 0), .01, TABLECLOTH); }

Cloth::Cloth(int nx, int ny, float dx, float dy, const f3vec& startPos, double timestep, int style)
{
    nX_ = nx;
    nY_ = ny;
    numParticles_ = nx * ny;
    numTriangles_ = 2 * (nx - 1) * (ny - 1);
    restDX_ = dx;
    restDY_ = dy;
    restDDiag_ = sqrt(dx * dx + dy * dy);
    startPos_ = startPos;
    timeStep_ = timestep;
    clothStyle_ = style;
    gravity_ = f3vec(0, -40, 0);

    // Create cloth node points and constraints
    x_ = new f3vec[numParticles_];
    oldx_ = new f3vec[numParticles_];
    a_ = new f3vec[numParticles_];
    triangles_ = new int[numTriangles_ * 3];
    colors_ = new float[numParticles_ * 3];
    texCoords_ = new float[numParticles_ * 2];
    Reset();

    // Create colliders
    CreateSpheres();
    CreateQuads();
    isSphereMode_ = true;

    // Read texture
    ReadTexture("PlaidCloth.jpg");
}

// DESTRUCTOR
Cloth::~Cloth() {}

void Cloth::SetCollideMode(bool isSphereMode) { isSphereMode_ = isSphereMode; }

void Cloth::CreateSpheres()
{
    numSpheres_ = 3;
    spherePos_ = new f3vec[numSpheres_];
    sphereRadius_ = new float[numSpheres_];
    spherePos_[0] = f3vec(-7.5, 0, 0);
    sphereRadius_[0] = 10.0;
    spherePos_[1] = f3vec(7.5, 0, 0);
    sphereRadius_[1] = 10.0;
    spherePos_[2] = f3vec(7.5, 0, 15);
    sphereRadius_[2] = 10.0;
}

void Cloth::CreateQuads()
{
    float hx = 15.0f;
    float hz = 15.0f;

    numQuads_ = 1;
    collisionQuads_ = new CollisionQuad[1];
    collisionQuads_[0].kNormal_ = f3vec(0, 1, 0);
    collisionQuads_[0].kV_[0] = f3vec(-hx, 0, -hz);
    collisionQuads_[0].kV_[1] = f3vec(hx, 0, -hz);
    collisionQuads_[0].kV_[2] = f3vec(hx, 0, hz);
    collisionQuads_[0].kV_[3] = f3vec(-hx, 0, hz);
}

void Cloth::MoveSphere(const f3vec& dx)
{
    for (int i = 0; i < numSpheres_; i++) { spherePos_[i] += dx; }
}

void Cloth::MoveQuads(const f3vec& dx)
{
    for (int i = 0; i < numQuads_; i++) {
        collisionQuads_[i].kV_[0] += dx;
        collisionQuads_[i].kV_[1] += dx;
        collisionQuads_[i].kV_[2] += dx;
        collisionQuads_[i].kV_[3] += dx;
    }
}

void Cloth::Reset()
{
    // Find width/height of cloth
    double width = nX_ * restDX_;
    double height = nY_ * restDY_;
    f3vec offset(-width / 2.0, 10, -height / 2.0);
    std::cerr << offset << '\n';

    int index = 0;
    if (clothStyle_ == TABLECLOTH) {
        // Create particles in a grid pattern
        for (int j = 0; j < nY_; j++) {
            for (int i = 0; i < nX_; i++) {
                index = i + nX_ * j;
                x_[index] = oldx_[index] = startPos_ + offset + f3vec(restDX_ * i, 0, restDY_ * j);
                colors_[3 * index] = (float)i / nX_;
                colors_[3 * index + 1] = (float)j / nY_;
                colors_[3 * index + 2] = .5f;
                texCoords_[2 * index] = 10 * (float)i / nX_;
                texCoords_[2 * index + 1] = 10 * (float)j / nY_;
            }
        }

        // CREATE CONSTRAINTS
        index = 0;
        // !!!!!!!!!!!! NEED THESE (or cloth will fall apart) !!!!!!!
        for (int j = 0; j < nY_ - 1; j++) {
            for (int i = 0; i < nX_ - 1; i++) {
                // Index points
                // p1---p2
                //  |    |
                // p3---p4
                f3vec* p1 = &x_[i + nX_ * j];
                f3vec* p2 = &x_[i + 1 + nX_ * j];
                f3vec* p3 = &x_[i + nX_ * (j + 1)];
                f3vec* p4 = &x_[i + 1 + nX_ * (j + 1)];
                // Horizontal springs
                Constraint* hC = new RodConstraint(p1, p2, restDX_);
                constraints_.push_back(hC);

                // Diagonal springs
                Constraint* dC = new RodConstraint(p1, p4, restDDiag_);
                constraints_.push_back(dC);

                // Vertical springs
                Constraint* vC = new RodConstraint(p1, p3, restDY_);
                constraints_.push_back(vC);
            }
        }
        // Last row
        for (int i = 0; i < nX_ - 1; i++) {
            f3vec* p1 = &x_[i + nX_ * (nY_ - 1)];
            f3vec* p2 = &x_[i + 1 + nX_ * (nY_ - 1)];
            Constraint* hC = new RodConstraint(p1, p2, restDX_);
            constraints_.push_back(hC);
        }
        // Last column
        for (int j = 0; j < nY_ - 1; j++) {
            f3vec* p1 = &x_[nX_ - 1 + j * nX_];
            f3vec* p2 = &x_[nX_ - 1 + (j + 1) * nX_];
            Constraint* vC = new RodConstraint(p1, p2, restDY_);
            constraints_.push_back(vC);
        }
        // End of !!!!!! NEED THESE !!!!!! section

        // THE REST OF THE CONSTRAINTS ARE FOR CURTAIN-LIKE BEHAVIOR
#ifdef CURTAIN
        f3vec* p1;
        Constraint* pC;

        // Fix two corner points
        /*
        p1 = &x_[0];
        pC = new PointConstraint(p1, *p1);
        constraints_.push_back(pC);

        p1 = &x_[29];
        pC = new PointConstraint(p1, *p1);
        constraints_.push_back(pC);
        */

        // Constrain top of cloth to x_axis
        for (i = 1; i < nX_; i++) {
            if (i % 4 == 0) {
                p1 = &x_[i];
                f3vec cp(p1->x * .8, p1->y, p1->z);
                // pC = new SlideConstraint(p1,*p1, CY_AXIS | CZ_AXIS);
                pC = new PointConstraint(p1, cp);
                constraints_.push_back(pC);
            }
        }
#endif

        // Shuffle constraints
        for (int i = constraints_.size() - 1; i >= 0; i--) {
            int j = LRand(i + 1);
            // std::swap
            Constraint* temp = constraints_[i];
            constraints_[i] = constraints_[j];
            constraints_[j] = temp;
        }

        // Create triangles
        index = 0;
        for (int j = 0; j < nY_ - 1; j++) {
            for (int i = 0; i < nX_ - 1; i++) {
                triangles_[3 * index] = i + j * nX_;
                triangles_[3 * index + 1] = i + (j + 1) * nX_;
                triangles_[3 * index + 2] = i + 1 + (j + 1) * nX_;
                index++;
                triangles_[3 * index] = i + j * nX_;
                triangles_[3 * index + 1] = i + 1 + (j + 1) * nX_;
                triangles_[3 * index + 2] = i + 1 + j * nX_;
                index++;
            }
        }
    }
}

// Verlet()
// Integration step
void Cloth::Verlet()
{
    for (int i = 0; i < numParticles_; i++) {
        f3vec& x = x_[i];
        f3vec temp = x;
        f3vec& oldx = oldx_[i];
        f3vec& a = a_[i];

        // Verlet integration.  x-oldx is an approximation of velocity.
        x += x * KDAMPING - oldx * KDAMPING + a * timeStep_ * timeStep_;
        oldx = temp;
    }
}

// XXX: Replace with Vector.h
f3vec vmin(const f3vec& a, const f3vec& b)
{
    float x = (a.x < b.x) ? a.x : b.x;
    float y = (a.y < b.y) ? a.y : b.y;
    float z = (a.z < b.z) ? a.z : b.z;
    return f3vec(x, y, z);
}

f3vec vmax(const f3vec& a, const f3vec& b)
{
    float x = (a.x >= b.x) ? a.x : b.x;
    float y = (a.y >= b.y) ? a.y : b.y;
    float z = (a.z >= b.z) ? a.z : b.z;
    return f3vec(x, y, z);
}

// SatisfyConstraints
void Cloth::SatisfyConstraints()
{
    for (int j = 0; j < NUM_ITERATIONS; j++) {
        if (isSphereMode_)
            CollisionWithSphere();
        else
            CollisionWithQuad();

        for (int i = 0; i < constraints_.size(); i++) {
            Constraint* c = constraints_[i];
            c->Apply();
        }
    }
}

// AccumulateForces
// Accumulate forces on each particle
void Cloth::AccumulateForces()
{
    // All particles are affected by gravity
    for (int i = 0; i < numParticles_; i++) { a_[i] = gravity_; }
}

// CollisionWithSphere
void Cloth::CollisionWithSphere()
{
    // Collide with sphere with center spherePos_, radius sphereRadius_
    for (int i = 0; i < numParticles_; i++) {
        double lengthV;

        for (int j = 0; j < numSpheres_; j++) {
            f3vec V = x_[i] - spherePos_[j];
            lengthV = V.length();

            // Test for intersection with sphere
            if (lengthV < sphereRadius_[j]) {
                // Translate point outwards along radius
                x_[i] = spherePos_[j] + (V / lengthV) * sphereRadius_[j];
            }
        }
    }
}

void Cloth::CollisionWithQuad()
{
    // Assumes quad is in the XZ plane
    for (int j = 0; j < numQuads_; j++) {
        // Check xz bounds of each point
        for (int i = 0; i < numParticles_; i++) {
            if (x_[i].x > collisionQuads_[j].kV_[0].x && x_[i].x < collisionQuads_[j].kV_[1].x && x_[i].z > collisionQuads_[j].kV_[0].z &&
                x_[i].z < collisionQuads_[j].kV_[3].z) {
                // The point is over the polygon, check height
                if (x_[i].y > collisionQuads_[j].kV_[0].y - .1 && x_[i].y < collisionQuads_[j].kV_[0].y + .1) {
                    // Add a point constraint
                    f3vec constrainTo(x_[i].x, collisionQuads_[j].kV_[0].y, x_[i].z);
                    Constraint* pC;
                    pC = new PointConstraint(&x_[i], constrainTo);
                    constraints_.push_back(pC);
                }
            }
        }
    }
}

// MAIN LOOP
// TimeStep()
// Add up forces, advance system, satisfy constraints
void Cloth::TimeStep()
{
    AccumulateForces();
    Verlet();
    SatisfyConstraints();

    // f3vec ctr(0.f);
    // Aabb box;
    //
    // for (int i = 0; i < numParticles_; i++) {
    //     ctr += x_[i];
    //     box.grow(x_[i]);
    // }
    // ctr /= (float)numParticles_;
    // std::cerr << "TimeStep: " << ctr << box << '\n';
}

// Display()
void Cloth::Display(int mode)
{
    glEnable(GL_DEPTH_TEST);

    if (mode & DRAW_POINTS) {
        glPointSize(6.0);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_POINTS);
        for (int i = 0; i < numParticles_; i++) { glVertex3f(x_[i].x, x_[i].y, x_[i].z); }
        glEnd();
    }

    if (mode & DRAW_TRIS) {
        glColor3f(1, 1, 1);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        // glPolygonMode(GL_FRONT, GL_FILL);
        // glPolygonMode(GL_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);
        // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glBindTexture(GL_TEXTURE_2D, texID_);

        for (int i = 0; i < numTriangles_; i++) {
            f3vec &a = x_[triangles_[3 * i]], &b = x_[triangles_[3 * i + 1]], &c = x_[triangles_[3 * i + 2]];
            f3vec n1 = Cross((c - b), (a - b)); // XXX: Make sure the winding is correct here.
            // n1.normalize();
            glBegin(GL_TRIANGLES);
            glNormal3f(n1.x, n1.y, n1.z);
            glTexCoord2fv(&texCoords_[2 * triangles_[3 * i]]);
            glVertex3f(a.x, a.y, a.z); // glColor3fv(colors_[triangles_[i][0]]);
            glTexCoord2fv(&texCoords_[2 * triangles_[3 * i + 1]]);
            glVertex3f(b.x, b.y, b.z); // glColor3fv(colors_[triangles_[i][1]]);
            glTexCoord2fv(&texCoords_[2 * triangles_[3 * i + 2]]);
            glVertex3f(c.x, c.y, c.z); // glColor3fv(colors_[triangles_[i][2]]);
            glEnd();
            // Draw Normal
            /*
            glLineWidth(3.0);
            glColor3f(1,1,1);
            glDisable(GL_LIGHTING);
            glBegin(GL_LINES);
            f3vec &alpha = (a-b);
            f3vec &beta = (c-b);
            f3vec &pni = b + alpha*.5 + beta*.5;
            f3vec &pnf = pni + n1*1.0;
            glVertex3f(pni.x,pni.y,pni.z);
            glVertex3f(pnf.x, pnf.y, pnf.z);
            glEnd();
            glEnable(GL_LIGHTING);
            */

            // glVertex3f(a.x, a.y, a.z);  glColor3fv(&colors_[3*triangles_[3*i]]);
            // glVertex3f(b.x, b.y, b.z);  glColor3fv(&colors_[3*triangles_[3*i+1]]);
            // glVertex3f(c.x, c.y, c.z);  glColor3fv(&colors_[3*triangles_[3*i+2]]);
        }
        // glTexCoord2f(0.0,0.0); glVertex3f(-5,0,-5);
        // glTexCoord2f(0.0,1.0); glVertex3f(-5,0, 5);
        // glTexCoord2f(1.0,1.0); glVertex3f(5,0,5);
        // glTexCoord2f(1.0,0.0); glVertex3f(5,0,-5);
        // glEnd();
        glFlush();
        glDisable(GL_TEXTURE_2D);
        // glDisable(GL_LIGHTING);
    }
    GL_ASSERT();

    if (isSphereMode_) {
        // Draw Sphere
        for (int i = 0; i < numSpheres_; i++) {
            glPushMatrix();
            glTranslatef(spherePos_[i].x, spherePos_[i].y, spherePos_[i].z);
            glutWireSphere(.9 * sphereRadius_[i], 10, 10);
            glPopMatrix();
        }
    } else {
        // Draw quads
        glColor3f(.5f, .5f, .5f);
        glBegin(GL_QUADS);
        for (int i = 0; i < numQuads_; i++) {
            glNormal3f(collisionQuads_[i].kNormal_.x, collisionQuads_[i].kNormal_.y, collisionQuads_[i].kNormal_.z);
            glVertex3f(collisionQuads_[i].kV_[0].x, collisionQuads_[i].kV_[0].y, collisionQuads_[i].kV_[0].z);
            glVertex3f(collisionQuads_[i].kV_[1].x, collisionQuads_[i].kV_[1].y, collisionQuads_[i].kV_[1].z);
            glVertex3f(collisionQuads_[i].kV_[2].x, collisionQuads_[i].kV_[2].y, collisionQuads_[i].kV_[2].z);
            glVertex3f(collisionQuads_[i].kV_[3].x, collisionQuads_[i].kV_[3].y, collisionQuads_[i].kV_[3].z);
        }
        glEnd();
    }
    GL_ASSERT();
}

void Cloth::ReadTexture(const char* texName)
{
    uc3Image texIm(texName);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &texID_);
    glBindTexture(GL_TEXTURE_2D, texID_);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texIm.w(), texIm.h(), 0, GL_RGB, GL_UNSIGNED_BYTE, texIm.pp());
    GL_ASSERT();
}

void Cloth::WriteTriModel(const char* FileName)
{
    printf("Writing to %s (%d triangles). . .\n", FileName, numTriangles_);

    unsigned int HexColor;

    FILE* fp = fopen(FileName, "w");
    if (fp == NULL) {
        printf("ERROR: unable to open TriObj [%s]!\n", FileName);
        exit(1);
    }

    fprintf(fp, "%d\n", numTriangles_);
    for (int i = 0; i < numTriangles_; i++) {
        f3vec &a = x_[triangles_[3 * i]], &b = x_[triangles_[3 * i + 1]], &c = x_[triangles_[3 * i + 2]];
        fprintf(fp, "%f %f %f ", a.x, a.y, a.z);
        fprintf(fp, "%f %f %f ", b.x, b.y, b.z);
        fprintf(fp, "%f %f %f ", c.x, c.y, c.z);

        HexColor = (((unsigned int)255) << 16) + (((unsigned int)255) << 8) + ((unsigned int)255);
        fprintf(fp, "%x\n", HexColor);
    }
    fclose(fp);
}
