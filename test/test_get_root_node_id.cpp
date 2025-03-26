#include "unity.h"

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}


void test_function_should_doBlahAndBlah(void) {
    
    // test stuff
  }

int runUnityTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_function_should_doBlahAndBlah);
    return UNITY_END();
}

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void)
{
    return runUnityTests();
}