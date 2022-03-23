#pragma once

#include "Math/Vector.h"

class Constraint {
public:
    Constraint() {}
    ~Constraint() {}
    virtual void Apply() const = 0;
};

// Constrain particle to specific point
class PointConstraint : public Constraint {
    f3vec* m_pA;
    f3vec m_fixedPos;

public:
    PointConstraint(f3vec* pa, const f3vec& fp) : m_pA(pa), m_fixedPos(fp) {}
    ~PointConstraint() {};
    void Apply() const;
};

// Constrain two particles to a specific distance from each other
class RodConstraint : public Constraint {
    f3vec *m_pA, *m_pB;
    float m_restLen;
    float m_restLenSqr;

public:
    RodConstraint(f3vec* pa, f3vec* pb, float rl) : m_pA(pa), m_pB(pb), m_restLen(rl), m_restLenSqr(rl * rl) {}
    ~RodConstraint() {};
    void Apply() const;
};

// Constrain particle in some axes but allow movement in others
enum ConstrainAxis { CX_AXIS = 1, CY_AXIS = 2, CZ_AXIS = 4 };
class SlideConstraint : public Constraint {
    f3vec* m_pA;
    f3vec m_fixedPos;
    ConstrainAxis m_constrainAxis;

public:
    SlideConstraint(f3vec* pa, const f3vec& fp, ConstrainAxis axis) : m_pA(pa), m_fixedPos(fp), m_constrainAxis(axis) {}
    ~SlideConstraint() {};
    void Apply() const;
};

inline void PointConstraint::Apply() const { *m_pA = m_fixedPos; }

inline void RodConstraint::Apply() const
{
    f3vec delta = *m_pB - *m_pA;
#if 0
    float deltaLen = delta.length();
    float halfDiff = 0.5f * (deltaLen - m_restLen) / deltaLen;
#else
    // Faster because no sqrt, but a bit less accurate
    float halfDiff = -(m_restLenSqr / (dot(delta, delta) + m_restLenSqr) - 0.5f);
#endif
    delta *= halfDiff;
    *m_pA += delta;
    *m_pB -= delta;
}

inline void SlideConstraint::Apply() const
{
    if (m_constrainAxis & CX_AXIS) { (*m_pA)[0] = m_fixedPos[0]; }
    if (m_constrainAxis & CY_AXIS) { (*m_pA)[1] = m_fixedPos[1]; }
    if (m_constrainAxis & CZ_AXIS) { (*m_pA)[2] = m_fixedPos[2]; }
}
