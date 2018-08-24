#include <poincare/hyperbolic_arc_sine.h>
#include <poincare/simplification_helper.h>
#include <poincare/trigonometry.h>
extern "C" {
#include <assert.h>
}
#include <cmath>

namespace Poincare {

ExpressionNode::Type HyperbolicArcSine::type() const {
  return Type::HyperbolicArcSine;
}

Expression * HyperbolicArcSine::clone() const {
  HyperbolicArcSine * a = new HyperbolicArcSine(m_operands, true);
  return a;
}

Expression HyperbolicArcSine::shallowReduce(Context& context, Preferences::AngleUnit angleUnit) const {
  Expression e = Expression::defaultShallowReduce(context, angleUnit);
  if (e.isUndefinedOrAllocationFailure()) {
    return e;
  }
#if MATRIX_EXACT_REDUCING
  Expression * op = childAtIndex(0);
  if (op->type() == Type::Matrix) {
    return SimplificationHelper::Map(this, context, angleUnit);
  }
#endif
  return this;
}

template<typename T>
std::complex<T> HyperbolicArcSine::computeOnComplex(const std::complex<T> c, Preferences::AngleUnit angleUnit) {
  std::complex<T> result = std::asinh(c);
  /* asinh has a branch cut on ]-inf*i, -i[U]i, +inf*i[: it is then multivalued
   * on this cut. We followed the convention chosen by the lib c++ of llvm on
   * ]+i+0, +i*inf+0[ (warning: atanh takes the other side of the cut values on
   * ]+i-0, +i*inf+0[) and choose the values on ]-inf*i, -i[ to comply with
   * asinh(-x) = -asinh(x). */
  if (c.real() == 0 && c.imag() < 1) {
    result.real(-result.real()); // other side of the cut
  }
  return Trigonometry::RoundToMeaningfulDigits(result);
}

}
