#include "deviceIds.h"
#define MESH_PREFIX1 "painless1"
#define MESH_PREFIX2 "painless2"
#define MESH_PREFIX3 "painless3"
#define MESH_PREFIX4 "painless4"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

#define VISIBLE_NETWORKS constexpr std::unordered_map<uint32_t, std::vector<uint32_t>> visibleNetworks = {{CHIP1, {CHIP2, CHIP3, CHIP4}}, {CHIP2, {CHIP1, CHIP3}}, {CHIP3, {CHIP1, CHIP2, CHIP4}}, {CHIP4,{CHIP1, CHIP3}}};
