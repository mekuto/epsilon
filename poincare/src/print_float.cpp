#include <poincare/print_float.h>
#include <poincare/ieee754.h>
#include <poincare/infinity.h>
#include <poincare/preferences.h>
#include <poincare/print_int.h>
#include <poincare/serialization_helper.h>
#include <poincare/undefined.h>
#include <ion/unicode/utf8_decoder.h>
#include <ion/unicode/utf8_helper.h>
extern "C" {
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <limits.h>
}
#include <cmath>

namespace Poincare {

static inline int minInt(int x, int y) { return x < y ? x : y; }

PrintFloat::Long::Long(int64_t i) :
  m_negative(i < 0)
{
  m_digits[1] = i % k_base;
  m_digits[0] = (i - m_digits[1]) / k_base;
}

PrintFloat::Long::Long(uint32_t d1, uint32_t d2, bool negative) :
  m_negative(negative)
{
  m_digits[1] = d2;
  m_digits[0] = d1;
  assert((!negative) || (!isZero()));
}

void PrintFloat::Long::DivisionByTen(const Long & longToDivide, Long * quotient, Long * digit) {
  assert(quotient != nullptr && digit != nullptr);
  if (longToDivide.isZero()) {
    *quotient = Long(0);
    *digit = Long(0);
    return;
  }

  uint32_t digit0 = longToDivide.digit(0);
  uint32_t digit1 = longToDivide.digit(1);

  uint32_t d = digit1 % 10;
  uint32_t digit1DividedByTen = (digit1 - d)/10;
  *digit = Long(d);


  if (digit0 == 0) {
    *quotient = Long(0, digit1DividedByTen, longToDivide.isNegative());
  } else {
    *quotient = Long(digit0/10, digit1DividedByTen + k_base * digit0 % 10, longToDivide.isNegative());
  }
}

int PrintFloat::Long::serialize(char * buffer, int bufferSize) const {
  int numberOfChars = m_negative ? 1 : 0; // 1 for the minus sign char
  if (m_digits[0] != 0) {
    numberOfChars += PrintInt::Left(m_digits[0], buffer + numberOfChars, bufferSize - numberOfChars - 1);
    int wantedNumberOfChars = numberOfChars + k_maxNumberOfCharsForDigit + 1;
    if (wantedNumberOfChars > bufferSize) {
      /* There is not enough space for serializing the second digit and the null
       * terminating char. */
      return wantedNumberOfChars;
    }
    numberOfChars+= PrintInt::Right(m_digits[1], buffer + numberOfChars, minInt(k_maxNumberOfCharsForDigit, bufferSize - numberOfChars - 1));
  } else {
    numberOfChars+= PrintInt::Left(m_digits[1], buffer + numberOfChars, bufferSize - numberOfChars - 1);
  }
  if (m_negative) {
    assert(UTF8Decoder::CharSizeOfCodePoint('-') == 1);
    buffer[0] = '-';
  }
  assert(numberOfChars <= bufferSize - 1);
  buffer[numberOfChars] = 0;
  return numberOfChars;
}

void PrintFloat::PrintLongWithDecimalMarker(char * buffer, int bufferLength, Long & i, int decimalMarkerPosition) {
  /* The decimal marker position is always preceded by a char, thus, it is never
   * in first position. When called by convertFloatToText, the buffer length is
   * always > 0 as we asserted a minimal number of available chars. */
  assert(bufferLength > 0 && decimalMarkerPosition != 0);
  constexpr int tempBufferSize = PrintFloat::k_maxFloatBufferLength;
  char tempBuffer[tempBufferSize];
  int intLength = i.serialize(tempBuffer, tempBufferSize);
  int firstDigitChar = UTF8Helper::CodePointIs(tempBuffer, '-') ? 1 : 0;
  /* We should use the UTF8Decoder to write code points in buffers, but it is
   * much clearer to manipulate chars directly as we know that the code point we
   * use ('.', '0, '1', '2', ...) are only one char long. */
  for (int k = bufferLength-1; k >= firstDigitChar; k--) {
    if (k == decimalMarkerPosition) {
      assert(UTF8Decoder::CharSizeOfCodePoint('.') == 1);
      buffer[k] = '.';
      continue;
    }
    if (intLength > firstDigitChar) {
      assert(UTF8Decoder::CharSizeOfCodePoint(tempBuffer[intLength-1]) == 1);
      buffer[k] = tempBuffer[--intLength];
      continue;
    }
    assert(UTF8Decoder::CharSizeOfCodePoint('0') == 1);
    buffer[k] = '0';
  }
  if (firstDigitChar == 1) {
    assert(UTF8Decoder::CharSizeOfCodePoint(tempBuffer[0]) == 1);
    buffer[0] = tempBuffer[0];
  }
}

template <class T>
int PrintFloat::convertFloatToText(T f, char * buffer, int bufferSize,
    int numberOfSignificantDigits, Preferences::PrintFloatMode mode, bool allowRounding)
{
  assert(numberOfSignificantDigits > 0);
  assert(bufferSize > 0);

  int numberOfZerosRemoved = 0;
  int requiredLength = ConvertFloatToTextPrivate(f, buffer, bufferSize, numberOfSignificantDigits, mode, &numberOfZerosRemoved);
  /* If the required buffer size overflows the buffer size, we first force the
   * display mode to scientific and decrease the number of significant digits to
   * fit the buffer size. */
  if (mode == Preferences::PrintFloatMode::Decimal && requiredLength >= bufferSize) {
    requiredLength = ConvertFloatToTextPrivate(f, buffer, bufferSize, numberOfSignificantDigits, Preferences::PrintFloatMode::Scientific, &numberOfZerosRemoved);
  }
  if (requiredLength >= bufferSize) {
    /* If the buffer size is still too small and rounding is allowed, we only
     * write the beginning of the float and truncate it (which can result in a
     * non sense text). If no rounding is allowed, we set the text to null. */
    if (!allowRounding) {
      buffer[0] = 0;
      return requiredLength;
    }
    int adjustedNumberOfSignificantDigits = numberOfSignificantDigits - numberOfZerosRemoved - requiredLength + bufferSize - 1;
    adjustedNumberOfSignificantDigits = adjustedNumberOfSignificantDigits < 1 ? 1 : adjustedNumberOfSignificantDigits;
    requiredLength = ConvertFloatToTextPrivate(f, buffer, bufferSize, adjustedNumberOfSignificantDigits, Preferences::PrintFloatMode::Scientific, &numberOfZerosRemoved);
  }
  return (requiredLength < bufferSize) ? requiredLength : (bufferSize - 1);
}

template <class T>
int PrintFloat::ConvertFloatToTextPrivate(T f, char * buffer, int bufferSize, int numberOfSignificantDigits, Preferences::PrintFloatMode mode, int * numberOfRemovedZeros) {
  assert(numberOfSignificantDigits > 0);
  if (std::isinf(f)) {
    // Infinity
    assert(Infinity::NameSize()+1 < bufferSize);
    int currentChar = 0;
    if (f < 0) {
      currentChar+= SerializationHelper::CodePoint(buffer + currentChar, bufferSize - currentChar, '-');
    }
    strlcpy(&buffer[currentChar], Infinity::Name(), bufferSize-1);
    return currentChar + Infinity::NameSize() - 1;
  }

  if (std::isnan(f)) {
    // Nan
    assert(Undefined::NameSize() < bufferSize);
    strlcpy(buffer, Undefined::Name(), bufferSize);
    return Undefined::NameSize() - 1;
  }

  int exponentInBase10 = IEEE754<T>::exponentBase10(f);

  /* Part I: Mantissa */

  // Compute mantissa
  T unroundedMantissa = f * std::pow((T)10.0, (T)(numberOfSignificantDigits - 1 - exponentInBase10));
  // Round mantissa to get the right number of significant digits
  T mantissa = std::round(unroundedMantissa);

  /* If (numberOfSignificantDigits - 1 - exponentInBase10) is too big (or too
   * small), mantissa is now inf. We handle this case by using logarithm
   * function. */
  if (std::isnan(mantissa) || std::isinf(mantissa)) {
    mantissa = std::round(std::pow(10, std::log10(std::fabs(f))+(T)(numberOfSignificantDigits -1 - exponentInBase10)));
    mantissa = std::copysign(mantissa, f);
  }
  /* We update the exponent in base 10 (if 0.99999999 was rounded to 1 for
   * instance)
   * NB: the following if-condition should rather be:
   * "exponentBase10(unroundedMantissa) != exponentBase10(mantissa)",
   * However, unroundedMantissa can have a different exponent than expected
   * (ex: f = 1E13, unroundedMantissa = 99999999.99 and mantissa = 1000000000) */
  if (f != 0 && IEEE754<T>::exponentBase10(mantissa) - exponentInBase10 != numberOfSignificantDigits - 1 - exponentInBase10) {
    exponentInBase10++;
  }

  if (mode == Preferences::PrintFloatMode::Decimal && exponentInBase10 >= numberOfSignificantDigits) {
    /* Exception 1: avoid inventing digits to fill the printed float: when
     * displaying 12345 with 2 significant digis in Decimal mode for instance.
     * This exception is caught by convertFloatToText and forces the mode to
     * Scientific */
    return INT_MAX;
  }

  // Correct the number of digits in mantissa after rounding
  if (IEEE754<T>::exponentBase10(mantissa) >= numberOfSignificantDigits) {
    mantissa = mantissa/10.0;
  }

  // Number of chars for the mantissa
  int numberOfCharsForMantissaWithoutSign = (exponentInBase10 >= 0 || mode == Preferences::PrintFloatMode::Scientific) ? numberOfSignificantDigits : (numberOfSignificantDigits - exponentInBase10);

  /* The number of digits in a mantissa is capped because the maximal int64_t is
   * 2^63 - 1. As our mantissa is an integer built from an int64_t, we assert
   * that we stay beyond this threshold during computation. */
  assert(numberOfSignificantDigits < std::log10(std::pow(2.0f, 63.0f)));

  // Remove the zeroes on the right side of the mantissa
  Long dividend = Long((int64_t)mantissa);
  Long digit;
  Long quotient;
  Long::DivisionByTen(dividend, &quotient, &digit);
  int minimumNumberOfCharsInMantissa = 1;
  int numberOfZerosRemoved = 0;
  while (digit.isZero()
      && numberOfCharsForMantissaWithoutSign > minimumNumberOfCharsInMantissa
      && (numberOfCharsForMantissaWithoutSign > exponentInBase10 + 1
        || mode == Preferences::PrintFloatMode::Scientific))
  {
    assert(UTF8Decoder::CharSizeOfCodePoint('0') == 1);
    numberOfCharsForMantissaWithoutSign--;
    dividend = quotient;
    Long::DivisionByTen(dividend, &quotient, &digit);
    numberOfZerosRemoved++;
  }
  if (numberOfRemovedZeros != nullptr) {
    *numberOfRemovedZeros = numberOfZerosRemoved;
  }

  /* Part II: Decimal marker */

  // Force a decimal marker if there is fractional part
  bool decimalMarker = (mode == Preferences::PrintFloatMode::Scientific
      && numberOfCharsForMantissaWithoutSign > 1)
    || (mode == Preferences::PrintFloatMode::Decimal
        && numberOfCharsForMantissaWithoutSign > exponentInBase10 +1);
  if (decimalMarker) {
    assert(UTF8Decoder::CharSizeOfCodePoint('.') == 1);
    numberOfCharsForMantissaWithoutSign++;
  }

  /* Find the position of the decimal marker position */
  int decimalMarkerPosition = (exponentInBase10 < 0 || mode == Preferences::PrintFloatMode::Scientific) ? 1 : exponentInBase10+1;
  decimalMarkerPosition = f < 0 ? decimalMarkerPosition+1 : decimalMarkerPosition;
  assert(UTF8Decoder::CharSizeOfCodePoint('-') == 1);

  /* Part III: Exponent */

  int numberOfCharExponent = exponentInBase10 != 0 ? std::log10(std::fabs((T)exponentInBase10)) + 1 : 1;
  if (exponentInBase10 < 0){
    // If the exponent is < 0, we need a additional char for the sign
    numberOfCharExponent++;
    assert(UTF8Decoder::CharSizeOfCodePoint('-') == 1);
  }

  /* Part III: print mantissa*10^exponent */
  int numberOfCharsForMantissaWithSign = f >= 0 ? numberOfCharsForMantissaWithoutSign : numberOfCharsForMantissaWithoutSign + 1;
  assert(UTF8Decoder::CharSizeOfCodePoint('-') == 1);
  // Print mantissa
  if (numberOfCharsForMantissaWithSign >= bufferSize) {
    /* Exception 3: if we are about to overflow the buffer, we escape by
     * returning a big int. This will be caught by 'convertFloatToText' which
     * will force displayMode to Scientific. */
    assert(mode == Preferences::PrintFloatMode::Decimal);
    return INT_MAX;
  }
  assert(numberOfCharsForMantissaWithSign < bufferSize);
  PrintLongWithDecimalMarker(buffer, numberOfCharsForMantissaWithSign, dividend, decimalMarkerPosition);
  if (mode == Preferences::PrintFloatMode::Decimal || exponentInBase10 == 0) {
    buffer[numberOfCharsForMantissaWithSign] = 0;
    return numberOfCharsForMantissaWithSign;
  }
  // Print exponent
  assert(numberOfCharsForMantissaWithSign < bufferSize);
  int currentNumberOfChar = numberOfCharsForMantissaWithSign;
  currentNumberOfChar+= SerializationHelper::CodePoint(buffer + currentNumberOfChar, bufferSize - currentNumberOfChar, UCodePointLatinLetterSmallCapitalE);
  assert(numberOfCharExponent+currentNumberOfChar < bufferSize);
  dividend = Long(exponentInBase10); // reuse dividend as it is not needed anymore
  PrintLongWithDecimalMarker(buffer + currentNumberOfChar, numberOfCharExponent, dividend, -1);
  buffer[currentNumberOfChar + numberOfCharExponent] = 0;
  return (currentNumberOfChar + numberOfCharExponent);
}

template int PrintFloat::convertFloatToText<float>(float, char*, int, int, Preferences::Preferences::PrintFloatMode, bool);
template int PrintFloat::convertFloatToText<double>(double, char*, int, int, Preferences::Preferences::PrintFloatMode, bool);

}
