#include "unity.h"
#include "pearTests.cpp"

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
    RUN_TEST(processReceivedData_unseenPearNodeTree_mapWithPearNodeTree);
    RUN_TEST(deviceExceedsThreshold_deviceExceedingThreshold_true);
    RUN_TEST(deviceExceedsThreshold_deviceNotExceedingThreshold_false);
    RUN_TEST(run_parentCandidateExceedsLimit_reroutesIsEmpty);
    RUN_TEST(updateParent_nodeWithValidParentCandidates_reroutesContainsReroute);
    RUN_TEST(updateParent_nodeWithInvalidParentCandidates_reroutesContainsNoRoutes);
    RUN_TEST(getAllDevicesBreadthFirst_rootNodeTree_listOfPearNodesBreadthFirst);
    RUN_TEST(test_run_should_reroute_1_node);
    RUN_TEST(energyProfileSetInsert_unorderedEnergyProfiles_setWithOrderedEnergyProfilesAsc);
    return UNITY_END();
}

/**
 * For Arduino framework
 */
int main()
{
  return runUnityTests();
}