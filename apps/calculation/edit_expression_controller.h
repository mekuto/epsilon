#ifndef CALCULATION_EDIT_EXPRESSION_CONTROLLER_H
#define CALCULATION_EDIT_EXPRESSION_CONTROLLER_H

#include <escher.h>
#include "editable_expression_view.h"
#include "../shared/text_field_delegate.h"
#include "../shared/expression_layout_field_delegate.h"
#include "history_controller.h"
#include "calculation_store.h"

namespace Calculation {
class HistoryController;

/* TODO: implement a split view */
class EditExpressionController : public DynamicViewController, public Shared::TextFieldDelegate, public Shared::ExpressionLayoutFieldDelegate {
public:
  EditExpressionController(Responder * parentResponder, HistoryController * historyController, CalculationStore * calculationStore);
  void didBecomeFirstResponder() override;
  void viewDidDisappear() override;
  bool handleEvent(Ion::Events::Event event) override;
  const char * textBody();
  void insertTextBody(const char * text);

  /* TextFieldDelegate */
  bool textFieldDidReceiveEvent(::TextField * textField, Ion::Events::Event event) override;
  bool textFieldDidFinishEditing(::TextField * textField, const char * text, Ion::Events::Event event) override;
  bool textFieldDidAbortEditing(::TextField * textField) override;

  /* ExpressionLayoutFieldDelegate */
  bool expressionLayoutFieldDidReceiveEvent(::ExpressionLayoutField * expressionLayoutField, Ion::Events::Event event) override;
  bool expressionLayoutFieldDidFinishEditing(::ExpressionLayoutField * expressionLayoutField, const char * text, Ion::Events::Event event) override;
  bool expressionLayoutFieldDidAbortEditing(::ExpressionLayoutField * expressionLayoutField) override;
  void expressionLayoutFieldDidChangeSize(::ExpressionLayoutField * expressionLayoutField) override;

private:
  class ContentView : public View {
  public:
    ContentView(Responder * parentResponder, TableView * subview, TextFieldDelegate * textFieldDelegate, ExpressionLayoutFieldDelegate * expressionLayoutFieldDelegate);
    void reload();
    TableView * mainView() { return m_mainView; }
    EditableExpressionView * editableExpressionView() { return &m_editableExpressionView; }
    /* View */
    int numberOfSubviews() const override { return 2; }
    View * subviewAtIndex(int index) override;
    void layoutSubviews() override;
  private:
    TableView * m_mainView;
    EditableExpressionView m_editableExpressionView;
  };
  View * loadView() override;
  void unloadView(View * view) override;
  void reloadView();
  bool inputViewDidReceiveEvent(Ion::Events::Event event);
  bool inputViewDidFinishEditing(const char * text, Ion::Events::Event event);
  bool inputViewDidAbortEditing(const char * text);
  Shared::TextFieldDelegateApp * textFieldDelegateApp() override;
  char m_cacheBuffer[TextField::maxBufferSize()];
  Shared::EditableExpressionViewDelegateApp * editableExpressionViewDelegateApp() override;
  Poincare::ExpressionLayout * expressionLayout();
  HistoryController * m_historyController;
  CalculationStore * m_calculationStore;
};

}

#endif
