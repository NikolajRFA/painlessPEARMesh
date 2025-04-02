#ifndef JSON_HELPER_HPP
#define JSON_HELPER_HPP

#include <ArduinoJson.h>

inline String buildNewParentJson(const uint8_t *bssid) {
    JsonDocument doc;
    // create an object

    // Add an array of integers to the JSON object
    JsonArray array = doc["newParent"].to<JsonArray>();

    array.add(bssid[0]);
    array.add(bssid[1]);
    array.add(bssid[2]);
    array.add(bssid[3]);
    array.add(bssid[4]);
    array.add(bssid[5]);

    // Convert JSON object to string
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

inline bool jsonContainsNewParent(JsonDocument json) {
    return json["newParent"].is<JsonArray>();
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
