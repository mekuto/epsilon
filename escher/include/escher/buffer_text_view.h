#ifndef ESCHER_BUFFER_TEXT_VIEW_H
#define ESCHER_BUFFER_TEXT_VIEW_H

#include <escher/text_view.h>

namespace Escher {

class BufferTextView : public TextView {
public:
  static constexpr int k_maxNumberOfChar = 256;
  BufferTextView(const KDFont * font = KDFont::LargeFont, float horizontalAlignment = 0.5f, float verticalAlignment = 0.5f,
    KDColor textColor = KDColorBlack, KDColor backgroundColor = KDColorWhite, int maxDisplayedTextLength = k_maxNumberOfChar-1);
  void setText(const char * text) override;
  const char * text() const override;
  void appendText(const char * text);
private:
  char m_buffer[k_maxNumberOfChar];
  int m_maxDisplayedTextLength;
};

}

#endif
