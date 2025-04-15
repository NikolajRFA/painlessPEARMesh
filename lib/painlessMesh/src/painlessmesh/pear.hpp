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
        std::set<PearNodeTree> subsDescendingTx;

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
            for (const auto &sub: nodeTree.subs) {
                subsDescendingTx.insert(PearNodeTree(sub));
            }
        }

    private:
    };

    class Pear {
    public:
        uint8_t noOfVerifiedDevices = 0;
        //uint8_t energyProfile = 200; // Why 200 you might ask...
        std::vector<PearNodeTree> pearNodeTrees;

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

        void updateParent(uint32_t deviceId) {
            // get the device using the id
            // create a descending list of subs sorted by transmission - nodesToReroute
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
