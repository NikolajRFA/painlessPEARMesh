#include "unity.h"
#include "painlessmesh/tcp.hpp"

void decodeNodeId_3206773453_8813BF237ACD(void)
{
  using namespace painlessmesh::tcp;
  uint32_t encodedValue = 3206773453;
  uint8_t bssid[6];

  decodeNodeId(encodedValue, bssid);

  TEST_ASSERT_EQUAL_INT(0x88, bssid[0]);
  TEST_ASSERT_EQUAL_INT(0x13, bssid[1]);
  TEST_ASSERT_EQUAL_INT(0xBF, bssid[2]);
  TEST_ASSERT_EQUAL_INT(0x23, bssid[3]);
  TEST_ASSERT_EQUAL_INT(0x7A, bssid[4]);
  TEST_ASSERT_EQUAL_INT(0xCD, bssid[5]);
}

void decodeNodeId_3206793885_8813BF23CA9D(void)
{
  using namespace painlessmesh::tcp;
  uint32_t encodedValue = 3206793885;
  uint8_t bssid[6];

  decodeNodeId(encodedValue, bssid);

  TEST_ASSERT_EQUAL_INT(0x88, bssid[0]);
  TEST_ASSERT_EQUAL_INT(0x13, bssid[1]);
  TEST_ASSERT_EQUAL_INT(0xBF, bssid[2]);
  TEST_ASSERT_EQUAL_INT(0x23, bssid[3]);
  TEST_ASSERT_EQUAL_INT(0xCA, bssid[4]);
  TEST_ASSERT_EQUAL_INT(0x9D, bssid[5]);
}

void decodeNodeId_3211408993_8813BF6A3661(void)
{
  using namespace painlessmesh::tcp;
  uint32_t encodedValue = 3211408993;
  uint8_t bssid[6];

  decodeNodeId(encodedValue, bssid);

  TEST_ASSERT_EQUAL_INT(0x88, bssid[0]);
  TEST_ASSERT_EQUAL_INT(0x13, bssid[1]);
  TEST_ASSERT_EQUAL_INT(0xBF, bssid[2]);
  TEST_ASSERT_EQUAL_INT(0x6A, bssid[3]);
  TEST_ASSERT_EQUAL_INT(0x36, bssid[4]);
  TEST_ASSERT_EQUAL_INT(0x61, bssid[5]);
}