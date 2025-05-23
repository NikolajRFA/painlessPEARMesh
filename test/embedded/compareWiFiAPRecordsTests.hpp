#include "unity.h"
#include "painlessMeshSTA.h"

void compareWiFiAPRecords_useTargetNodeIdTrueAndTargetNodeId_true(void)
{
    uint8_t bssid1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t bssid2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    WiFi_AP_Record_t record1;
    memcpy((void *)&record1.bssid, (void *)bssid1, sizeof(record1.bssid));
    record1.rssi = -50;
    WiFi_AP_Record_t record2;
    memcpy((void *)&record2.bssid, (void *)bssid2, sizeof(record2.bssid));
    record2.rssi = -25;
    uint32_t targetNodeId = painlessmesh::tcp::encodeNodeId(bssid1);

    bool result = StationScan::compareWiFiAPRecords(record1, record2, true, targetNodeId);

    TEST_ASSERT_TRUE(result);
}

void compareWiFiAPRecords_1stHasBetterRssi_true(void)
{
    uint8_t bssid1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t bssid2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    WiFi_AP_Record_t record1;
    memcpy((void *)&record1.bssid, (void *)bssid1, sizeof(record1.bssid));
    record1.rssi = -25;
    WiFi_AP_Record_t record2;
    memcpy((void *)&record2.bssid, (void *)bssid2, sizeof(record2.bssid));
    record2.rssi = -50;
    uint32_t targetNodeId = painlessmesh::tcp::encodeNodeId(bssid1);

    bool result = StationScan::compareWiFiAPRecords(record1, record2, false, targetNodeId);

    TEST_ASSERT_TRUE(result);
}

void compareWiFiAPRecords_2ndHasBetterRssi_false(void)
{
    uint8_t bssid1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t bssid2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    WiFi_AP_Record_t record1;
    memcpy((void *)&record1.bssid, (void *)bssid1, sizeof(record1.bssid));
    record1.rssi = -50;
    WiFi_AP_Record_t record2;
    memcpy((void *)&record2.bssid, (void *)bssid2, sizeof(record2.bssid));
    record2.rssi = -25;
    uint32_t targetNodeId = painlessmesh::tcp::encodeNodeId(bssid1);

    bool result = StationScan::compareWiFiAPRecords(record1, record2, false, targetNodeId);

    TEST_ASSERT_FALSE(result);
}

void compareWiFiAPRecords_apListAndUseTargetNodeID_targetNodeIdIsFirst()
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
    uint32_t targetNodeId = painlessmesh::tcp::encodeNodeId(bssid3);

    aps.sort([targetNodeId](WiFi_AP_Record_t a, WiFi_AP_Record_t b)
             { return StationScan::compareWiFiAPRecords(a, b, true, targetNodeId); });
    
    TEST_ASSERT_EQUAL_INT8_ARRAY(bssid3, aps.front().bssid, sizeof(bssid3));
    aps.pop_front();
    TEST_ASSERT_EQUAL_INT8_ARRAY(bssid1, aps.front().bssid, sizeof(bssid1));
}
