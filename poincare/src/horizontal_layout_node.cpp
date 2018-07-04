#include <poincare/horizontal_layout_node.h>
#include <poincare/layout_engine.h>

namespace Poincare {

static inline KDCoordinate maxCoordinate(KDCoordinate c1, KDCoordinate c2) { return c1 > c2 ? c1 : c2; }

void HorizontalLayoutNode::addOrMergeChildAtIndex(LayoutNode * l, int index, bool removeEmptyChildren) {
  if (l->isHorizontal()) {
    mergeChildrenAtIndex(static_cast<HorizontalLayoutNode *>(l), index, removeEmptyChildren);
  } else {
    addChildAtIndex(l, index);
  }
}

void HorizontalLayoutNode::mergeChildrenAtIndex(HorizontalLayoutNode * h, int index, bool removeEmptyChildren) {
  /* Remove any empty child that would be next to the inserted layout.
   * If the layout to insert starts with a vertical offset layout, any empty
   * layout child directly on the left of the inserted layout (if there is one)
   * should not be removed: it will be the base for the VerticalOffsetLayout. */
  bool shouldRemoveOnLeft = h->numberOfChildren() == 0 ? true : !(h->childAtIndex(0)->mustHaveLeftSibling());
  int newIndex = removeEmptyChildBeforeInsertionAtIndex(index, shouldRemoveOnLeft);

  // Merge the horizontal layout
  LayoutRef(this).mergeChildrenAtIndex(LayoutRef(h), newIndex);
}

int HorizontalLayoutNode::writeTextInBuffer(char * buffer, int bufferSize, int numberOfSignificantDigits) const {
  if (numberOfChildren() == 0) {
    if (bufferSize == 0) {
      return -1;
    }
    buffer[0] = 0;
    return 0;
  }
  return LayoutEngine::writeInfixTreeRefTextInBuffer(TreeRef(const_cast<HorizontalLayoutNode *>(this)), buffer, bufferSize, numberOfSignificantDigits, "");
}

void HorizontalLayoutNode::moveCursorLeft(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  if (this == cursor->layoutReference().node()) {
    if (cursor->position() == LayoutCursor::Position::Left) {
      // Case: Left. Ask the parent.
      LayoutNode * parentNode = parent();
      if (parentNode != nullptr) {
        parentNode->moveCursorLeft(cursor, shouldRecomputeLayout);
      }
      return;
    }
    assert(cursor->position() == LayoutCursor::Position::Right);
    /* Case: Right. Go to the last child if there is one, and move Left. Else
     * go Left and ask the parent. */
    int childrenCount = numberOfChildren();
    if (childrenCount >= 1) {
      cursor->setLayoutNode(static_cast<LayoutNode *>(childTreeAtIndex(childrenCount-1)));
    } else {
      cursor->setPosition(LayoutCursor::Position::Left);
    }
    return cursor->moveLeft(shouldRecomputeLayout);
  }

  // Case: The cursor is Left of a child.
  assert(cursor->position() == LayoutCursor::Position::Left);
  int childIndex = indexOfChildByIdentifier(cursor->layoutIdentifier());
  assert(childIndex >= 0);
  if (childIndex == 0) {
    // Case: the child is the leftmost. Ask the parent.
    if (parent()) {
      cursor->setLayoutNode(this);
      return cursor->moveLeft(shouldRecomputeLayout);
    }
    return;
  }
  // Case: the child is not the leftmost. Go to its left sibling and move Left.
  cursor->setLayoutNode(static_cast<LayoutNode *>(childTreeAtIndex(childIndex-1)));
  cursor->setPosition(LayoutCursor::Position::Right);
  cursor->moveLeft(shouldRecomputeLayout);
}

void HorizontalLayoutNode::moveCursorRight(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  //TODO
  LayoutNode * parentNode = parent();
  if (parentNode != nullptr) {
    parentNode->moveCursorRight(cursor, shouldRecomputeLayout);
  }
}

void HorizontalLayoutNode::computeSize() {
  assert(!m_sized);
  KDCoordinate totalWidth = 0;
  KDCoordinate maxUnderBaseline = 0;
  KDCoordinate maxAboveBaseline = 0;
  for (LayoutNode * l : children()) {
    KDSize childSize = l->layoutSize();
    totalWidth += childSize.width();
    maxUnderBaseline = maxCoordinate(maxUnderBaseline, childSize.height() - l->baseline());
    maxAboveBaseline = maxCoordinate(maxAboveBaseline, l->baseline());
  }
  m_frame.setSize(KDSize(totalWidth, maxUnderBaseline + maxAboveBaseline));
  m_sized = true;
}

void HorizontalLayoutNode::computeBaseline() {
  assert(!m_baselined);
  m_baseline = 0;
  for (LayoutNode * l : children()) {
    m_baseline = maxCoordinate(m_baseline, l->baseline());
  }
  m_baselined = true;
}

KDPoint HorizontalLayoutNode::positionOfChild(LayoutNode * l) {
  assert(hasChild(l));
  KDCoordinate x = 0;
  int index = indexOfChild(l);
  assert(index > -1);
  if (index > 0) {
    LayoutNode * previousChild = childAtIndex(index-1);
    x = previousChild->origin().x() + previousChild->layoutSize().width();
  }
  KDCoordinate y = baseline() - l->baseline();
  return KDPoint(x, y);
}

// Private

void HorizontalLayoutNode::privateAddSibling(LayoutCursor * cursor, LayoutNode * sibling, bool moveCursor) {
  int childrenCount = numberOfChildren();
  // Add the "sibling" as a child.
  if (cursor->position() == LayoutCursor::Position::Left) {
    int indexForInsertion = 0;
    /* If the first child is empty, remove it before adding the layout, unless
     * the new sibling needs the empty layout as a base. */
    if (childrenCount > 0 && childAtIndex(0)->isEmpty()) {
      if (sibling->mustHaveLeftSibling()) {
        indexForInsertion = 1;
      } else {
        /* We force the removal of the child even followed by a neighbourg
         * requiring a left sibling as we are about to add a sibling in first
         * position anyway. */
        privateRemoveChildAtIndex(0, true);
      }
    }
    if (moveCursor) {
      if (childrenCount > indexForInsertion) {
        cursor->setLayoutNode(childAtIndex(indexForInsertion));
      } else {
        cursor->setLayoutNode(this);
        cursor->setPosition(LayoutCursor::Position::Right);
      }
    }
    addOrMergeChildAtIndex(sibling, indexForInsertion, false);
    return;
  }
  assert(cursor->position() == LayoutCursor::Position::Right);
  // If the last child is empty, remove it before adding the layout.
  if (childrenCount > 0 && childAtIndex(childrenCount - 1)->isEmpty() && !sibling->mustHaveLeftSibling()) {
    /* Force remove the last child. */
    privateRemoveChildAtIndex(childrenCount - 1, true);
    childrenCount--;
  }
  addOrMergeChildAtIndex(sibling, childrenCount, false);
  if (moveCursor) {
    cursor->setLayoutNode(this);
  }
}

void HorizontalLayoutNode::privateRemoveChildAtIndex(int index, bool forceRemove) {
  /* Remove the child before potentially adding an EmptyLayout. Indeed, adding
   * a new child would remove any EmptyLayout surrounding the new child and in
   * the case the child to be removed was an Empty layout, it would result in
   * removing it twice if we remove it afterwards. */
  removeChild(childAtIndex(index));
  /* If the child to remove is at index 0 and its right sibling must have a left
   * sibling (e.g. it is a VerticalOffsetLayout), replace the child with an
   * EmptyLayout instead of removing it. */

  /*if (!forceRemove && index == 0 && numberOfChildren() > 0 && child(0)->mustHaveLeftSibling()) {
    addChildAtIndex(new EmptyLayout(), 0);
  }*/ //TODO
}

int HorizontalLayoutNode::removeEmptyChildBeforeInsertionAtIndex(int index, bool shouldRemoveOnLeft) {
  int currentNumberOfChildren = numberOfChildren();
  assert(index >= 0 && index <= currentNumberOfChildren);
  int newIndex = index;
  /* If empty, remove the child that would be on the right of the inserted
   * layout. */
  if (newIndex < currentNumberOfChildren) {
    LayoutNode * c = childAtIndex(newIndex);
    if (c->isEmpty()) {
      removeChild(c);
      currentNumberOfChildren--;
    }
  }
  /* If empty, remove the child that would be on the left of the inserted
   * layout. */
  if (shouldRemoveOnLeft && newIndex - 1 >= 0 && newIndex - 1 <= currentNumberOfChildren -1) {
    LayoutNode * c = childAtIndex(newIndex - 1);
    if (c->isEmpty()) {
      removeChild(c);
      newIndex = index - 1;
    }
  }
  return newIndex;
}

}
