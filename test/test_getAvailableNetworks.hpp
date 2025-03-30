#include "unity.h"
#include "..\lib\painlessMesh\src\painlessmesh\mesh.hpp"
#include "painlessmesh.h"
#include "..\lib\painlessMesh\src\painlessmesh\wifiWrapper.hpp"

void test_getAvailableNetworks_should_return_2networks(void)
{
    MockWiFiWrapper mockWiFi;
    std::list<String> availableNetworks = painlessMesh::Mesh::getAvailableNetworks(mockWiFi);
    TEST_ASSERT_EQUAL(2, availableNetworks.size());     
}
