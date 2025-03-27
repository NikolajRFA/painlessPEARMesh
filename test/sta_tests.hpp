#include "unity.h"
#include "painlessMeshSTA.h"

void test_compareWiFiAPRecords_should_return_true(void)
{
    uint8_t bssid1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t bssid2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    WiFi_AP_Record_t record1;
    memcpy((void *)&record1.bssid, (void *)bssid1, sizeof(record1.bssid));
    record1.rssi = -50;
    WiFi_AP_Record_t record2;
    memcpy((void *)&record2.bssid, (void *)bssid2, sizeof(record2.bssid));
    record2.rssi = -25;
    
    bool result = StationScan::compareWiFiAPRecords(record1, record2, true, bssid1);

    TEST_ASSERT_TRUE(result);
}
