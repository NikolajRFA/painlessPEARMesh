#ifndef _PAINLESS_MESH_PEAR_HPP_
#define _PAINLESS_MESH_PEAR_HPP_

#include <Arduino.h>
#include <set>
#include <list>
#include <queue>
#include <unordered_map>
#include <map>

#include "protocol.hpp"

namespace painlessmesh {
    class PearNodeTree : public protocol::NodeTree {
    public:
        int periodTx = 0;
        int periodRx = 0;
        std::list<PearNodeTree> parentCandidates;
        int txThreshold = 150;
        int rxThreshold = 100;
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


        PearNodeTree() {
        }

        PearNodeTree(const NodeTree &nodeTree) {
            this->nodeId = nodeTree.nodeId;
            this->root = nodeTree.root;
            this->subs = nodeTree.subs;
        }

        PearNodeTree(const NodeTree &nodeTree, const int periodTx, const int periodRx,
                     const std::list<PearNodeTree> parentCandidates) {
            this->nodeId = nodeTree.nodeId;
            this->root = nodeTree.root;
            this->subs = nodeTree.subs;
            this->periodRx = periodRx;
            this->periodTx = periodTx;
            this->parentCandidates = parentCandidates;
        }
    };

    class Pear {
    public:
        uint8_t noOfVerifiedDevices = 0;
        std::map<uint32_t, PearNodeTree> pearNodeTreeMap;
        std::unordered_map<uint32_t, uint32_t> reroutes; // Create logic to send the reroute information

        Pear() {
        }

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
        void run(protocol::NodeTree &rootNodeTree) {
            auto listOfAllDevices = getAllDevicesBreadthFirst(rootNodeTree);
            for (auto pearNodeTree: listOfAllDevices) {
                if (noOfVerifiedDevices < 10) {
                    if (deviceExceedsLimit(pearNodeTree.nodeId)) updateParent(pearNodeTree);
                }
                return;
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
        bool deviceExceedsThreshold(const PearNodeTree pearNodeTree) {
            return pearNodeTree.periodRx > pearNodeTree.rxThreshold && pearNodeTree.periodTx > pearNodeTree.txThreshold;
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
        bool deviceExceedsLimit(uint32_t deviceId) {
            //auto pearNodeTree = findPearNodeTreeById(deviceId);
            const auto it = pearNodeTreeMap.find(deviceId);
            if (it == pearNodeTreeMap.end()) return false;
            const auto pearNodeTree = it->second;
            if (deviceExceedsThreshold(pearNodeTree)) {
                return true;
            }
            noOfVerifiedDevices++;
            return false;
        }

        /**
         * @brief Evaluates and updates potential parent nodes for the given PearNodeTree based on transmission priorities and thresholds.
         *
         * This method iterates over the subnodes of the given `pearNodeTree`, collects valid subnodes from the global
         * `pearNodeTreeMap`, and sorts them into a set (`descendingTxList`). For each subnode, if it has available parent candidates,
         * the method attempts to reroute its parent assignment to the first candidate that does not exceed a defined threshold.
         *
         * Successful reroutes are recorded in the global `reroutes` map, where the key is the node ID of the child being rerouted,
         * and the value is the node ID of the new parent candidate.
         *
         * Nodes are only rerouted to the first eligible parent candidate that does not exceed the threshold.
         * No further candidates are evaluated once a valid one is found.
         *
         * @param pearNodeTree The `PearNodeTree` whose subnodes are evaluated for potential parent updates.
         *
         * @note This function assumes:
         * - `pearNodeTreeMap` is a globally accessible mapping from node IDs to `PearNodeTree` objects.
         * - `deviceExceedsThreshold(candidate)` is a predicate function used to check if a candidate node is eligible.
         * - `reroutes` is a globally accessible map used to track rerouted parent relationships.
         */
        void updateParent(PearNodeTree &pearNodeTree) {
            std::set<PearNodeTree> descendingTxList;
            for (auto sub: pearNodeTree.subs) {
                const auto it = pearNodeTreeMap.find(sub.nodeId);
                if (it == pearNodeTreeMap.end()) break;
                const auto pearNodeTree = it->second;
                descendingTxList.insert(pearNodeTree);
            }
            for (auto nodeToReroute: descendingTxList) {
                if (!nodeToReroute.parentCandidates.empty()) {
                    for (auto candidate: nodeToReroute.parentCandidates) {
                        if (deviceExceedsThreshold(candidate)) break;
                        reroutes.insert({nodeToReroute.nodeId, candidate.nodeId});
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
        void processReceivedData(JsonDocument &pearData, protocol::NodeTree &nodeTree) {
            int periodTx = pearData["periodTx"];
            int periodRx = pearData["periodRx"];
            auto parentCandidatesJsonArray = pearData["parentCandidates"].as<JsonArray>();
            std::list<PearNodeTree> parentCandidates;
            for (uint32_t id: parentCandidatesJsonArray) {
                const auto it = pearNodeTreeMap.find(id);
                if (it == pearNodeTreeMap.end()) {
                    const auto missingNode = layout::getNodeById(nodeTree, id);
                    pearNodeTreeMap.insert({id, missingNode});
                    parentCandidates.push_back(PearNodeTree(missingNode));
                } else {
                    const auto pearNodeTree = it->second;
                    parentCandidates.push_back(pearNodeTree);
                }
            }

            if (pearNodeTreeMap.count(nodeTree.nodeId)) {
                auto foundPearNodeTree = pearNodeTreeMap[nodeTree.nodeId];
                foundPearNodeTree.periodTx = periodTx;
                foundPearNodeTree.periodRx = periodRx;
                foundPearNodeTree.parentCandidates = parentCandidates;
                foundPearNodeTree.subs = nodeTree.subs;
            } else {
                pearNodeTreeMap.insert({nodeTree.nodeId, PearNodeTree(nodeTree, periodTx, periodRx, parentCandidates)});
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
        std::list<PearNodeTree> getAllDevicesBreadthFirst(const protocol::NodeTree &rootNodeTree) {
            std::list<PearNodeTree> result;
            std::queue<PearNodeTree> queue;

            queue.push(rootNodeTree);

            while (!queue.empty()) {
                PearNodeTree current = queue.front();
                queue.pop();

                result.push_back(current);

                for (const auto &child: current.subs) {
                    queue.push(PearNodeTree(child)); // Convert NodeTree to PearNodeTree if needed
                }
            }

            return result;
        }
    };
}

#endif //_PAINLESS_MESH_PEAR_HPP_
