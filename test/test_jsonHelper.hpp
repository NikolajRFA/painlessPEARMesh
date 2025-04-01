#include "unity.h"
#include "../lib/painlessMesh/src/painlessmesh/jsonHelper.hpp"

void test_build_new_parent_json_should_return_jsonString()
{
    uint8_t bssid[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    String jsonString = buildNewParentJson(bssid);

    TEST_MESSAGE(jsonString.c_str());

    TEST_ASSERT_EQUAL_STRING("{\"newParent\":[170,170,170,170,170,170]}", jsonString.c_str());
}

void test_json_contains_new_parent_returns_true()
{
    JsonDocument doc;

    // Add an array of integers to the JSON object
    JsonArray array = doc["newParent"].to<JsonArray>();

    uint8_t bssid[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    array.add(bssid[0]);
    array.add(bssid[1]);
    array.add(bssid[2]);
    array.add(bssid[3]);
    array.add(bssid[4]);
    array.add(bssid[5]);

    TEST_ASSERT_TRUE(jsonContainsNewParent(doc));
}

void test_json_contains_new_parent_returns_false()
{
    JsonDocument doc;

    // Add an array of integers to the JSON object
    JsonArray array = doc["oldParent"].to<JsonArray>();

    uint8_t bssid[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    array.add(bssid[0]);
    array.add(bssid[1]);
    array.add(bssid[2]);
    array.add(bssid[3]);
    array.add(bssid[4]);
    array.add(bssid[5]);

    TEST_ASSERT_FALSE(jsonContainsNewParent(doc));
}

void test_buildNewParentJson_and_jsonContainsNewParent_returns_true(){
    uint8_t bssid[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    String jsonString = buildNewParentJson(bssid);

    JsonDocument doc;
    deserializeJson(doc, jsonString);

    TEST_ASSERT_TRUE(jsonContainsNewParent(doc));
}