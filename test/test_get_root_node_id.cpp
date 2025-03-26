#include "unity.h"
#include "..\src\meshHelpers.cpp"
painlessMesh mesh;

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_getRootNodeId_should_return_3206773453(void)
{
    u_int32_t rootId = getRootNodeId("{\"nodeId\" : 3211386233, \"subs\" : [ {\"nodeId\" : 3211408993, \"subs\" : [ {\"nodeId\" : 3206773453, \"root\" : true, \"subs\" : [ {\"nodeId\" : 3206793885} ]} ]} ]}");
    TEST_ASSERT_EQUAL_INT32(3206773453, rootId);
}

int runUnityTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_getRootNodeId_should_return_3206773453);
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