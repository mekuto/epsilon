#include <poincare/binomial_coefficient.h>
#include <poincare/binomial_coefficient_layout_node.h>
#include <poincare/rational.h>
#include <poincare/serialization_helper.h>
#include <poincare/undefined.h>
#include <stdlib.h>
#include <assert.h>
#include <cmath>

namespace Poincare {

BinomialCoefficientNode * BinomialCoefficientNode::FailedAllocationStaticNode() {
  static AllocationFailureExpressionNode<BinomialCoefficientNode> failure;
  TreePool::sharedPool()->registerStaticNodeIfRequired(&failure);
  return &failure;
}

Expression BinomialCoefficientNode::shallowReduce(Context& context, Preferences::AngleUnit angleUnit, const Expression futureParent) {
  return BinomialCoefficient(this).shallowReduce(context, angleUnit);
}

LayoutReference BinomialCoefficientNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return BinomialCoefficientLayoutRef(
      childAtIndex(0)->createLayout(floatDisplayMode, numberOfSignificantDigits),
      childAtIndex(1)->createLayout(floatDisplayMode, numberOfSignificantDigits));
}

int BinomialCoefficientNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
    return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, "binomial");
}

template<typename T>
Complex<T> BinomialCoefficientNode::templatedApproximate(Context& context, Preferences::AngleUnit angleUnit) const {
  Evaluation<T> nInput = childAtIndex(0)->approximate(T(), context, angleUnit);
  Evaluation<T> kInput = childAtIndex(1)->approximate(T(), context, angleUnit);
  T n = nInput.toScalar();
  T k = kInput.toScalar();
  return Complex<T>(compute(k, n));
}

template<typename T>
T BinomialCoefficientNode::compute(T k, T n) {
  k = k > (n-k) ? n-k : k;
  if (std::isnan(n) || std::isnan(k) || n != std::round(n) || k != std::round(k) || k > n || k < 0 || n < 0) {
    return NAN;
  }
  T result = 1;
  for (int i = 0; i < k; i++) {
    result *= (n-(T)i)/(k-(T)i);
    if (std::isinf(result) || std::isnan(result)) {
      return result;
    }
  }
  return std::round(result);
}

Expression BinomialCoefficient::shallowReduce(Context& context, Preferences::AngleUnit angleUnit, const Expression futureParent) {
  Expression e = Expression::defaultShallowReduce(context, angleUnit);
  if (e.isUndefinedOrAllocationFailure()) {
    return e;
  }
  Expression op0 = childAtIndex(0);
  Expression op1 = childAtIndex(1);
#if MATRIX_EXACT_REDUCING
  if (op0.type() == ExpressionNode::Type::Matrix || op1.type() == ExpressionNode::Type::Matrix) {
    return Undefined();
  }
#endif
  if (op0.type() == ExpressionNode::Type::Rational) {
    Rational r0 = static_cast<Rational>(op0);
    if (!r0.integerDenominator().isOne() || r0.integerDenominator().isNegative()) {
      return Undefined();
    }
  }
  if (op1.type() == ExpressionNode::Type::Rational) {
    Rational r1 = static_cast<Rational>(op1);
    if (!r1.integerDenominator().isOne() || r1.integerDenominator().isNegative()) {
      return Undefined();
    }
  }
  if (op0.type() != ExpressionNode::Type::Rational || op1.type() != ExpressionNode::Type::Rational) {
    return *this;
  }
  Rational r0 = static_cast<Rational>(op0);
  Rational r1 = static_cast<Rational>(op1);

  Integer n = r0.signedIntegerNumerator();
  Integer k = r1.signedIntegerNumerator();
  if (n.isLowerThan(k)) {
    return Undefined();
  }
  /* If n is too big, we do not reduce in order to avoid too long computation.
   * The binomial coefficient will be approximatively evaluated later. */
  if (Integer(k_maxNValue).isLowerThan(n)) {
    return *this;
  }
  Rational result(1);
  Integer kBis = Integer::Subtraction(n, k);
  k = kBis.isLowerThan(k) ? kBis : k;
  int clippedK = k.extractedInt(); // Authorized because k < n < k_maxNValue
  for (int i = 0; i < clippedK; i++) {
    Rational factor = Rational(Integer::Subtraction(n, Integer(i)), Integer::Subtraction(k, Integer(i)));
    result = Rational::Multiplication(result, factor);
  }
  // As we cap the n < k_maxNValue = 300, result < binomial(300, 150) ~2^89
  assert(!result.numeratorOrDenominatorIsInfinity());
  return Rational(result);
}

template double BinomialCoefficientNode::compute(double k, double n);
template float BinomialCoefficientNode::compute(float k, float n);

}
