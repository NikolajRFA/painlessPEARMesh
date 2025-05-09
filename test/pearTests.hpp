#include <unity.h>
#include "../lib/painlessmesh/src/painlessmesh/pear.hpp"
#include "../lib/painlessmesh/src/painlessmesh/protocol.hpp"

void processReceivedData_unseenPearNodeTree_mapWithPearNodeTree(void){
      using namespace painlessmesh;
      Pear::reset();
      Pear& pear = Pear::getInstance();
      auto node1 = protocol::NodeTree(1, false);
      auto node2 = protocol::NodeTree(2, false);

      node1.subs.push_back(node2);

      JsonDocument doc;
      doc["txPeriod"] = 123;
      doc["rxPeriod"] = 50;
      JsonArray array = doc["parentCandidates"].to<JsonArray>();
      array.add(node2.nodeId);

      TEST_ASSERT_EQUAL(0, pear.pearNodeTreeMap.count(node1.nodeId));

      pear.processReceivedData(doc, std::make_shared<protocol::NodeTree>(node1));

      auto pearNode = pear.pearNodeTreeMap[node1.nodeId];

      TEST_ASSERT_EQUAL(1, pear.pearNodeTreeMap.count(node1.nodeId));
      TEST_ASSERT_EQUAL_INT(123, pearNode->periodTx);
      TEST_ASSERT_EQUAL_INT(50, pearNode->periodRx);
      TEST_ASSERT_EQUAL_INT(1, pearNode->parentCandidates.size());
}

void deviceExceedsThreshold_deviceExceedingThreshold_true(void){
  using namespace painlessmesh;
  Pear::reset();
  Pear& pear = painlessmesh::Pear::getInstance();
  auto nodeTree = std::make_shared<protocol::NodeTree>(protocol::NodeTree(1, false));
  auto pearNodeTree = std::make_shared<PearNodeTree>(painlessmesh::PearNodeTree(nodeTree));
  pearNodeTree->periodTx = 3000;
  pearNodeTree->periodRx = 3000;

  TEST_ASSERT_TRUE(pear.deviceExceedsThreshold(pearNodeTree));
}

void deviceExceedsThreshold_deviceNotExceedingThreshold_false(void){
  using namespace painlessmesh;
  Pear::reset();
  Pear& pear = Pear::getInstance();
  auto nodeTree = std::make_shared<protocol::NodeTree>(protocol::NodeTree(1, false));
  auto pearNodeTree = std::make_shared<PearNodeTree>(PearNodeTree(nodeTree));
  pearNodeTree->periodTx = 30;
  pearNodeTree->periodRx = 20;

  TEST_ASSERT_FALSE(pear.deviceExceedsThreshold(pearNodeTree));
}

void deviceExceedsLimit_deviceInMapExceedingLimit_true(void){
  using namespace painlessmesh;
  Pear::reset();
  Pear& pear = Pear::getInstance();
  auto nodeTree = painlessmesh::protocol::NodeTree(1, false);
  auto pearNodeTree = std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<protocol::NodeTree>(nodeTree)));

  pear.pearNodeTreeMap.insert({pearNodeTree->nodeId, pearNodeTree});

  TEST_ASSERT_TRUE(pear.deviceExceedsLimit(pearNodeTree->nodeId));
  TEST_ASSERT_EQUAL_INT(0, pear.noOfVerifiedDevices);
}

void deviceExceedsLimit_deviceNotInMap_false(void){
  using namespace painlessmesh;
  Pear::reset();
  Pear& pear = Pear::getInstance();
  auto nodeTree = std::make_shared<protocol::NodeTree>(protocol::NodeTree(1, false));
  auto pearNodeTree = std::make_shared<PearNodeTree>(PearNodeTree(nodeTree));

  TEST_ASSERT_FALSE(pear.deviceExceedsLimit(pearNodeTree->nodeId));
  TEST_ASSERT_EQUAL_INT(0, pear.noOfVerifiedDevices);
}


void run_parentCandidateExceedsLimit_reroutesIsEmpty(void){
    using namespace painlessmesh;
    Pear::reset();
    Pear& pear = Pear::getInstance();
    auto node1 = painlessmesh::protocol::NodeTree(1, true);
    auto node2 = painlessmesh::protocol::NodeTree(2, false);
    auto node3 = painlessmesh::protocol::NodeTree(3, false);
    auto node4 = painlessmesh::protocol::NodeTree(4, false);
    auto node5 = painlessmesh::protocol::NodeTree(5, false);
    auto node6 = painlessmesh::protocol::NodeTree(6, false);

    node3.subs.push_back(node5);
    node3.subs.push_back(node6);
    node2.subs.push_back(node4);
    node1.subs.push_back(node2);
    node1.subs.push_back(node3);

    JsonDocument docNode4;
    docNode4["txPeriod"] = 100; // Exceeds base value of 38, sub(s) should be rerouted
    docNode4["rxPeriod"] = 100;
    JsonArray array2 = docNode4["parentCandidates"].to<JsonArray>();
    array2.add(node3.nodeId);

    JsonDocument docNode3;
    docNode3["txPeriod"] = 400; // Exceeds base value of 38, therefore we cannot reroute to this node
    docNode3["rxPeriod"] = 300;

    JsonDocument docNode5;
    docNode5["txPeriod"] = 200; // sub of 3, should try to reroute to 4, but 4 exceeds
    docNode5["rxPeriod"] = 200;
    JsonArray array3 = docNode5["ParentCandidates"].to<JsonArray>();
    array3.add(node4.nodeId);

    pear.processReceivedData(docNode4, std::make_shared<protocol::NodeTree>(node4));
    pear.processReceivedData(docNode3, std::make_shared<protocol::NodeTree>(node3));
    pear.processReceivedData(docNode5, std::make_shared<protocol::NodeTree>(node5));

    pear.run(node1);

    TEST_ASSERT_EQUAL_INT(2, pear.noOfVerifiedDevices);

    TEST_ASSERT_EQUAL_INT(0, pear.reroutes.count(node4.nodeId));
}

void updateParent_nodeWithValidParentCandidates_reroutesContainsReroute(void){
  using namespace painlessmesh;
  Pear::reset();
  Pear& pear = Pear::getInstance();
  using NodeTree = protocol::NodeTree;
  auto node1 = NodeTree(1, true);
  auto sub1 = NodeTree(2, false);
  auto sub2 = NodeTree(3, false);
  auto sub3 = NodeTree(4, false);
  node1.subs.push_back(sub1);
  node1.subs.push_back(sub2);
  node1.subs.push_back(sub3);
  auto pearNodeTree = std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<protocol::NodeTree>(node1)));
  auto pCandidate = PearNodeTree(std::make_shared<protocol::NodeTree>(node1));
  std::list<std::shared_ptr<PearNodeTree>> parentCandidates;
  parentCandidates.push_back(std::make_shared<PearNodeTree>(pCandidate));

  auto subPear1 = std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<protocol::NodeTree>(sub1), 50, 100, parentCandidates));
  pear.pearNodeTreeMap.insert({subPear1->nodeId, subPear1});

  pear.updateParent(pearNodeTree);

  TEST_ASSERT_EQUAL_INT(1, pear.reroutes.count(subPear1->nodeId));
}

void updateParent_nodeWithInvalidParentCandidates_reroutesContainsNoRoutes(void){
  using namespace painlessmesh;
  Pear::reset();
  Pear& pear = Pear::getInstance();
  auto node1 = protocol::NodeTree(1, true);
  auto sub1 = protocol::NodeTree(2, false);
  auto sub2 = protocol::NodeTree(3, false);
  auto sub3 = protocol::NodeTree(4, false);
  node1.subs.push_back(sub1);
  node1.subs.push_back(sub2);
  node1.subs.push_back(sub3);
  auto pearNodeTree = std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<protocol::NodeTree>(node1)));
  auto pCandidate = std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<protocol::NodeTree>(node1)));
  pCandidate->periodTx = 1000;
  pCandidate->periodRx = 1000;
  std::list<std::shared_ptr<PearNodeTree>> parentCandidates;
  parentCandidates.push_back(pCandidate);

  auto subPear1 = std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<protocol::NodeTree>(sub1), 2000, 1000, parentCandidates));
  pear.pearNodeTreeMap.insert({subPear1->nodeId, subPear1});

  pear.updateParent(pearNodeTree);

  TEST_ASSERT_EQUAL_INT(0, pear.reroutes.count(subPear1->nodeId));
}

void getAllDevicesBreadthFirst_rootNodeTree_listOfPearNodesBreadthFirst(void){
  using namespace painlessmesh;
  using NodeTree = protocol::NodeTree;
  Pear::reset();
  Pear& pear = Pear::getInstance();

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

  std::list<std::shared_ptr<PearNodeTree>> expectedList;
  expectedList.push_back(std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<NodeTree>(rootSub1))));
  expectedList.push_back(std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<NodeTree>(rootSub2))));
  expectedList.push_back(std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<NodeTree>(subSub1))));
  expectedList.push_back(std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<NodeTree>(subSub2))));

  // Check sizes
  TEST_ASSERT_EQUAL_UINT32(expectedList.size(), list.size());

  // Check contents
  auto it1 = expectedList.begin();
  auto it2 = list.begin();
  while (it1 != expectedList.end() && it2 != list.end()) {
    TEST_ASSERT_TRUE(**it1 == **it2); // Assumes operator== is defined for PearNodeTree
    ++it1;
    ++it2;
  }
}

void test_run_should_reroute_1_node(void) {
  using namespace painlessmesh;
  Pear::reset();
  Pear& pear = Pear::getInstance();
  auto node1 = painlessmesh::protocol::NodeTree(1, true);
  auto node2 = painlessmesh::protocol::NodeTree(2, false);
  auto node3 = painlessmesh::protocol::NodeTree(3, false);
  auto node4 = painlessmesh::protocol::NodeTree(4, false);
  auto node5 = painlessmesh::protocol::NodeTree(5, false);
  auto node6 = painlessmesh::protocol::NodeTree(6, false);

  node4.subs.push_back(node5);
  node4.subs.push_back(node6);
  node2.subs.push_back(node4);
  node1.subs.push_back(node2);
  node1.subs.push_back(node3);

  JsonDocument docNode4;
  docNode4["txPeriod"] = 100; // Exceeds base value of 38, should be rerouted
  docNode4["rxPeriod"] = 100; // Exceeds base value of 8, should be rerouted


  JsonDocument docNode3;
  docNode3["txPeriod"] = 0; // does not exceed, so we can reroute to this
  docNode3["rxPeriod"] = 0;

  JsonDocument docNode5;
  docNode5["txPeriod"] = 100; // Exceeds base value of 38, should be rerouted
  docNode5["rxPeriod"] = 100; // Exceeds base value of 8, should be rerouted
  const JsonArray parentCandidates = docNode5["parentCandidates"].to<JsonArray>();
  parentCandidates.add(node3.nodeId);

  pear.processReceivedData(docNode4, std::make_shared<protocol::NodeTree>(node4));
  pear.processReceivedData(docNode3, std::make_shared<protocol::NodeTree>(node3));
  pear.processReceivedData(docNode5, std::make_shared<protocol::NodeTree>(node5));

  pear.run(node1);

  TEST_ASSERT_EQUAL_UINT32(4, pear.noOfVerifiedDevices);
  TEST_ASSERT_EQUAL_UINT32(1, pear.reroutes.size());
  TEST_ASSERT_TRUE(pear.reroutes.count(5)); // node 4 rerouted to node 3
}
