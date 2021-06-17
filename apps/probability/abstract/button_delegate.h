#ifndef APPS_PROBABILITY_ABSTRACT_BUTTON_DELEGATE_H
#define APPS_PROBABILITY_ABSTRACT_BUTTON_DELEGATE_H

#include <escher/invocation.h>

namespace Probability {

/* Simple Delegate interface to use a button
 * Just need to define buttonAction() */
class ButtonDelegate {
 public:
  virtual void buttonAction() = 0;
  Escher::Invocation buttonActionInvocation() {
    return Escher::Invocation(
        [](void * c, void * s) {
          static_cast<ButtonDelegate *>(c)->buttonAction();
          return true;
        },
        this);
  }
};

}  // namespace Probability

#endif /* APPS_PROBABILITY_ABSTRACT_BUTTON_DELEGATE_H */