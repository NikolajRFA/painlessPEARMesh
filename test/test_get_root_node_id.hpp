#include "unity.h"
#include "..\lib\painlessMesh\src\painlessmesh\mesh.hpp"
#include "painlessmesh.h"

void test_getRootNodeId_should_return_3206773453(void)
{
    u_int32_t rootId = painlessMesh::Mesh::getRootNodeId("{\"nodeId\" : 3211386233, \"subs\" : [ {\"nodeId\" : 3211408993, \"subs\" : [ {\"nodeId\" : 3206773453, \"root\" : true, \"subs\" : [ {\"nodeId\" : 3206793885} ]} ]} ]}");
    TEST_ASSERT_EQUAL_INT32(3206773453, rootId);
}
