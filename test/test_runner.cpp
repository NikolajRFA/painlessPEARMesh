#include "unity.h"
#include "painlessMeshSTA.h"
#include "compareWiFiAPRecords_tests.hpp"
#include "test_containsTargetBSSID.hpp"
#include "test_jsonHelper.hpp"
#include "getRootNodeIdTests.hpp"
#include "decodeNodeIdTests.hpp"

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

int runUnityTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_compareWiFiAPRecords_with_useTargetBSSID_should_return_true);
    RUN_TEST(test_compareWiFiAPRecords_where_2nd_has_better_rssi_should_return_false);
    RUN_TEST(test_compareWiFiAPRecords_through_ap_list_and_with_useTargetBSSID_then_target_BSSID_is_first);
    RUN_TEST(test_containsTargetNodeId_should_return_true);
    RUN_TEST(test_build_new_parent_json_should_return_jsonString);
    RUN_TEST(test_json_contains_new_parent_returns_true);
    RUN_TEST(test_json_contains_new_parent_returns_false);
    RUN_TEST(test_buildNewParentJson_and_jsonContainsNewParent_returns_true);
    RUN_TEST(getRootNodeId_NodeTreeWithRootNode_1);
    RUN_TEST(getRootNodeId_NodeTreeWithoutRootNode_0);
    RUN_TEST(decodeNodeId_3206773453_8813BF237ACD);
    RUN_TEST(decodeNodeId_3206793885_8813BF23CA9D);
    RUN_TEST(decodeNodeId_3211408993_8813BF6A3661);
    return UNITY_END();
}

/**
 * For Arduino framework
 */
void setup()
{
    // Wait ~2 seconds before the Unity test runner
    // establishes connection with a board Serial interface
    delay(2000);

    runUnityTests();
}
void loop() {}
