#include "unity.h"
#include "../lib/painlessMesh/src/painlessmesh/protocol.hpp"
#include "../lib/painlessMesh/src/painlessmesh/layout.hpp"

void test_getRootNodeFromNodeTree_should_return_correctRoot(void)
{
    auto nt = painlessmesh::protocol::NodeTree(123, false);
    
}