#include "unity.h"
#include "../lib/painlessMesh/src/painlessmesh/jsonHelper.hpp"
#include "list"

void buildNewParentJson_nodeId_expectedJsonWithNewParentKey()
{
    uint32_t nodeId = 3206773453;

    String jsonString = buildNewParentJson(nodeId);

    TEST_MESSAGE(jsonString.c_str());
    TEST_ASSERT_EQUAL_STRING("{\"newParent\":3206773453}", jsonString.c_str());
}

void jsonContainsNewParent_jsonWithNewParent_true()
{
    uint32_t nodeId = 3206773453;
    JsonDocument doc;

    // Add an array of integers to the JSON object
    doc[NEW_PARENT] = nodeId;

    TEST_ASSERT_TRUE(jsonContainsNewParent(doc));
}

void jsonContainsNewParent_jsonWithoutNewParent_false()
{
    uint32_t nodeId = 3206773453;
    JsonDocument doc;

    // Add an array of integers to the JSON object
    doc["oldParent"] = nodeId;

    TEST_ASSERT_FALSE(jsonContainsNewParent(doc));
}

void buildNewParentJsonAndjsonContainsNewParent_nodeId_true(){
    uint32_t nodeId = 3206773453;
    String jsonString = buildNewParentJson(nodeId);

    JsonDocument doc;
    deserializeJson(doc, jsonString);

    TEST_ASSERT_TRUE(jsonContainsNewParent(doc));
}

void buildPearReportJson_transmissionRateAndNetworksList_buildsExpectedJson(){
  uint8_t rxPeriod = 10;
  uint8_t txPeriod = 20;
  std::list<uint32_t> networks;

  networks.push_back(123);
  networks.push_back(456);
  networks.push_back(789);

  TEST_ASSERT_EQUAL_STRING("{\"txPeriod\":20,\"rxPeriod\":10,\"availableNetworks\":[123,456,789]}\"", buildPearReportJson(txPeriod, rxPeriod, networks).c_str());
}
