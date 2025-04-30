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

inline String buildPearReportJson(uint8_t txPeriod, uint8_t rxPeriod, std::list<uint32_t> networks) {
    JsonDocument pearData;
    pearData["txPeriod"] = txPeriod;
    pearData["rxPeriod"] = rxPeriod;
    Serial.println("buildPearReportJson(): Building pear report!\n");
    Serial.printf("buildPearReportJson(): txPeriod: %u <> rxPeriod: %u\n", txPeriod, rxPeriod);

    JsonArray availableNetworksArray = pearData["availableNetworks"].to<JsonArray>();
    for (auto networkId: networks) {
        availableNetworksArray.add(networkId);
        Serial.printf("buildPearReportJson(): Adding %u to availableNetworksArray\n", networkId);
    }

    String pearDataString;
    serializeJson(pearData, pearDataString);
    Serial.printf("buildPearReportJson(): Finished building pear report: %s\n", pearDataString.c_str());

    return pearDataString;
}

#endif
