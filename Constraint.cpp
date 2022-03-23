#include "Constraint.hpp"


// POINT CONSTRAINTS
PointConstraint::PointConstraint(Vec3f *pa, const Vec3f& fp)
{
  pA_ = pa;
  fixedPos_ = fp;
}


// ROD CONSTRAINTS
RodConstraint::RodConstraint(Vec3f *pa, Vec3f *pb, float rl)
{
  pA_ = pa;
  pB_ = pb;
  restLength_ = rl;
  restLengthSquared_ = rl*rl;
}

// SLIDE CONSTRAINTS
SlideConstraint::SlideConstraint(Vec3f *pa, const Vec3f& fp, int axis)
{
  pA_ = pa;
  fixedPos_ = fp;
  constrainAxis_ = axis;
}