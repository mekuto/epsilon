#include <poincare/confidence_interval.h>
#include <poincare/addition.h>
#include <poincare/layout_helper.h>
#include <poincare/matrix.h>
#include <poincare/multiplication.h>
#include <poincare/power.h>
#include <poincare/serialization_helper.h>
#include <poincare/undefined.h>
#include <cmath>
#include <assert.h>

namespace Poincare {

ConfidenceIntervalNode * ConfidenceIntervalNode::FailedAllocationStaticNode() {
  static AllocationFailureExpressionNode<ConfidenceIntervalNode> failure;
  TreePool::sharedPool()->registerStaticNodeIfRequired(&failure);
  return &failure;
}

LayoutReference ConfidenceIntervalNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(ConfidenceInterval(this), floatDisplayMode, numberOfSignificantDigits, name());
}

int ConfidenceIntervalNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, name());
}

Expression ConfidenceIntervalNode::shallowReduce(Context& context, Preferences::AngleUnit angleUnit, const Expression futureParent) {
  return ConfidenceInterval(this).shallowReduce(context, angleUnit);
}

template<typename T>
Evaluation<T> ConfidenceIntervalNode::templatedApproximate(Context& context, Preferences::AngleUnit angleUnit) const {
  Evaluation<T> fInput = childAtIndex(0)->approximate(T(), context, angleUnit);
  Evaluation<T> nInput = childAtIndex(1)->approximate(T(), context, angleUnit);
  T f = static_cast<Complex<T> &>(fInput).toScalar();
  T n = static_cast<Complex<T> &>(nInput).toScalar();
  if (std::isnan(f) || std::isnan(n) || n != (int)n || n < 0 || f < 0 || f > 1) {
    return Complex<T>::Undefined();
  }
  std::complex<T> operands[2];
  operands[0] = std::complex<T>(f - 1/std::sqrt(n));
  operands[1] = std::complex<T>(f + 1/std::sqrt(n));
  return MatrixComplex<T>(operands, 1, 2);
}

SimplePredictionIntervalNode * SimplePredictionIntervalNode::FailedAllocationStaticNode() {
  static AllocationFailureExpressionNode<SimplePredictionIntervalNode> failure;
  TreePool::sharedPool()->registerStaticNodeIfRequired(&failure);
  return &failure;
}

Expression ConfidenceInterval::shallowReduce(Context& context, Preferences::AngleUnit angleUnit, const Expression futureParent) {
  Expression e = Expression::defaultShallowReduce(context, angleUnit);
  if (e.isUndefinedOrAllocationFailure()) {
    return e;
  }
  Expression c0 = childAtIndex(0);
  Expression c1 = childAtIndex(1);
#if MATRIX_EXACT_REDUCING
  if (c0.type() == ExpressionNode::Type::Matrix || c1.type() == ExpressionNode::Type::Matrix) {
    return Undefined();
  }
#endif
  if (c0.type() == ExpressionNode::Type::Rational) {
    Rational r0 = static_cast<Rational&>(c0);
    if (r0.signedIntegerNumerator().isNegative() || Integer::NaturalOrder(r0.signedIntegerNumerator(), r0.integerDenominator()) > 0) {
      return Undefined();
    }
  }
  if (c1.type() == ExpressionNode::Type::Rational) {
    Rational r1 = static_cast<Rational&>(c1);
    if (!r1.integerDenominator().isOne() || r1.signedIntegerNumerator().isNegative()) {
      return Undefined();
    }
  }
  if (c0.type() != ExpressionNode::Type::Rational || c1.type() != ExpressionNode::Type::Rational) {
    return *this;
  }
  Rational r0 = static_cast<Rational&>(c0);
  Rational r1 = static_cast<Rational&>(c1);
  // Compute [r0-1/sqr(r1), r0+1/sqr(r1)]
  Expression sqr = Power(r1, Rational(-1, 2));
  Matrix matrix;
  matrix.addChildAtIndexInPlace(Addition(r0, Multiplication(Rational(-1), sqr)), 0, 0);
  matrix.addChildAtIndexInPlace(Addition(r0, sqr), 1, 1);
  matrix.setDimensions(1, 2);
  return matrix.deepReduce(context, angleUnit);
}

}

