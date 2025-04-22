#include <unity.h>
#include "../lib/painlessmesh/src/painlessmesh/pear.hpp"
#include "../lib/painlessmesh/src/painlessmesh/protocol.hpp"

void processReceivedData_unseenPearNodeTree_mapWithPearNodeTree(void){
      painlessmesh::Pear pear;
      auto node1 = painlessmesh::protocol::NodeTree(1, false);
      auto node2 = painlessmesh::protocol::NodeTree(2, false);

      node1.subs.push_back(node2);

      JsonDocument doc;
      doc["periodTx"] = 123;
      doc["periodRx"] = 50;
      JsonArray array = doc["parentCandidates"].to<JsonArray>();
      array.add(node2.nodeId);

      TEST_ASSERT_EQUAL(0, pear.pearNodeTreeMap.count(node1.nodeId));

      pear.processReceivedData(doc, node1);

      TEST_ASSERT_EQUAL(1, pear.pearNodeTreeMap.count(node1.nodeId));
      TEST_ASSERT_EQUAL_INT(123, pear.pearNodeTreeMap[node1.nodeId].periodTx);
      TEST_ASSERT_EQUAL_INT(50, pear.pearNodeTreeMap[node1.nodeId].periodRx);
      TEST_ASSERT_EQUAL_INT(1, pear.pearNodeTreeMap[node1.nodeId].parentCandidates.size());
}

void deviceExceedsThreshold_deviceExceedingThreshold_true(void){
  painlessmesh::Pear pear;
  auto nodeTree = painlessmesh::protocol::NodeTree(1, false);
  auto pearNodeTree = painlessmesh::PearNodeTree(nodeTree);
  pearNodeTree.periodTx = 3000;
  pearNodeTree.periodRx = 3000;

  TEST_ASSERT_TRUE(pear.deviceExceedsThreshold(pearNodeTree));
}

void deviceExceedsThreshold_deviceNotExceedingThreshold_false(void){
  painlessmesh::Pear pear;
  auto nodeTree = painlessmesh::protocol::NodeTree(1, false);
  auto pearNodeTree = painlessmesh::PearNodeTree(nodeTree);
  pearNodeTree.periodTx = 30;
  pearNodeTree.periodRx = 20;

  TEST_ASSERT_FALSE(pear.deviceExceedsThreshold(pearNodeTree));
}

void deviceExceedsLimit_deviceInMapExceedingLimit_true(void){
  painlessmesh::Pear pear;
  auto nodeTree = painlessmesh::protocol::NodeTree(1, false);
  auto pearNodeTree = painlessmesh::PearNodeTree(nodeTree);

  pear.pearNodeTreeMap.insert({pearNodeTree.nodeId, pearNodeTree});

  TEST_ASSERT_TRUE(pear.deviceExceedsLimit(pearNodeTree.nodeId));
  TEST_ASSERT_EQUAL_INT(0, pear.noOfVerifiedDevices);
}

void deviceExceedsLimit_deviceNotInMap_false(void){
  painlessmesh::Pear pear;
  auto nodeTree = painlessmesh::protocol::NodeTree(1, false);
  auto pearNodeTree = painlessmesh::PearNodeTree(nodeTree);

  TEST_ASSERT_FALSE(pear.deviceExceedsLimit(pearNodeTree.nodeId));
  TEST_ASSERT_EQUAL_INT(0, pear.noOfVerifiedDevices);
}


void run_parentCandidateExceedsLimit_reroutesIsEmpty(void){
    painlessmesh::Pear pear;
    auto node1 = painlessmesh::protocol::NodeTree(1, true);
    auto node2 = painlessmesh::protocol::NodeTree(2, false);
    auto node3 = painlessmesh::protocol::NodeTree(3, false);
    auto node4 = painlessmesh::protocol::NodeTree(4, false);
    auto node5 = painlessmesh::protocol::NodeTree(5, false);
    auto node6 = painlessmesh::protocol::NodeTree(6, false);

    node1.subs.push_back(node2);
    node1.subs.push_back(node3);
    node3.subs.push_back(node5);
    node3.subs.push_back(node6);
    node2.subs.push_back(node4);

    JsonDocument docNode4;
    docNode4["periodTx"] = 200;
    docNode4["periodRx"] = 125;
    JsonArray array2 = docNode4["parentCandidates"].to<JsonArray>();
    array2.add(node3.nodeId);

    JsonDocument docNode3;
    docNode3["periodTx"] = 400;
    docNode3["periodRx"] = 300;

    pear.processReceivedData(docNode3, node3);
    pear.processReceivedData(docNode4, node4);

    pear.run(node1);

    TEST_ASSERT_EQUAL_INT(0, pear.reroutes.count(node4.nodeId));
}

void updateParent_nodeWithValidParentCandidates_reroutesContainsReroute(void){
  painlessmesh::Pear pear;
  auto node1 = painlessmesh::protocol::NodeTree(1, true);
  auto sub1 = painlessmesh::protocol::NodeTree(2, false);
  auto sub2 = painlessmesh::protocol::NodeTree(3, false);
  auto sub3 = painlessmesh::protocol::NodeTree(4, false);
  node1.subs.push_back(sub1);
  node1.subs.push_back(sub2);
  node1.subs.push_back(sub3);
  auto pearNodeTree = painlessmesh::PearNodeTree(node1);
  auto pCandidate = painlessmesh::PearNodeTree(node1);
  std::list<painlessmesh::PearNodeTree> parentCandidates;
  parentCandidates.push_back(pCandidate);

  auto subPear1 = painlessmesh::PearNodeTree(sub1, 50, 100, parentCandidates);
  pear.pearNodeTreeMap.insert({subPear1.nodeId, subPear1});

  pear.updateParent(pearNodeTree);

  TEST_ASSERT_EQUAL_INT(1, pear.reroutes.count(subPear1.nodeId));
}

void updateParent_nodeWithInvalidParentCandidates_reroutesContainsNoRoutes(void){
  painlessmesh::Pear pear;
  auto node1 = painlessmesh::protocol::NodeTree(1, true);
  auto sub1 = painlessmesh::protocol::NodeTree(2, false);
  auto sub2 = painlessmesh::protocol::NodeTree(3, false);
  auto sub3 = painlessmesh::protocol::NodeTree(4, false);
  node1.subs.push_back(sub1);
  node1.subs.push_back(sub2);
  node1.subs.push_back(sub3);
  auto pearNodeTree = painlessmesh::PearNodeTree(node1);
  auto pCandidate = painlessmesh::PearNodeTree(node1);
  pCandidate.periodTx = 1000;
  pCandidate.periodRx = 1000;
  std::list<painlessmesh::PearNodeTree> parentCandidates;
  parentCandidates.push_back(pCandidate);

  auto subPear1 = painlessmesh::PearNodeTree(sub1, 2000, 1000, parentCandidates);
  pear.pearNodeTreeMap.insert({subPear1.nodeId, subPear1});

  pear.updateParent(pearNodeTree);

  TEST_ASSERT_EQUAL_INT(0, pear.reroutes.count(subPear1.nodeId));
}

void getAllDevicesBreadthFirst_rootNodeTree_listOfPearNodesBreadthFirst(void){
  painlessmesh::Pear pear;
  using NodeTree = painlessmesh::protocol::NodeTree;

  // Create sub-sub-nodes
  auto subSub1 = NodeTree(4, false);
  auto subSub2 = NodeTree(5, false);

  // Attach sub-sub-nodes to sub-nodes
  auto rootSub1 = NodeTree(2, false);
  rootSub1.subs.push_back(subSub1);

  auto rootSub2 = NodeTree(3, false);
  rootSub2.subs.push_back(subSub2);

  // Create the root and attach sub-nodes
  auto rootNode = NodeTree(1, true);
  rootNode.subs.push_back(rootSub1);
  rootNode.subs.push_back(rootSub2);

  auto list = pear.getAllDevicesBreadthFirst(rootNode);

  std::list<painlessmesh::PearNodeTree> expectedList;
  expectedList.push_back(painlessmesh::PearNodeTree(rootNode));
  expectedList.push_back(painlessmesh::PearNodeTree(rootSub1));
  expectedList.push_back(painlessmesh::PearNodeTree(rootSub2));
  expectedList.push_back(painlessmesh::PearNodeTree(subSub1));
  expectedList.push_back(painlessmesh::PearNodeTree(subSub2));

  // Check sizes
  TEST_ASSERT_EQUAL_UINT32(expectedList.size(), list.size());

  // Check contents
  auto it1 = expectedList.begin();
  auto it2 = list.begin();
  while (it1 != expectedList.end() && it2 != list.end()) {
    TEST_ASSERT_TRUE(*it1 == *it2); // Assumes operator== is defined for PearNodeTree
    ++it1;
    ++it2;
  }
}