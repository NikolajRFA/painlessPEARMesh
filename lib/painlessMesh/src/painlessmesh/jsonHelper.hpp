#ifndef JSON_HELPER_HPP
#define JSON_HELPER_HPP

#include <ArduinoJson.h>

#include "pear.hpp"


using namespace painlessmesh;

inline String buildNewParentJson(const uint32_t nodeId) {
    JsonDocument doc;
    // create an object

    // Add an array of integers to the JSON object
    doc["newParent"] = nodeId;

    // Convert JSON object to string
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

inline bool jsonContainsNewParent(JsonDocument json) {
    return json["newParent"].is<uint32_t>();
}

inline String buildPearReportJson(const uint8_t txPeriod, const uint8_t rxPeriod, const std::list<uint32_t>& networks) {
    JsonDocument pearData;
    pearData["txPeriod"] = txPeriod;
    pearData["rxPeriod"] = rxPeriod;
    Serial.println("buildPearReportJson(): Building pear report!\n");
    Serial.printf("buildPearReportJson(): txPeriod: %u, rxPeriod: %u\n", txPeriod, rxPeriod);

    const JsonArray parentCandidates = pearData["parentCandidates"].to<JsonArray>();
    for (auto networkId: networks) {
        parentCandidates.add(networkId);
        Serial.printf("buildPearReportJson(): Adding %u to parentCandidates\n", networkId);
    }

    String pearDataString;
    serializeJson(pearData, pearDataString);
    Serial.printf("buildPearReportJson(): Finished building pear report: %s\n", pearDataString.c_str());

    return pearDataString;
}

#endif
