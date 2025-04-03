//
// Created by nrfad on 03-04-2025.
//
#ifndef MOCKCONSTANTS_H
#define MOCKCONSTANTS_H
#include "../../../src/meshConstants.h"

#define VISIBLE_NETWORKS const std::unordered_map<uint32_t, std::vector<uint32_t>> visibleNetworks = {{CHIP1, {CHIP2, CHIP3, CHIP4}}, {CHIP2, {CHIP1, CHIP3}}, {CHIP3, {CHIP1, CHIP2, CHIP4}}, {CHIP4,{CHIP1, CHIP3}}};

#endif //MOCKCONSTANTS_H
