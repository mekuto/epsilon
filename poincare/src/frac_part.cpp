#include <poincare/frac_part.h>
#include <poincare/simplification_helper.h>
#include <poincare/rational.h>
extern "C" {
#include <assert.h>
}
#include <cmath>

namespace Poincare {

ExpressionNode::Type FracPart::type() const {
  return Type::FracPart;
}

Expression * FracPart::clone() const {
  FracPart * c = new FracPart(m_operands, true);
  return c;
}

Expression FracPart::shallowReduce(Context& context, Preferences::AngleUnit angleUnit) const {
  Expression e = Expression::defaultShallowReduce(context, angleUnit);
  if (e.isUndefinedOrAllocationFailure()) {
    return e;
  }
  Expression * op = childAtIndex(0);
#if MATRIX_EXACT_REDUCING
  if (op->type() == Type::Matrix) {
    return SimplificationHelper::Map(this, context, angleUnit);
  }
#endif
  if (op->type() != Type::Rational) {
    return this;
  }
  Rational * r = static_cast<Rational *>(op);
  IntegerDivision div = Integer::Division(r->numerator(), r->denominator());
  return replaceWith(new Rational(div.remainder, r->denominator()), true);
}

template<typename T>
std::complex<T> FracPart::computeOnComplex(const std::complex<T> c, Preferences::AngleUnit angleUnit) {
  if (c.imag() != 0) {
    return Complex<T>::Undefined();
  }
  return Complex<T>(c.real()-std::floor(c.real()));
}

}


