#include "Constraint.h"

// POINT CONSTRAINTS
PointConstraint::PointConstraint(f3vec* pa, const f3vec& fp)
{
    pA_ = pa;
    fixedPos_ = fp;
}

// ROD CONSTRAINTS
RodConstraint::RodConstraint(f3vec* pa, f3vec* pb, float rl)
{
    pA_ = pa;
    pB_ = pb;
    restLength_ = rl;
    restLengthSquared_ = rl * rl;
}

// SLIDE CONSTRAINTS
SlideConstraint::SlideConstraint(f3vec* pa, const f3vec& fp, int axis)
{
    pA_ = pa;
    fixedPos_ = fp;
    constrainAxis_ = axis;
}
