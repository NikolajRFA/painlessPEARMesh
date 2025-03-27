#include "unity.h"
#include "painlessMeshSTA.h"

void test_compareWiFiAPRecords_with_useTargetBSSID_should_return_true(void)
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

void test_compareWiFiAPRecords_where_1st_has_better_rssi_should_return_true(void)
{
    uint8_t bssid1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t bssid2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    WiFi_AP_Record_t record1;
    memcpy((void *)&record1.bssid, (void *)bssid1, sizeof(record1.bssid));
    record1.rssi = -25;
    WiFi_AP_Record_t record2;
    memcpy((void *)&record2.bssid, (void *)bssid2, sizeof(record2.bssid));
    record2.rssi = -50;

    bool result = StationScan::compareWiFiAPRecords(record1, record2, false, bssid1);

    TEST_ASSERT_TRUE(result);
}

void test_compareWiFiAPRecords_where_2nd_has_better_rssi_should_return_false(void)
{
    uint8_t bssid1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t bssid2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    WiFi_AP_Record_t record1;
    memcpy((void *)&record1.bssid, (void *)bssid1, sizeof(record1.bssid));
    record1.rssi = -50;
    WiFi_AP_Record_t record2;
    memcpy((void *)&record2.bssid, (void *)bssid2, sizeof(record2.bssid));
    record2.rssi = -25;

    bool result = StationScan::compareWiFiAPRecords(record1, record2, false, bssid1);

    TEST_ASSERT_FALSE(result);
}

void test_compareWiFiAPRecords_through_ap_list_and_with_useTargetBSSID_then_target_BSSID_is_first()
{
    uint8_t bssid1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t bssid2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t bssid3[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    WiFi_AP_Record_t record1;
    memcpy((void *)&record1.bssid, (void *)bssid1, sizeof(record1.bssid));
    record1.rssi = -20;
    WiFi_AP_Record_t record2;
    memcpy((void *)&record2.bssid, (void *)bssid2, sizeof(record2.bssid));
    record2.rssi = -30;
    WiFi_AP_Record_t record3;
    memcpy((void *)&record3.bssid, (void *)bssid3, sizeof(record3.bssid));
    record3.rssi = -60;
    std::list<WiFi_AP_Record_t> aps;
    aps.push_back(record1);
    aps.push_back(record2);
    aps.push_back(record3);

    aps.sort([bssid3](WiFi_AP_Record_t a, WiFi_AP_Record_t b)
             { return StationScan::compareWiFiAPRecords(a, b, true, bssid3); });
    
    TEST_ASSERT_EQUAL_INT8_ARRAY(bssid3, aps.front().bssid, sizeof(bssid3));
    aps.pop_front();
    TEST_ASSERT_EQUAL_INT8_ARRAY(bssid1, aps.front().bssid, sizeof(bssid1));
}
