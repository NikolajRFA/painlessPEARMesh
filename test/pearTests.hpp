#include <unity.h>
#include "../lib/painlessmesh/src/painlessmesh/pear.hpp"
#include "../lib/painlessmesh/src/painlessmesh/protocol.hpp"
void processReceivedData_unseenPearNodeTree_mapWithPearNodeTree(void){
      painlessmesh::Pear pear;
      auto node1 = painlessmesh::protocol::NodeTree(1, false);
      auto node2 = painlessmesh::protocol::NodeTree(2, false);
      auto node3 = painlessmesh::protocol::NodeTree(3, false);

      node1.subs.push_back(node2);
      node2.subs.push_back(node3);

      JsonDocument doc;
      doc["periodTx"] = 123;
      doc["periodRx"] = 50;
      JsonArray array = doc["parentCandidates"].to<JsonArray>();
      array.add(node3.nodeId);

      TEST_ASSERT_EQUAL(0, pear.pearNodeTreeMap.count(node1.nodeId));

      pear.processReceivedData(doc, node1);

      TEST_ASSERT_EQUAL(1, pear.pearNodeTreeMap.count(node1.nodeId));
      TEST_ASSERT_EQUAL_INT(123, pear.pearNodeTreeMap[node1.nodeId].periodTx);
      TEST_ASSERT_EQUAL_INT(50, pear.pearNodeTreeMap[node1.nodeId].periodRx);
      TEST_ASSERT_EQUAL_INT(1, pear.pearNodeTreeMap[node1.nodeId].parentCandidates.size());
}