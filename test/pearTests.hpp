#include "unity.h"
#include <Arduino.h>
#include "list"
#include "../lib/painlessMesh/src/painlessmesh/pear.hpp"

void subsDescendingTx_listOfSubs_orderedSetOfSubs(void){
  auto sub1 = painlessmesh::PearNodeTree();
  sub1.periodRx = 10;
  auto sub2 = painlessmesh::PearNodeTree();
  sub2.periodRx = 20;
  auto sub3 = painlessmesh::PearNodeTree();
  sub3.periodRx = 30;

  auto orderedNodeTree = painlessmesh::PearNodeTree();
  orderedNodeTree.subsDescendingTx.insert(sub3);
  orderedNodeTree.subsDescendingTx.insert(sub1);
  orderedNodeTree.subsDescendingTx.insert(sub2);

  auto periodTx = 30;
  auto currentSub = orderedNodeTree.subsDescendingTx.begin();
  while (currentSub != orderedNodeTree.subsDescendingTx.end()) {
    TEST_ASSERT_EQUAL_INT(periodTx, currentSub->periodRx);
    currentSub++;
    periodTx -= 10;
  }
}