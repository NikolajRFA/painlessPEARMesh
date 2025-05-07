#include "unity.h"
#include "painlessMeshSTA.h"
#include "compareWiFiAPRecordsTests.hpp"
#include "containsTargetNodeIdTests.hpp"
#include "jsonHelperTests.hpp"
#include "getRootNodeIdTests.hpp"
#include "decodeNodeIdTests.hpp"
#include "pearTests.hpp"
#include "stopwatchTests.hpp"

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
    RUN_TEST(compareWiFiAPRecords_useTargetNodeIdTrueAndTargetNodeId_true);
    RUN_TEST(compareWiFiAPRecords_1stHasBetterRssi_true);
    RUN_TEST(compareWiFiAPRecords_2ndHasBetterRssi_false);
    RUN_TEST(compareWiFiAPRecords_apListAndUseTargetNodeID_targetNodeIdIsFirst);
    RUN_TEST(containsTargetNodeId_listOfAps_true);
    RUN_TEST(buildNewParentJson_nodeId_expectedJsonWithNewParentKey);
    RUN_TEST(jsonContainsNewParent_jsonWithNewParent_true);
    RUN_TEST(jsonContainsNewParent_jsonWithoutNewParent_false);
    RUN_TEST(buildNewParentJsonAndjsonContainsNewParent_nodeId_true);
    RUN_TEST(getRootNodeId_NodeTreeWithRootNode_1);
    RUN_TEST(getRootNodeId_NodeTreeWithoutRootNode_0);
    RUN_TEST(decodeNodeId_3206773453_8813BF237ACD);
    RUN_TEST(decodeNodeId_3206793885_8813BF23CA9D);
    RUN_TEST(decodeNodeId_3211408993_8813BF6A3661);
    RUN_TEST(processReceivedData_unseenPearNodeTree_mapWithPearNodeTree);
    RUN_TEST(deviceExceedsThreshold_deviceExceedingThreshold_true);
    RUN_TEST(deviceExceedsThreshold_deviceNotExceedingThreshold_false);
    RUN_TEST(run_parentCandidateExceedsLimit_reroutesIsEmpty);
    RUN_TEST(updateParent_nodeWithValidParentCandidates_reroutesContainsReroute);
    RUN_TEST(updateParent_nodeWithInvalidParentCandidates_reroutesContainsNoRoutes);
    RUN_TEST(getAllDevicesBreadthFirst_rootNodeTree_listOfPearNodesBreadthFirst);
    RUN_TEST(test_run_should_process_multiple_nodes_until_threshold);
    RUN_TEST(timeSinceLastReportPearDataTask_delay500_true);
    RUN_TEST(timeSinceLastRunPearTask_delay500_true);
    RUN_TEST(timeSinceLastrunPearTask_delay500x2_true);
    return UNITY_END();
}

/**
 * For Arduino framework
 */
void setup()
{
    Serial.begin(115200);
    // Wait ~2 seconds before the Unity test runner
    // establishes connection with a board Serial interface
    delay(2000);
    Serial.println("Test starting...");
    runUnityTests();
}
void loop() {}
