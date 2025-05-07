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

    node1.subs.push_back(node2);
    node1.subs.push_back(node3);
    node3.subs.push_back(node5);
    node3.subs.push_back(node6);
    node2.subs.push_back(node4);

    JsonDocument docNode4;
    docNode4["txPeriod"] = 200;
    docNode4["rxPeriod"] = 125;
    JsonArray array2 = docNode4["parentCandidates"].to<JsonArray>();
    array2.add(node3.nodeId);

    JsonDocument docNode3;
    docNode3["txPeriod"] = 400;
    docNode3["rxPeriod"] = 300;

    pear.processReceivedData(docNode3, std::make_shared<protocol::NodeTree>(node3));
    pear.processReceivedData(docNode4, std::make_shared<protocol::NodeTree>(node4));

    pear.run(node1);

    TEST_ASSERT_EQUAL_INT(0, pear.noOfVerifiedDevices);

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

void test_run_should_process_multiple_nodes_until_threshold(void) {
  using namespace painlessmesh;
  using protocol::NodeTree;

  Pear::reset();
  Pear& pear = Pear::getInstance();

  // Step 1: Build deepest child first
  protocol::NodeTree child2(3, false);

  // Step 2: Then build its parent and attach child2
  protocol::NodeTree child1(2, false);
  child1.subs.push_back(child2);

  // Step 3: Create another node that doesn't exceed the limit
  protocol::NodeTree child4(4, false);

  // Step 4: Now build the root and attach both child1 and child4
  protocol::NodeTree rootNode(1, true);
  rootNode.subs.push_back(child1);
  rootNode.subs.push_back(child4);

  // Create PearNodes and assign periods
  std::shared_ptr<PearNodeTree> rootPear = std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<NodeTree>(rootNode)));

  std::shared_ptr<PearNodeTree> child1Pear = std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<NodeTree>(child1)));
  child1Pear->periodTx = 999; // exceeds
  child1Pear->periodRx = 999; // exceeds

  std::shared_ptr<PearNodeTree> child2Pear = std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<NodeTree>(child2)));
  child2Pear->periodTx = 25; // does not exceed
  child2Pear->periodRx = 0;  // does not exceed

  std::shared_ptr<PearNodeTree> child4Pear = std::make_shared<PearNodeTree>(PearNodeTree(std::make_shared<NodeTree>(child4)));
  child4Pear->periodTx = 10;  // does not exceed
  child4Pear->periodRx = 0;  // does not exceed

  // Connect PearNodeTree structure (manually mirror NodeTree)
  child1Pear->subs.push_back(child2);

  // Add parent candidates
  child1Pear->parentCandidates.push_back(rootPear);
  child2Pear->parentCandidates.push_back(child4Pear);  // Now has a valid alternate parent
  child4Pear->parentCandidates.push_back(rootPear);

  // Load the map
  pear.pearNodeTreeMap[1] = rootPear;
  pear.pearNodeTreeMap[2] = child1Pear;
  pear.pearNodeTreeMap[3] = child2Pear;
  pear.pearNodeTreeMap[4] = child4Pear;

  // Call run
  pear.run(rootNode);

  // Expectations:
  // - child 1 exceeds threshold → reroute subs
  // - child 2 is sub to child 1 - we want to reroute this node
  // - Child 2 should be rerouted to child 4
  // - Child 3 and 4 are checked using deviceExceedsLimit() - they are within the limit
  // - this means noOfVerifiedDevices is incremented twice
  TEST_ASSERT_EQUAL_UINT32(2, pear.noOfVerifiedDevices);
  TEST_ASSERT_EQUAL_UINT32(1, pear.reroutes.size());
  TEST_ASSERT_TRUE(pear.reroutes.count(3)); // node 3 rerouted to node 4
}
