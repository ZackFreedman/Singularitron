#ifndef SCROLLABLELIST_H
#define SCROLLABLELIST_H

#include "bufferedVfd.h"

#define maxListItemCount 30

class ScrollableList {
  private:
    char items[maxListItemCount][19] = {"<< GO BACK"}; // Cancel item is always part of the list, and always first item
    int numberOfItems = 1;
    int selectedItem = 0;
    int topLineItem = 0;
    byte caratPosition = 0;

  public:
    ScrollableList();
    ~ScrollableList() {}

    void reset();
    int append(char * item);
    int control(int knobDelta, bool knobClicked); // Returns position of selected item if knob is clicked, else returns -1
    void scrollToTop();
    void render(BufferedVfd * display);
};

ScrollableList::ScrollableList() {
  reset();
}

void ScrollableList::reset() {
  numberOfItems = 1;
  scrollToTop();
}

int ScrollableList::append(char * item) {
  if (numberOfItems >= maxListItemCount) return 0xFF;

  int output = numberOfItems;
  strcpy(items[numberOfItems], item);
  numberOfItems++;
  return output;
}

int ScrollableList::control(int knobDelta, bool knobClicked) {
  int lastSelectedItem = selectedItem;
  selectedItem = constrain(selectedItem + knobDelta, 0, numberOfItems - 1);

  if (selectedItem == 0) {
    caratPosition = 0;
    topLineItem = 0;
  }
  else if (selectedItem == numberOfItems - 1) {
    caratPosition = min(numberOfItems - 1, 3);
    topLineItem = numberOfItems - 4;
  }
  else if (selectedItem < lastSelectedItem) {
    if (caratPosition == 1) topLineItem += selectedItem - lastSelectedItem; // If carat is already in place, scroll list around it
    else {
      topLineItem += selectedItem - lastSelectedItem + (caratPosition - 1);
      caratPosition = max(1, caratPosition + (selectedItem - lastSelectedItem));
    }
  }
  else if (selectedItem > lastSelectedItem) {
    if (caratPosition == 2) topLineItem += selectedItem - lastSelectedItem; // If carat is already in place, scroll list around it
    else {
      topLineItem += selectedItem - lastSelectedItem + (caratPosition - 2);
      caratPosition = min(2, caratPosition + (selectedItem - lastSelectedItem));
    }
  }

  topLineItem = constrain(topLineItem, 0, max(0, numberOfItems - 4));

  if (selectedItem != lastSelectedItem) {
    debug_print("Selected: ");
    debug_print(selectedItem);
    debug_print(" Top: ");
    debug_print(topLineItem);
    debug_print(" Carat: ");
    debug_println(caratPosition);
  }

  if (knobClicked && numberOfItems > 0) return selectedItem;
  else return -1;
}

void ScrollableList::scrollToTop() {
  selectedItem = 0;
  topLineItem = 0;
  caratPosition = 0;
}

void ScrollableList::render(BufferedVfd * display) {
  if (numberOfItems <= 0 || selectedItem < 0) {
    display->bufferedPrint("No items in the list!", 0, 0);
    return;
  }

  for (byte i = 0; i < min(numberOfItems, 4); i++)
    display->bufferedPrint(items[topLineItem + i], i, 1);

  display->bufferedPrint('\x1d', caratPosition, 0); // Right-pointing carat
}

#endif
