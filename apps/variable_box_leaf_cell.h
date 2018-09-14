#ifndef APPS_VARIABLE_BOX_LEAF_CELL_H
#define APPS_VARIABLE_BOX_LEAF_CELL_H

#include <escher.h>
#include <poincare.h>

class VariableBoxLeafCell : public HighlightCell {
public:
  VariableBoxLeafCell();
  void displayExpression(bool displayExpression);
  void reloadCell() override;
  void setLabel(const char * text);
  void setSubtitle(const char * text);
  void setLayoutReference(Poincare::LayoutReference layoutRef);
  void drawRect(KDContext * ctx, KDRect rect) const override;
  const char * text() const override {
    return m_labelView.text();
  }
private:
  constexpr static KDCoordinate k_separatorThickness = 1;
  constexpr static KDCoordinate k_widthMargin = 10;
  int numberOfSubviews() const override;
  View * subviewAtIndex(int index) override;
  void layoutSubviews() override;
  BufferTextView m_labelView;
  BufferTextView m_subtitleView;
  ExpressionView m_expressionView;
  bool m_displayExpression;
};

#endif
