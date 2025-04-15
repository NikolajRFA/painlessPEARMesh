#ifndef _PAINLESS_MESH_PEAR_HPP_
#define _PAINLESS_MESH_PEAR_HPP_

#include <Arduino.h>
#include <set>
#include <vector>
#include <list>
#include "protocol.hpp"

namespace painlessmesh {
    class PearNodeTree : public protocol::NodeTree {
    public:
        int periodTx = 0;
        int periodRx = 0;
        std::list<uint32_t> parentCandidates;
        int energyProfile = 200;

        // Define the < operator for comparison
        bool operator<(const PearNodeTree &other) const {
            return periodRx > other.periodRx;
        }

        PearNodeTree() {
        }

        PearNodeTree(const NodeTree& nodeTree) {
            this->nodeId = nodeTree.nodeId;
            this->root = nodeTree.root;
            this->subs = nodeTree.subs;
        }

        PearNodeTree(const NodeTree &nodeTree, const int periodTx, const int periodRx,
                     const std::list<uint32_t> parentCandidates) {
            this->nodeId = nodeTree.nodeId;
            this->root = nodeTree.root;
            this->subs = nodeTree.subs;
            this->periodRx = periodRx;
            this->periodTx = periodTx;
            this->parentCandidates = parentCandidates;
        }

    private:
    };

    class Pear {
    public:
        uint8_t noOfVerifiedDevices = 0;
        //uint8_t energyProfile = 200; // Why 200 you might ask...
        std::vector<PearNodeTree> pearNodeTrees; // TODO: Can this be a map

        Pear() {
        }

        void run() {
            // Create a list of all devices - devices
            // for each device in devices:
            if (noOfVerifiedDevices < 10) {
                // if(deviceExceedsLimit(device)) updateParent(device);
            }
        }

        bool deviceExceedsLimit(uint32_t deviceId) {
            // get the device using the id
            // if(device.transmissions > device.threshold) return true;
            noOfVerifiedDevices++;
            return false;
        }

        PearNodeTree findPearNodeTreeById(uint32_t targetNodeId) {
            auto it = std::find_if(pearNodeTrees.begin(), pearNodeTrees.end(),
                                   [targetNodeId](const PearNodeTree& node) {
                                       return node.nodeId == targetNodeId;
                                   });

            if (it != pearNodeTrees.end()) {
                return *it; // Return a copy
            } else {
                throw std::runtime_error("PearNodeTree with nodeId " + std::to_string(targetNodeId) + " not found.");
            }
        }


        void updateParent(PearNodeTree& pearNodeTree) {
            // get the device using the id
            // create a descending list of subs sorted by transmission - nodesToReroute
            std::set<PearNodeTree> descendingTxList;
            for (auto sub: pearNodeTree.subs) {
                descendingTxList.insert(findPearNodeTreeById(sub.nodeId));
            }
            // for each nodeToReroute:
            // for each visibleNode in nodeToReroute's visible nodes (getAvailableNetworks):
            // if visibleNode is within limits set nodeToReroute.newParent = visibleNode
        }

        void processReceivedData(JsonDocument &pearData, const protocol::NodeTree &nodeTree) {
            const auto it = std::find(pearNodeTrees.begin(), pearNodeTrees.end(), nodeTree);
            int periodTx = pearData["periodTx"];
            int periodRx = pearData["periodRx"];
            auto parentCandidatesJsonArray = pearData["parentCandidates"].as<JsonArray>();
            std::list<uint32_t> parentCandidates;
            for (uint32_t id: parentCandidatesJsonArray) {
                parentCandidates.push_back(id);
            }

            if (it != pearNodeTrees.end()) {
                // pearNodeTree found - update
                PearNodeTree &foundNode = *it;
                foundNode.periodTx = periodTx;
                foundNode.periodRx = periodRx;
                foundNode.parentCandidates = parentCandidates;
            } else {
                // pearNodeTree not found - add
                pearNodeTrees.push_back(PearNodeTree(nodeTree, periodTx, periodRx, parentCandidates));
            }
        }
    };
}

#endif //_PAINLESS_MESH_PEAR_HPP_
