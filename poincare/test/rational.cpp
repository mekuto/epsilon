#include <quiz.h>
#include <poincare.h>
#include <assert.h>
#include "helper.h"
#include "tree/helpers.h"

using namespace Poincare;

QUIZ_CASE(poincare_rational_constructor) {
  Rational a(Integer("123"), Integer("324"));
  Rational b(Integer("3456"));
  Rational c(123,324);
  Rational d(3456789);
  Rational e(Integer::Overflow());
  Rational f(Integer::Overflow(), Integer::Overflow());
  assert_pool_size(6);
#if POINCARE_TREE_LOG
  log_pool();
#endif
}

static inline void assert_equal(const Rational i, const Rational j) {
  assert(Rational::NaturalOrder(i, j) == 0);
}
static inline void assert_not_equal(const Rational i, const Rational j) {
  assert(Rational::NaturalOrder(i, j) != 0);
}

static inline void assert_lower(const Rational i, const Rational j) {
  assert(Rational::NaturalOrder(i, j) < 0);
}

static inline void assert_greater(const Rational i, const Rational j) {
  assert(Rational::NaturalOrder(i, j) > 0);
}

QUIZ_CASE(poincare_rational_compare) {
  assert_equal(Rational(123,324), Rational(41,108));
  assert_not_equal(Rational(123,234), Rational(42, 108));
  assert_lower(Rational(123,234), Rational(456,567));
  assert_lower(Rational(-123, 234),Rational(456, 567));
  assert_greater(Rational(123, 234),Rational(-456, 567));
  assert_greater(Rational(123, 234),Rational("123456789123456789", "12345678912345678910"));
}

QUIZ_CASE(poincare_rational_properties) {
  assert(Rational(-2).sign() == ExpressionNode::Sign::Negative);
  assert(Rational(-2, 3).sign() == ExpressionNode::Sign::Negative);
  assert(Rational(2, 3).sign() == ExpressionNode::Sign::Positive);
  assert(Rational(0).isZero());
  assert(!Rational(231).isZero());
  assert(Rational(1).isOne());
  assert(!Rational(-1).isOne());
  assert(!Rational(1).isMinusOne());
  assert(Rational(-1).isMinusOne());
  assert(Rational(1,2).isHalf());
  assert(!Rational(-1).isHalf());
  assert(Rational(-1,2).isMinusHalf());
  assert(!Rational(3,2).isMinusHalf());
  assert(Rational(10).isTen());
  assert(!Rational(-1).isTen());
}

QUIZ_CASE(poincare_rational_evaluate) {
#if 0
  assert_parsed_expression_evaluates_to<float>("1/3", "0.3333333");
  assert_parsed_expression_evaluates_to<double>("123456/1234567", "9.9999432999586E-2");
#endif
}

QUIZ_CASE(poincare_rational_simplify) {
#if 0
  assert_parsed_expression_simplify_to("-1/3", "-1/3");
  assert_parsed_expression_simplify_to("22355/45325", "4471/9065");
  assert_parsed_expression_simplify_to("0000.000000", "0");
  assert_parsed_expression_simplify_to(".000000", "0");
  assert_parsed_expression_simplify_to("0000", "0");
  assert_parsed_expression_simplify_to("0.1234567", "1234567/10000000");
  assert_parsed_expression_simplify_to("123.4567", "1234567/10000");
  assert_parsed_expression_simplify_to("0.1234", "617/5000");
  assert_parsed_expression_simplify_to("0.1234000", "617/5000");
  assert_parsed_expression_simplify_to("001234000", "1234000");
  assert_parsed_expression_simplify_to("001.234000E3", "1234");
  assert_parsed_expression_simplify_to("001234000E-4", "617/5");
  assert_parsed_expression_simplify_to("3/4+5/4-12+1/567", "-5669/567");
  assert_parsed_expression_simplify_to("34/78+67^(-1)", "1178/2613");
  assert_parsed_expression_simplify_to("12348/34564", "3087/8641");
  assert_parsed_expression_simplify_to("1-0.3-0.7", "0");
  assert_parsed_expression_simplify_to("123456789123456789+112233445566778899", "235690234690235688");
  assert_parsed_expression_simplify_to("56^56", "79164324866862966607842406018063254671922245312646690223362402918484170424104310169552592050323456");
  assert_parsed_expression_simplify_to("999^999", "999^999");
  assert_parsed_expression_simplify_to("999^-999", "1/999^999");
  assert_parsed_expression_simplify_to("0^0", "undef");
  assert_parsed_expression_simplify_to("x^0", "1");
  assert_parsed_expression_simplify_to("P^0", "1");
  assert_parsed_expression_simplify_to("A^0", "1");
  assert_parsed_expression_simplify_to("(-3)^0", "1");
#endif
}
