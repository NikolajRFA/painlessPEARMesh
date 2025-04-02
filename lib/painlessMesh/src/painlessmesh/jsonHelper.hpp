#ifndef JSON_HELPER_HPP
#define JSON_HELPER_HPP

#include <ArduinoJson.h>

inline String buildNewParentJson(const uint32_t nodeId)
{
    JsonDocument doc;
    // create an object

    // Add an array of integers to the JSON object
    doc["newParent"] = nodeId;

    // Convert JSON object to string
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

inline bool jsonContainsNewParent(JsonDocument json){
    return json["newParent"].is<uint32_t>();
}

inline String buildPearReportJson(uint8_t transmissionRate, std::list<uint32_t> networks) {
    JsonDocument pearData;
    pearData["transmissionRate"] = transmissionRate;

    JsonArray availableNetworksArray = pearData["availableNetworks"].to<JsonArray>();
    for (auto networkId: networks) {
        availableNetworksArray.add(networkId);
    }

    String pearDataString;
    serializeJson(pearData, pearDataString);

    return pearDataString;
}
#endif
