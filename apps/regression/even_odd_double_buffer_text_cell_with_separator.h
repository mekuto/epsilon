#ifndef REGRESSION_EVEN_ODD_DOUBLE_BUFFER_TEXT_CELL_WITH_SEPARATOR_H
#define REGRESSION_EVEN_ODD_DOUBLE_BUFFER_TEXT_CELL_WITH_SEPARATOR_H

#include <escher.h>
#include "even_odd_buffer_text_cell_with_margin.h"

namespace Regression {

class EvenOddDoubleBufferTextCellWithSeparator : public EvenOddCell, public Responder{
public:
  EvenOddDoubleBufferTextCellWithSeparator(Responder * parentResponder = nullptr, float horizontalAlignment = 1.0f, float verticalAlignment = 0.5f);
  const char * firstText();
  const char * secondText();
  void reloadCell() override;
  void setHighlighted(bool highlight) override;
  Responder * responder() override {
    return this;
  }
  void setEven(bool even) override;
  bool firstTextSelected();
  void selectFirstText(bool selectFirstText);
  void setFirstText(const char * textContent);
  void setSecondText(const char * textContent);
  void setTextColor(KDColor textColor);
  void drawRect(KDContext * ctx, KDRect rect) const override;
  int numberOfSubviews() const override;
  View * subviewAtIndex(int index) override;
  void layoutSubviews() override;
  bool handleEvent(Ion::Events::Event event) override;
protected:
  bool m_firstTextSelected;
  EvenOddBufferTextCellWithMargin m_firstBufferTextView;
  EvenOddBufferTextCellWithMargin m_secondBufferTextView;
};

}

#endif
