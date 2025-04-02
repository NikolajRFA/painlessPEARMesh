#include "unity.h"
#include "../lib/painlessMesh/src/painlessmesh/protocol.hpp"
#include "../lib/painlessMesh/src/painlessmesh/layout.hpp"

void getRootNodeId_NodeTreeWithRootNode_1(void)
{
    using namespace painlessmesh::protocol;
    auto n8 = NodeTree(8, false);
    auto n3 = NodeTree(3, false);
    n3.subs.push_back(n8);
    auto n7 = NodeTree(7, false);
    auto n1 = NodeTree(1, true);
    n1.subs.push_back(n7);
    n1.subs.push_back(n3);
    auto n4 = NodeTree(4, false);
    auto n5 = NodeTree(5, false);
    auto n2 = NodeTree(2, false);
    n2.subs.push_back(n1);
    n2.subs.push_back(n4);
    n2.subs.push_back(n5);
    auto n6 = NodeTree(6, false);
    n6.subs.push_back(n2);

    auto rootNodeId = painlessmesh::layout::getRootNodeId(n6);

    TEST_ASSERT_EQUAL(1, rootNodeId);
}

void getRootNodeId_NodeTreeWithoutRootNode_0(void)
{
    using namespace painlessmesh::protocol;
    auto n8 = NodeTree(8, false);
    auto n3 = NodeTree(3, false);
    n3.subs.push_back(n8);
    auto n7 = NodeTree(7, false);
    auto n1 = NodeTree(1, false);
    n1.subs.push_back(n7);
    n1.subs.push_back(n3);
    auto n4 = NodeTree(4, false);
    auto n5 = NodeTree(5, false);
    auto n2 = NodeTree(2, false);
    n2.subs.push_back(n1);
    n2.subs.push_back(n4);
    n2.subs.push_back(n5);
    auto n6 = NodeTree(6, false);
    n6.subs.push_back(n2);

    auto rootNodeId = painlessmesh::layout::getRootNodeId(n6);

    TEST_ASSERT_EQUAL(0, rootNodeId);
}
