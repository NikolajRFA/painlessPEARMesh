#ifndef _PAINLESS_MESH_PEAR_HPP_
#define _PAINLESS_MESH_PEAR_HPP_

#include <Arduino.h>
#include <set>
#include <list>
#include <queue>
#include <unordered_map>

#include "protocol.hpp"

namespace painlessmesh {
    class PearNodeTree : public protocol::NodeTree {
    public:
        int periodTx = 0;
        int periodRx = 0;
        std::list<PearNodeTree> parentCandidates;
        int txThreshold = 200;
        int rxThreshold = 150;
        int energyProfile = (txThreshold + rxThreshold) / 2;

        // Define the < operator for comparison
        bool operator<(const PearNodeTree &other) const {
            return periodRx > other.periodRx;
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
        std::unordered_map<uint32_t, uint32_t> reroutes;

        Pear() {
        }

        void run(protocol::NodeTree &rootNodeTree) {
            auto listOfAllDevices = getAllDevicesBreadthFirst(rootNodeTree);
            for (auto pearNodeTree: listOfAllDevices) {
                if (noOfVerifiedDevices < 10) {
                    if (deviceExceedsLimit(pearNodeTree.nodeId)) updateParent(pearNodeTree);
                }
                return;
            }
        }

        bool deviceExceedsThreshold(const PearNodeTree pearNodeTree) {
            return pearNodeTree.periodRx > pearNodeTree.rxThreshold && pearNodeTree.periodTx > pearNodeTree.txThreshold;
        }

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

        void updateParent(PearNodeTree &pearNodeTree) {
            std::set<PearNodeTree> descendingTxList;
            for (auto sub: pearNodeTree.subs) {
                const auto it = pearNodeTreeMap.find(sub.nodeId);
                if (it == pearNodeTreeMap.end()) break;
                const auto pearNodeTree = it->second;
                descendingTxList.insert(pearNodeTree);
            }
            for (auto nodeToReroute: descendingTxList) {
                if (nodeToReroute.parentCandidates.size() > 0) {
                    for (auto candidate: nodeToReroute.parentCandidates) {
                        if (deviceExceedsThreshold(candidate)) break;
                        reroutes.insert({nodeToReroute.nodeId, candidate.nodeId});
                    }
                }
            }
        }

        void processReceivedData(JsonDocument &pearData, protocol::NodeTree &nodeTree) {
            int periodTx = pearData["periodTx"];
            int periodRx = pearData["periodRx"];
            auto parentCandidatesJsonArray = pearData["parentCandidates"].as<JsonArray>();
            std::list<PearNodeTree> parentCandidates;
            for (uint32_t id: parentCandidatesJsonArray) {
                const auto it = pearNodeTreeMap.find(id);
                if (it == pearNodeTreeMap.end()) {
                    const auto nodeTree123 = layout::getNodeById(nodeTree, id);
                    pearNodeTreeMap.insert({id, nodeTree123});
                    parentCandidates.push_back(PearNodeTree(nodeTree123));
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
            } else {
                pearNodeTreeMap.insert({nodeTree.nodeId, PearNodeTree(nodeTree, periodTx, periodRx, parentCandidates)});
            }
        }

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
