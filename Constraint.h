#pragma once

#include "Math/Vector.h"

enum ConstraintType { ROD_CONSTRAINT, POINT_CONSTRAINT };

enum ConstrainAxis { CX_AXIS = 1, CY_AXIS = 2, CZ_AXIS = 4 };

class Constraint {
public:
    Constraint() {}
    ~Constraint() {}
    virtual void Apply() = 0;
};

class PointConstraint : public Constraint {
public:
    PointConstraint() {};
    ~PointConstraint() {};
    PointConstraint(f3vec* pa, const f3vec& fp);
    void Apply();
    f3vec* pA_;
    f3vec fixedPos_;
};

class RodConstraint : public Constraint {
public:
    RodConstraint() {};
    ~RodConstraint() {};
    RodConstraint(f3vec* pa, f3vec* pb, float rl);
    void Apply();
    f3vec *pA_, *pB_; // Pointers to 2 particles connected by a rod
    float restLength_;
    float restLengthSquared_;
};

class SlideConstraint : public Constraint {
public:
    SlideConstraint() {};
    ~SlideConstraint() {};
    SlideConstraint(f3vec* pa, const f3vec& fp, int axis);

    void Apply();
    f3vec* pA_;
    f3vec fixedPos_;
    int constrainAxis_;
};

// INLINE FUNCTIONS
inline void PointConstraint::Apply() { *pA_ = fixedPos_; }

inline void RodConstraint::Apply()
{
    f3vec delta = *pB_ - *pA_;
    /*float deltalength = delta.Length();
    float diff = (deltalength-restLength_)/deltalength;
    *pA_ += delta*0.5*diff;
    *pB_ -= delta*0.5*diff;*/
    delta *= restLengthSquared_ / (delta * delta + restLengthSquared_) - 0.5f;
    *pA_ -= delta;
    *pB_ += delta;
}

inline void SlideConstraint::Apply()
{
    if (constrainAxis_ & CX_AXIS) { (*pA_)[0] = fixedPos_[0]; }
    if (constrainAxis_ & CY_AXIS) { (*pA_)[1] = fixedPos_[1]; }
    if (constrainAxis_ & CZ_AXIS) { (*pA_)[2] = fixedPos_[2]; }
}
