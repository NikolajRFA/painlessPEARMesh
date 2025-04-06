#include "unity.h"
#include "painlessMeshSTA.h"

void test_containsTargetNodeId_should_return_true(void)
{
    uint8_t target[] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
    uint32_t targetNodeId = painlessmesh::tcp::encodeNodeId(target);
    uint8_t bssid1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t bssid2[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t bssid3[] = {0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb};
    WiFi_AP_Record_t record1;
    memcpy((void *)&record1.bssid, (void *)target, sizeof(record1.bssid));
    WiFi_AP_Record_t record2;
    memcpy((void *)&record2.bssid, (void *)bssid1, sizeof(record2.bssid));
    WiFi_AP_Record_t record3;
    memcpy((void *)&record3.bssid, (void *)bssid2, sizeof(record3.bssid));
    WiFi_AP_Record_t record4;
    memcpy((void *)&record4.bssid, (void *)bssid3, sizeof(record4.bssid));

    std::list<WiFi_AP_Record_t> aps;
    aps.push_back(record1);
    aps.push_back(record2);
    aps.push_back(record3);
    aps.push_back(record4);

    bool result = StationScan::containsTargetNodeId(aps, targetNodeId);

    TEST_ASSERT_TRUE(result);
}