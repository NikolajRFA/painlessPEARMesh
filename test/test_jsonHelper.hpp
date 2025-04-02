#include "unity.h"
#include "../lib/painlessMesh/src/painlessmesh/jsonHelper.hpp"
#include "list"

void test_build_new_parent_json_should_return_jsonString()
{
    uint32_t nodeId = 3206773453;

    String jsonString = buildNewParentJson(nodeId);

    TEST_MESSAGE(jsonString.c_str());
    TEST_ASSERT_EQUAL_STRING("{\"newParent\":3206773453}", jsonString.c_str());
}

void test_json_contains_new_parent_returns_true()
{
    uint32_t nodeId = 3206773453;
    JsonDocument doc;

    // Add an array of integers to the JSON object
    doc["newParent"] = nodeId;

    TEST_ASSERT_TRUE(jsonContainsNewParent(doc));
}

void test_json_contains_new_parent_returns_false()
{
    uint32_t nodeId = 3206773453;
    JsonDocument doc;

    // Add an array of integers to the JSON object
    doc["oldParent"] = nodeId;

    TEST_ASSERT_FALSE(jsonContainsNewParent(doc));
}

void test_buildNewParentJson_and_jsonContainsNewParent_returns_true(){
    uint32_t nodeId = 3206773453;
    String jsonString = buildNewParentJson(nodeId);

    JsonDocument doc;
    deserializeJson(doc, jsonString);

    TEST_ASSERT_TRUE(jsonContainsNewParent(doc));
}

void test_buildPearReportJson_returns_expected_jsonString(){
  uint8_t transmissionRate = 69;
  std::list<uint32_t> networks;

  networks.push_back(123);
  networks.push_back(456);
  networks.push_back(789);

  TEST_ASSERT_EQUAL_STRING("{\"transmissionRate\":69,\"availableNetworks\":[123,456,789]}\"", buildPearReportJson(transmissionRate, networks).c_str());
}
