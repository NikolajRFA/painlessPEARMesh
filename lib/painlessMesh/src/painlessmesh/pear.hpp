#ifndef _PAINLESS_MESH_PEAR_HPP_
#define _PAINLESS_MESH_PEAR_HPP_

#include <Arduino.h>
#include <set>
#include <list>
#include <queue>
#include <unordered_map>
#include <map>

#include "jsonHelper.hpp"
#include "protocol.hpp"
#include "layout.hpp"
#include "logger.hpp"

namespace painlessmesh {
    class PearNodeTree : public protocol::NodeTree {
    public:
        int periodTx = 0;
        int periodRx = 0;
        std::list<std::shared_ptr<PearNodeTree> > parentCandidates;
        int txThreshold = 38;
        int rxThreshold = 8;
        int energyProfile = (txThreshold + rxThreshold) / 2;

        // Define the < operator for comparison
        bool operator<(const PearNodeTree &other) const {
            return periodRx > other.periodRx;
        }

        bool operator==(const PearNodeTree &other) const {
            return periodRx == other.periodRx
                   && periodTx == other.periodTx
                   && energyProfile == other.energyProfile
                   && parentCandidates == other.parentCandidates;
        }


        PearNodeTree() = default;

        explicit PearNodeTree(const std::shared_ptr<NodeTree> &nodeTree) {
            this->nodeId = nodeTree->nodeId;
            this->root = nodeTree->root;
            this->subs = nodeTree->subs;
        }

        PearNodeTree(const std::shared_ptr<NodeTree> &nodeTree, const int periodTx, const int periodRx,
                     const std::list<std::shared_ptr<PearNodeTree> > &parentCandidates) {
            this->nodeId = nodeTree->nodeId;
            this->root = nodeTree->root;
            this->subs = nodeTree->subs;
            this->periodRx = periodRx;
            this->periodTx = periodTx;
            this->parentCandidates = parentCandidates;
        }

        PearNodeTree(const std::shared_ptr<NodeTree> &nodeTree, const int txThreshold, const int rxThreshold) {
            this->nodeId = nodeTree->nodeId;
            this->root = nodeTree->root;
            this->subs = nodeTree->subs;
            this->txThreshold = txThreshold;
            this->rxThreshold = rxThreshold;
        }
    };

    class Pear {
    public:
        static Pear &getInstance() {
            static Pear instance; // Guaranteed to be created only once (thread-safe in C++11 and later)
            return instance;
        }

        static void reset() {
            auto instance = &getInstance();
            instance->noOfVerifiedDevices = 0;
            instance->pearNodeTreeMap.clear();
            instance->reroutes.clear();
        }

        // Delete copy constructor and assignment operator
        Pear(const Pear &) = delete;

        Pear &operator=(const Pear &) = delete;

        uint8_t noOfVerifiedDevices = 0;
        std::map<uint32_t, std::shared_ptr<PearNodeTree> > pearNodeTreeMap;
        std::unordered_map<uint32_t, String> reroutes; // Create logic to send the reroute information

        /**
         * @brief Executes the main evaluation routine (PEAR) on the entire device tree.
         *
         * This function performs a breadth-first traversal of the given `rootNodeTree` to obtain a list of all devices
         * represented as `PearNodeTree` objects. It iterates through the list and, if the number of verified devices is below a limit (10),
         * it continues checking devices to see if they exceed energy profile thresholds using `deviceExceedsLimit()`. If so, it triggers a parent
         * update for that device via `updateParent()`.
         *
         * @param rootNodeTree The root of the device tree to be processed.
         *
         * @note
         * - The function uses `getAllDevicesBreadthFirst()` to traverse the tree.
         * - It relies on global state: `noOfVerifiedDevices` and `pearNodeTreeMap`.
         * - Only one device is processed due to the immediate `return` statement inside the loop.
         */
        void run(const protocol::NodeTree &rootNodeTree) {
            const auto listOfAllDevices = getAllDevicesBreadthFirst(rootNodeTree);
            noOfVerifiedDevices = 0;
            for (const auto &pearNodeTree: listOfAllDevices) {
                const auto reroutesContainsReroute = reroutes.count(pearNodeTree->nodeId);
                if (noOfVerifiedDevices < 10 && !reroutesContainsReroute) {
                    if (deviceExceedsLimit(pearNodeTree->nodeId)) updateParent(pearNodeTree);
                } else {
                    using namespace painlessmesh::logger;
                    Log(PEAR_DEBUG, "Number of verified devices: %u\n", noOfVerifiedDevices);
                    if (reroutesContainsReroute) Log(PEAR_DEBUG, "Reroutes already contains a reroute for %u\n",
                                                     pearNodeTree->nodeId);
                    return;
                }
            }
        }

        /**
         * @brief Determines whether a given device exceeds both transmission and reception thresholds.
         *
         * This function checks whether the provided `PearNodeTree` instance has `periodRx` and `periodTx` values
         * that exceed their respective thresholds (`rxThreshold` and `txThreshold`). If both conditions are met,
         * the function returns `true`, indicating that the device is operating beyond its defined limits.
         *
         * @param pearNodeTree A `PearNodeTree` object representing the device to evaluate.
         *
         * @return `true` if both `periodRx` > `rxThreshold` and `periodTx` > `txThreshold`; otherwise, `false`.
         */
        static bool deviceExceedsThreshold(const std::shared_ptr<PearNodeTree> &pearNodeTree) {
            using namespace painlessmesh::logger;
            Log(PEAR_DEBUG, "Checking if node >%u< exceeds the threshold\n", pearNodeTree->nodeId);
            Log(PEAR_DEBUG, "txPeriod: %u > txThreshold: %u\n", pearNodeTree->periodTx, pearNodeTree->txThreshold);
            Log(PEAR_DEBUG, "rxPeriod: %u > rxThreshold: %u\n", pearNodeTree->periodRx, pearNodeTree->rxThreshold);

            return pearNodeTree->periodRx > pearNodeTree->rxThreshold && pearNodeTree->periodTx > pearNodeTree->
                   txThreshold;
        }

        /**
         * @brief Checks whether a device exceeds the defined energy profile thresholds.
         *
         * This function looks up a `PearNodeTree` by the given `deviceId` from the global `pearNodeTreeMap`. If the node is found,
         * it evaluates whether the device exceeds a predefined threshold by calling `deviceExceedsThreshold()`. If the threshold is exceeded,
         * the function returns `true`. Otherwise, it increments the global `noOfVerifiedDevices` counter and returns `false`.
         *
         * @param deviceId The unique identifier of the device to be evaluated.
         *
         * @return `true` if the device exceeds the operational threshold, `false` otherwise or if the device is not found.
         */
        bool deviceExceedsLimit(const uint32_t deviceId) {
            using namespace painlessmesh::logger;
            Serial.printf( "deviceExceedsLimit(): Checking if node: %u exceeds limit\n", deviceId);
            //auto pearNodeTree = findPearNodeTreeById(deviceId);
            const auto it = pearNodeTreeMap.find(deviceId);
            if (it == pearNodeTreeMap.end()) return false;
            const auto pearNodeTree = it->second;
            Serial.printf( "deviceExceedsLimit(): Raw pointer: %p\n", pearNodeTree.get());
            Serial.printf( "deviceExceedsLimit(): Found pearNodeTree: %u (tx: %u, rx: %u)\n", pearNodeTree->nodeId,
                pearNodeTree->periodTx, pearNodeTree->periodRx);
            if (deviceExceedsThreshold(pearNodeTree)) {
                Serial.printf( "deviceExceedsLimit(): Node: %u exceeds the threshold: rx: %i, tx: %i\n", deviceId,
                    pearNodeTree->rxThreshold, pearNodeTree->txThreshold);
                return true;
            }
            Serial.printf(
                "deviceExceedsLimit(): Incrementing verified devices as node: %u does not exceed the threshold\n",
                deviceId);
            noOfVerifiedDevices++;
            return false;
        }

        /**
         * @brief Evaluates and updates potential parent nodes for the given PearNodeTree based on transmission priorities and thresholds.
         *
         * This method iterates over the sub nodes of the given `pearNodeTree`, collects valid sub nodes from the global
         * `pearNodeTreeMap`, and sorts them into a set (`descendingTxList`). For each sub node, if it has available parent candidates,
         * the method attempts to reroute its parent assignment to the first candidate that does not exceed a defined threshold.
         *
         * Successful reroutes are recorded in the global `reroutes` map, where the key is the node ID of the child being rerouted,
         * and the value is the node ID of the new parent candidate.
         *
         * Nodes are only rerouted to the first eligible parent candidate that does not exceed the threshold.
         * No further candidates are evaluated once a valid one is found.
         *
         * @param pearNodeTree The `PearNodeTree` whose sub nodes are evaluated for potential parent updates.
         *
         * @note This function assumes:
         * - `pearNodeTreeMap` is a globally accessible mapping from node IDs to `PearNodeTree` objects.
         * - `deviceExceedsThreshold(candidate)` is a predicate function used to check if a candidate node is eligible.
         * - `reroutes` is a globally accessible map used to track rerouted parent relationships.
         */
        void updateParent(const std::shared_ptr<PearNodeTree> &pearNodeTree) {
            using namespace painlessmesh::logger;
            Log(PEAR_DEBUG, "updateParent(): Attempting to reroute the most consuming sub of node: %u\n",
                pearNodeTree->nodeId);
            std::set<std::shared_ptr<PearNodeTree> > descendingTxList;
            for (const auto &sub: pearNodeTree->subs) {
                const auto it = pearNodeTreeMap.find(sub.nodeId);
                if (it == pearNodeTreeMap.end()) {
                    Log(PEAR_DEBUG, "updateParent(): Sub not found in map");
                    continue;
                }
                const auto pearNodeTreeSub = it->second;
                descendingTxList.insert(pearNodeTreeSub);
            }

            if (descendingTxList.empty()) {
                Log(PEAR_DEBUG, "updateParent(): No subs founds - unable to optimize DPDU!");
                return;
            }

            for (const auto &nodeToReroute: descendingTxList) {
                Log(PEAR_DEBUG, "updateParent(): Checking if node: %u is able to be rerouted to a valid candidate\n",
                    nodeToReroute->nodeId);
                if (!nodeToReroute->parentCandidates.empty()) {
                    for (const auto &candidate: nodeToReroute->parentCandidates) {
                        Log(PEAR_DEBUG, "updateParent(): Checking candidate %u: rx %d, tx %d\n", candidate->nodeId,
                            candidate->periodRx, candidate->periodTx);
                        if (deviceExceedsThreshold(candidate)) {
                            Log(PEAR_DEBUG, "updateParent(): Candidate exceeds threshold, skipping");
                            continue;
                        }
                        String jsonString = buildNewParentJson(candidate->nodeId);

                        reroutes.insert({nodeToReroute->nodeId, jsonString});
                        Log(PEAR, "updateParent(): Rerouted %u to %u\n", nodeToReroute->nodeId, candidate->nodeId);
                        break;
                    }
                }
            }
        }


        /**
         * @brief Processes received JSON data and updates the internal pear node tree map.
         *
         * This method extracts transmission and reception periods (`periodTx`, `periodRx`) and a list of parent candidate node IDs
         * from the provided JSON document. It uses this data to update or create a corresponding `PearNodeTree` instance for the given
         * `PearNodeTree` within the global `pearNodeTreeMap`. If a candidate node is not already in the map, it retrieves the node from
         * the tree layout and inserts it.
         *
         * @param pearData A `JsonDocument` containing received data. Expected fields:
         * - `periodTx`: An integer representing the transmission period.
         * - `periodRx`: An integer representing the reception period.
         * - `parentCandidates`: A JSON array of node IDs representing potential parent nodes.
         *
         * @param nodeTree A reference to the `protocol::NodeTree` that corresponds to the current device being processed.
         *
         * @note This method assumes that `layout::getNodeById()` can retrieve nodes from the `nodeTree` structure and that
         *       `PearNodeTree` has a constructor accepting a `NodeTree`, or a `NodeTree` with additional metadata like tx/rx and candidates.
         */
        void processReceivedData(JsonDocument &pearData, const std::shared_ptr<protocol::NodeTree> &nodeTree) {
            using namespace painlessmesh::logger;
            Log(PEAR_DEBUG, "processReceivedData(): Started processing received data!");
            const int periodTx = pearData["txPeriod"];
            const int periodRx = pearData["rxPeriod"];
            Log(PEAR_DEBUG, "processReceivedData(): pearData: periodTx: %i, periodRx: %i, nodeId: %u\n", periodTx,
                periodRx, nodeTree->nodeId);
            const auto parentCandidatesJsonArray = pearData["parentCandidates"].as<JsonArray>();
            std::list<std::shared_ptr<PearNodeTree> > parentCandidates;
            Log(PEAR_DEBUG, "processReceivedData(): parentCandidates count: %i\n", parentCandidatesJsonArray.size());
            for (JsonVariant v: parentCandidatesJsonArray) {
                auto id = v.as<uint32_t>();
                Log(PEAR_DEBUG, "JsonVariant: %s, converted to uint32_t: %u\n", v.as<String>().c_str(), id);

                const auto it = pearNodeTreeMap.find(id);
                if (it == pearNodeTreeMap.end()) {
                    Log(PEAR_DEBUG, "processReceivedData(): Attempting to find NodeTree instance with nodeId: %u\n",
                        id);
                    Log(PEAR_DEBUG, "processReceivedData(): Passing NodeTree with id: %u to search from\n",
                        nodeTree->nodeId);
                    const auto missingNodeTree = layout::getNodeById(nodeTree, id);
                    if (missingNodeTree == nullptr) break;
                    //auto missingPearNodeTree = std::make_shared<PearNodeTree>(PearNodeTree(missingNodeTree));
                    auto missingPearNodeTree = std::make_shared<PearNodeTree>(missingNodeTree);
                    pearNodeTreeMap.insert({id, missingPearNodeTree});
                    parentCandidates.push_back(missingPearNodeTree);
                } else {
                    const auto pearNodeTree = it->second;
                    parentCandidates.push_back(pearNodeTree);
                }
                if (id == rootNodeId) {
                    // If a node has the root node as a parent candidate then we always want to reroute the node to the root
                    Serial.println("Reroute to root available - adding reroute!");

                    String jsonString = buildNewParentJson(rootNodeId);

                    reroutes.insert({id, jsonString});
                    break;
                }
            }

            if (pearNodeTreeMap.count(nodeTree->nodeId)) {
                const auto foundPearNodeTree = pearNodeTreeMap[nodeTree->nodeId];
                foundPearNodeTree->periodTx = periodTx;
                foundPearNodeTree->periodRx = periodRx;
                foundPearNodeTree->parentCandidates = parentCandidates;
                foundPearNodeTree->subs = nodeTree->subs;
            } else {
                pearNodeTreeMap.insert({
                    nodeTree->nodeId, std::make_shared<PearNodeTree>(nodeTree, periodTx, periodRx, parentCandidates)
                });
            }
        }

        /**
         * @brief Traverses a NodeTree structure in breadth-first order and returns all nodes as a list of PearNodeTree objects.
         *
         * This method performs a breadth-first traversal starting from the given root node, collecting each visited node
         * into a `std::list<PearNodeTree>`. It assumes that each node (represented as a PearNodeTree) contains a collection
         * of child nodes (`subs`). Each child node is converted to a `PearNodeTree` before being added to the traversal queue.
         *
         * @param rootNodeTree The root node of the tree structure to traverse, represented as a `protocol::NodeTree`.
         *
         * @return std::list<PearNodeTree> A list of all nodes visited in breadth-first order, each converted to `PearNodeTree`.
         *
         * @note This method assumes that the conversion from `protocol::NodeTree` to `PearNodeTree` is valid through the constructor
         *       `PearNodeTree(child)` where `child` is of type `NodeTree`.
         */
        std::list<std::shared_ptr<PearNodeTree> > getAllDevicesBreadthFirst(const protocol::NodeTree &rootNodeTree) {
            std::list<std::shared_ptr<PearNodeTree> > result;
            std::queue<std::shared_ptr<PearNodeTree> > queue;
            using namespace painlessmesh::logger;

            // Start with the children of the root, not the root itself
            for (const auto &child: rootNodeTree.subs) {
                Log(PEAR_DEBUG, "getAllDevicesBreadthFirst(): Adding node: %u (%i subs) to queue\n", child.nodeId,
                    child.subs.size());
                queue.push(std::make_shared<PearNodeTree>(std::make_shared<protocol::NodeTree>(child)));
            }

            while (!queue.empty()) {
                auto current = std::make_shared<PearNodeTree>(queue.front());
                queue.pop();

                Log(PEAR_DEBUG, "getAllDevicesBreadthFirst(): Adding node: %u to listOfAllDevices\n", current->nodeId);
                result.push_back(current);

                for (const auto &child: current->subs) {
                    const auto it = pearNodeTreeMap.find(child.nodeId);
                    if (it != pearNodeTreeMap.end()) {
                        queue.push(it->second);
                    } else {
                        Log(PEAR_DEBUG,
                            "getAllDevicesBreadthFirst(): Warning: child %u not found in map, using default-constructed PearNodeTree\n",
                            child.nodeId);
                        queue.push(std::make_shared<PearNodeTree>(std::make_shared<protocol::NodeTree>(child)));
                    }
                }
            }

            return result;
        }

        void setRootNodeId(const uint32_t nodeId) {
            rootNodeId = nodeId;
        }

    protected:
        Pear() {
        }

        uint32_t rootNodeId;
    };
}

#endif //_PAINLESS_MESH_PEAR_HPP_
