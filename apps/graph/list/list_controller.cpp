#include "list_controller.h"
#include "../app.h"
#include <apps/i18n.h>
#include <assert.h>
#include <escher/metric.h>
#include <poincare/code_point_layout.h>

using namespace Shared;
using namespace Escher;

namespace Graph {

ListController::ListController(Responder * parentResponder, ButtonRowController * header, ButtonRowController * footer) :
  Shared::FunctionListController(parentResponder, header, footer, I18n::Message::AddFunction),
  Shared::InputEventHandlerDelegate(),
  m_selectableTableView(this, this, this, this),
  m_parameterController(this, I18n::Message::FunctionColor, I18n::Message::DeleteFunction, this),
  m_modelsParameterController(this, nullptr, this),
  m_modelsStackController(nullptr, &m_modelsParameterController),
  m_functionToolbox(),
  m_parameterColumnSelected(false)
{
  m_selectableTableView.setMargins(0);
  m_selectableTableView.setVerticalCellOverlap(0);
}

/* TableViewDataSource */

KDCoordinate ListController::cellWidth() {
  return selectableTableView()->bounds().width();
}

int ListController::typeAtIndex(int index) {
  if (isAddEmptyRow(index)) {
    return 1;
  }
  return 0;
}

HighlightCell * ListController::reusableCell(int index, int type) {
  assert(index >= 0 && index < maxNumberOfDisplayableRows());
  if (type == 0) {
    return functionCells(index);
  }
  assert(type == 1);
  return &(m_addNewModel);
}

int ListController::reusableCellCount(int type) {
  if (type == 3) {
    return 2;
  }
  if (type > 3) {
    return 1;
  }
  return maxNumberOfDisplayableRows();
}

/* ViewController */

const char * ListController::title() {
  return I18n::translate(I18n::Message::FunctionTab);
}

bool layoutRepresentsAnEquality(Poincare::Layout l) {
  Poincare::Layout match = l.recursivelyMatches(
      [](Poincare::Layout layout) {
      constexpr size_t numberOfSymbols = 5;
      CodePoint symbols[numberOfSymbols] = { '=', '<', '>', UCodePointInferiorEqual, UCodePointSuperiorEqual};
      for (size_t i = 0; i < numberOfSymbols; i++) {
        if (layout.type() == Poincare::LayoutNode::Type::CodePointLayout && static_cast<Poincare::CodePointLayout &>(layout).codePoint() == symbols[i]) {
          return true;
        }
      }
      return false;
    });
  return !match.isUninitialized();
}

bool layoutRepresentsPolarFunction(Poincare::Layout l) {
  Poincare::Layout match = l.recursivelyMatches(
    [](Poincare::Layout layout) {
      return layout.type() == Poincare::LayoutNode::Type::CodePointLayout && static_cast<Poincare::CodePointLayout &>(layout).codePoint() == UCodePointGreekSmallLetterTheta;
    });
  return !match.isUninitialized();
}

bool ListController::layoutFieldDidReceiveEvent(LayoutField * layoutField, Ion::Events::Event event) {
  m_parameterColumnSelected = false;
  if (layoutField->isEditing() && layoutField->shouldFinishEditing(event)) {
    if (!layoutRepresentsAnEquality(layoutField->layout())) {
      layoutField->putCursorLeftOfLayout();
      if (layoutRepresentsPolarFunction(layoutField->layout())) {
        // Insert "f(θ)=" or, if "f" is taken, another default function name.
        constexpr int bufferSize = 10;
        char buffer[bufferSize];
        int length = m_modelsParameterController.defaultName(buffer, bufferSize - 6);
        buffer[length++] = '(';
        length += UTF8Decoder::CodePointToChars(UCodePointGreekSmallLetterTheta, buffer + length, bufferSize - length);
        assert(bufferSize >= length + 3);
        buffer[length++] = ')';
        buffer[length++] = '=';
        buffer[length++] = 0;
        layoutField->handleEventWithText(buffer);
      } else {
        layoutField->handleEventWithText("y=");
      }
    }
  }
  return Shared::LayoutFieldDelegate::layoutFieldDidReceiveEvent(layoutField, event);
}

/* Responder */

void ListController::didBecomeFirstResponder() {
  if (selectedRow() == -1) {
    selectCellAtLocation(0, 0);
  } else {
    selectCellAtLocation(selectedColumn(), selectedRow());
  }
  if (selectedRow() >= numberOfRows()) {
    selectCellAtLocation(selectedColumn(), numberOfRows()-1);
  }
  footer()->setSelectedButton(-1);
  Container::activeApp()->setFirstResponder(selectableTableView());
}

bool ListController::handleEvent(Ion::Events::Event event) {
  // Here we handle an additional parameter column, within FunctionCell's button
  if (selectedRow() >= 0 && selectedRow() <= numberOfRows() && !isAddEmptyRow(selectedRow())) {
    // Selected row is a function cell
    if (m_parameterColumnSelected) {
      // Parameter column is selected
      if (event == Ion::Events::OK || event == Ion::Events::EXE) {
        // Open function parameter menu
        configureFunction(modelStore()->recordAtIndex(modelIndexForRow(selectedRow())));
        return true;
      }
      if (event == Ion::Events::Left) {
        // Leave parameter column
        m_parameterColumnSelected = false;
        selectableTableView()->reloadData();
        return true;
      }
    } else if (event == Ion::Events::Right) {
      // Enter parameter column
      m_parameterColumnSelected = true;
      selectableTableView()->reloadData();
      return true;
    }
  }
  if (event == Ion::Events::Up) {
    if (selectedRow() == -1) {
      footer()->setSelectedButton(-1);
      selectableTableView()->selectCellAtLocation(0, numberOfRows()-1);
      Container::activeApp()->setFirstResponder(selectableTableView());
      return true;
    }
    selectableTableView()->deselectTable();
    assert(selectedRow() == -1);
    Container::activeApp()->setFirstResponder(tabController());
    return true;
  }
  if (selectedRow() < 0) {
    return false;
  }
  if (event == Ion::Events::Down) {
    selectableTableView()->deselectTable();
    footer()->setSelectedButton(0);
    return true;
  }
  return handleEventOnExpression(event);
}

Shared::ListParameterController * ListController::parameterController() {
  return &m_parameterController;
}

int ListController::maxNumberOfDisplayableRows() {
  return k_maxNumberOfDisplayableRows;
}

HighlightCell * ListController::functionCells(int index) {
  assert(index >= 0 && index < k_maxNumberOfDisplayableRows);
  return &m_expressionCells[index];
}

void ListController::willDisplayCellForIndex(HighlightCell * cell, int j) {
  assert(cell != nullptr);
  EvenOddCell * evenOddCell = static_cast<EvenOddCell *>(cell);
  evenOddCell->setEven(j%2 == 0);
  evenOddCell->setHighlighted(j == selectedRow());
  if (isAddEmptyRow(j)) {
    evenOddCell->reloadCell();
    return;
  }
  assert(j >= 0 && j < modelStore()->numberOfModels());
  FunctionCell * functionCell = static_cast<FunctionCell *>(cell);
  ExpiringPointer<ContinuousFunction> f = modelStore()->modelForRecord(modelStore()->recordAtIndex(j));
  functionCell->setLayout(f->layout());
  functionCell->setMessage(f->plotTypeMessage());
  KDColor functionColor = f->isActive() ? f->color() : Palette::GrayDark;
  functionCell->setColor(functionColor);
  KDColor textColor = f->isActive() ? KDColorBlack : Palette::GrayDark;
  functionCell->setTextColor(textColor);
  functionCell->setParameterSelected(m_parameterColumnSelected);
  functionCell->reloadCell();
}

Toolbox * ListController::toolboxForInputEventHandler(InputEventHandler * textInput) {
  // Set sender
  m_functionToolbox.setSender(textInput);
  return &m_functionToolbox;
}

void ListController::addModel() {
  Container::activeApp()->displayModalViewController(&m_modelsStackController, 0.f, 0.f, Metric::PopUpTopMargin, Metric::PopUpRightMargin, 0, Metric::PopUpLeftMargin);
}

ContinuousFunctionStore * ListController::modelStore() {
  return App::app()->functionStore();
}

}
