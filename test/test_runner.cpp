#include "unity.h"
#include "painlessMeshSTA.h"
#include "compareWiFiAPRecords_tests.hpp"
#include "test_get_root_node_id.hpp"

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
    RUN_TEST(test_getRootNodeId_should_return_3206773453);
    RUN_TEST(test_compareWiFiAPRecords_with_useTargetBSSID_should_return_true);
    RUN_TEST(test_compareWiFiAPRecords_where_2nd_has_better_rssi_should_return_false);
    RUN_TEST(test_compareWiFiAPRecords_through_ap_list_and_with_useTargetBSSID_then_target_BSSID_is_first);
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
