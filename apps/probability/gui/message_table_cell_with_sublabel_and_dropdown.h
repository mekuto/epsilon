#ifndef APPS_PROBABILITY_GUI_MESSAGE_TABLE_CELL_WITH_SUBLABEL_AND_DROPDOWN_H
#define APPS_PROBABILITY_GUI_MESSAGE_TABLE_CELL_WITH_SUBLABEL_AND_DROPDOWN_H

#include <escher/container.h>
#include <escher/message_table_cell_with_message.h>
#include <escher/responder.h>

#include "dropdown_view.h"
#include "layout_cell_with_sub_message.h"

namespace Probability {

class LayoutCellWithSublabelAndDropdown : public LayoutCellWithSubMessage,
                                                public Escher::Responder {
public:
  LayoutCellWithSublabelAndDropdown(Escher::Responder * parentResponder,
                                          Escher::ListViewDataSource * listDataSource,
                                          DropdownCallback * callback = nullptr) :
      Responder(parentResponder), m_dropdown(this, listDataSource, callback) {}
  const Escher::View * accessoryView() const override { return &m_dropdown; }
  Dropdown * dropdown() { return &m_dropdown; }
  Escher::Responder * responder() override { return this; }
  void didBecomeFirstResponder() override {
    Escher::Container::activeApp()->setFirstResponder(&m_dropdown);
  }
  bool isSublabelAlignedRight() const override { return false; }
  void reload() {
    m_dropdown.reloadAllCells();
    markRectAsDirty(bounds());
    layoutSubviews();
  }
  void setHighlighted(bool highlight) override {
    m_dropdown.setHighlighted(highlight);
    LayoutCellWithSubMessage::setHighlighted(highlight);
  }

private:
  Dropdown m_dropdown;
};

}  // namespace Probability

#endif /* APPS_PROBABILITY_GUI_MESSAGE_TABLE_CELL_WITH_SUBLABEL_AND_DROPDOWN_H */
