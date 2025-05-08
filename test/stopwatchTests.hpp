#include "unity.h"
#include "../lib/painlessMesh/src/painlessmesh/stopwatch.hpp"
void timeSinceLastReportPearDataTask_delay500_true()
{
    using namespace painlessmesh;
    Stopwatch::getInstance().reset();
    int timeSinceLastReportPearDataTaskInitial = Stopwatch::getInstance().timeSinceLastReportPearDataTask();
    delay(500);
    int timeSinceLastReportPearDataTask = Stopwatch::getInstance().timeSinceLastReportPearDataTask();

    TEST_ASSERT_EQUAL_INT(0, timeSinceLastReportPearDataTaskInitial);
    TEST_ASSERT_TRUE(timeSinceLastReportPearDataTask >= 500 && timeSinceLastReportPearDataTask <= 510);
}

void timeSinceLastRunPearTask_delay500_true()
{
    using namespace painlessmesh;
    Stopwatch::getInstance().reset();
    int timeSinceLastRunPearTaskInitial = Stopwatch::getInstance().timeSinceLastRunPearTask();
    delay(500);
    int timeSinceLastRunPearTask = Stopwatch::getInstance().timeSinceLastRunPearTask();

    TEST_ASSERT_EQUAL_INT(0, timeSinceLastRunPearTaskInitial);
    TEST_ASSERT_TRUE(timeSinceLastRunPearTask >= 500 && timeSinceLastRunPearTask <= 510);
}

void timeSinceLastrunPearTask_delay500x2_true()
{
    using namespace painlessmesh;
    Stopwatch::getInstance().reset();
    int timeSinceLastRunPearTaskInitial = Stopwatch::getInstance().timeSinceLastRunPearTask();
    delay(500);
    int timeSinceLastRunPearTaskFirstInterval = Stopwatch::getInstance().timeSinceLastRunPearTask();
    delay(500);
    int timeSinceLastRunPearTaskSecondInterval = Stopwatch::getInstance().timeSinceLastRunPearTask();

    TEST_ASSERT_EQUAL_INT(0, timeSinceLastRunPearTaskInitial);
    TEST_ASSERT_TRUE(timeSinceLastRunPearTaskFirstInterval >= 500 && timeSinceLastRunPearTaskFirstInterval <= 510);
    TEST_ASSERT_TRUE(timeSinceLastRunPearTaskSecondInterval >= 500 && timeSinceLastRunPearTaskSecondInterval <= 510);
}