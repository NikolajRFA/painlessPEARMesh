#ifndef JSON_HELPER_HPP
#define JSON_HELPER_HPP

#include <ArduinoJson.h>
#include <set>
#include "jsonKeyConstanst.hpp"


using namespace painlessmesh;

inline String buildNewParentJson(const uint32_t nodeId) {
    JsonDocument doc;
    // create an object

    // Add an array of integers to the JSON object
    doc[NEW_PARENT] = nodeId;

    // Convert JSON object to string
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

inline bool jsonContainsNewParent(JsonDocument json) {
    return json[NEW_PARENT].is<uint32_t>();
}

inline String buildPearReportJson(const uint8_t txPeriod, const uint8_t rxPeriod, const std::list<uint32_t>& networks) {
    JsonDocument pearData;
    pearData[TX_PERIOD] = txPeriod;
    pearData[RX_PERIOD] = rxPeriod;
    Serial.println("buildPearReportJson(): Building pear report!\n");
    Serial.printf("buildPearReportJson(): txPeriod: %u, rxPeriod: %u\n", txPeriod, rxPeriod);

    const JsonArray parentCandidates = pearData[PARENT_CANDIDATES].to<JsonArray>();
    for (auto networkId: networks) {
        parentCandidates.add(networkId);
        Serial.printf("buildPearReportJson(): Adding %u to parentCandidates\n", networkId);
    }

    String pearDataString;
    serializeJson(pearData, pearDataString);
    Serial.printf("buildPearReportJson(): Finished building pear report: %s\n", pearDataString.c_str());

    return pearDataString;
}

/**
 * Method created using chatGPT - modified to match ArduinoJson 7+
 * @param jsonString TSTRING represenation of the mesh connections
 * @return number of nodes in the mesh
 */
inline size_t countUniqueNodeIds(const TSTRING& jsonString) {
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, jsonString, DeserializationOption::NestingLimit(30));

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return 0;
    }

    std::set<uint32_t> uniqueNodeIds;

    std::function<void(JsonVariant)> traverse;
    traverse = [&](const JsonVariant node) {
        if (!node.is<JsonObject>()) return;

        const JsonVariant idField = node[NODE_ID];
        if (idField.is<uint32_t>()) {
            uniqueNodeIds.insert(idField.as<uint32_t>());
        }

        const JsonVariant subsField = node[SUBS];
        if (subsField.is<JsonArray>()) {
            for (JsonVariant subNode : subsField.as<JsonArray>()) {
                traverse(subNode);
            }
        }
    };

    traverse(doc.as<JsonVariant>());

    return uniqueNodeIds.size();
}

#endif
