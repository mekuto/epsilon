#ifndef PROBABILITY_MODELS_STATISTIC_TWO_MEANS_T_STATISTIC_H
#define PROBABILITY_MODELS_STATISTIC_TWO_MEANS_T_STATISTIC_H

#include "t_statistic.h"

namespace Probability {

class TwoMeansTStatistic : public TStatistic {
public:
  void init(Data::SubApp subapp) override;
  bool isValidParamAtIndex(int i, float p) override;
  void setParamAtIndex(int index, float p) override;

  void computeTest() override;
  void computeInterval() override;

  // Confidence interval
  const char * estimateSymbol() override { return "x̅1-x̅2"; };
  Poincare::Layout estimateLayout() override;
  I18n::Message estimateDescription() override { return I18n::Message::SampleTwoMeans; };

protected:
  // Parameters
  constexpr static int k_numberOfParams = 6;
  int numberOfStatisticParameters() const override { return k_numberOfParams; }
  enum ParamsOrder { X1, S1, N1, X2, S2, N2 };
  ParameterRepresentation paramRepresentationAtIndex(int i) const override;
  float * paramArray() override { return m_params; }
  bool validateInputs() override;

private:
  float x1() { return m_params[ParamsOrder::X1]; }
  float n1() { return m_params[ParamsOrder::N1]; }
  float s1() { return m_params[ParamsOrder::S1]; }
  float x2() { return m_params[ParamsOrder::X2]; }
  float n2() { return m_params[ParamsOrder::N2]; }
  float s2() { return m_params[ParamsOrder::S2]; }

  float _xEstimate(float meanSample1, float meanSample2);
  float computeT(float deltaMean,
                 float meanSample1,
                 float n1,
                 float sigma1,
                 float meanSample2,
                 float n2,
                 float sigma2);
  virtual float computeDegreesOfFreedom(float s1, float n1, float s2, float n2);
  virtual float computeStandardError(float s1, float n1, float s2, float n2);

  float m_params[k_numberOfParams];
};

}  // namespace Probability

#endif /* PROBABILITY_MODELS_STATISTIC_TWO_MEANS_T_STATISTIC_H */
