#ifndef _PAINLESS_MESH_ROUTER_HPP_
#define _PAINLESS_MESH_ROUTER_HPP_

#include <algorithm>
#include <map>

#include "painlessmesh/callback.hpp"
#include "painlessmesh/layout.hpp"
#include "painlessmesh/logger.hpp"
#include "painlessmesh/protocol.hpp"

extern painlessmesh::logger::LogClass Log;

namespace painlessmesh {
    /**
     * Helper functions to route messages
     */
    namespace router {
        template<class T>
        std::shared_ptr<T> findRoute(layout::Layout<T> tree,
                                     std::function<bool(std::shared_ptr<T>)> func) {
            auto route = std::find_if(tree.subs.begin(), tree.subs.end(), func);
            if (route == tree.subs.end()) return NULL;
            return (*route);
        }

        template<class T>
        std::shared_ptr<T> findRoute(layout::Layout<T> tree, uint32_t nodeId) {
            return findRoute<T>(tree, [nodeId](std::shared_ptr<T> s) {
                return layout::contains((*s), nodeId);
            });
        }

        template<class T, class U>
        bool send(T package, std::shared_ptr<U> conn, bool priority = false) {
            auto variant = painlessmesh::protocol::Variant(package);
            TSTRING msg;
            variant.printTo(msg);
            return conn->addMessage(msg, priority);
        }

        template<class U>
        bool send(protocol::Variant variant, std::shared_ptr<U> conn,
                  bool priority = false) {
            TSTRING msg;
            variant.printTo(msg);
            return conn->addMessage(msg, priority);
        }

        template<class T, class U>
        bool send(T package, layout::Layout<U> layout) {
            auto variant = painlessmesh::protocol::Variant(package);
            TSTRING msg;
            variant.printTo(msg);
            auto conn = findRoute<U>(layout, variant.dest());
            if (conn) return conn->addMessage(msg);
            return false;
        }

        template<class U>
        bool send(protocol::Variant variant, layout::Layout<U> layout) {
            TSTRING msg;
            variant.printTo(msg);
            auto conn = findRoute<U>(layout, variant.dest());
            if (conn) return conn->addMessage(msg);
            return false;
        }

        template<class T, class U>
        size_t broadcast(T package, layout::Layout<U> layout, uint32_t exclude) {
            auto variant = painlessmesh::protocol::Variant(package);
            TSTRING msg;
            variant.printTo(msg);
            size_t i = 0;
            for (auto &&conn: layout.subs) {
                if (conn->nodeId != 0 && conn->nodeId != exclude) {
                    auto sent = conn->addMessage(msg);
                    if (sent) ++i;
                }
            }
            return i;
        }

        template<class T>
        size_t broadcast(protocol::Variant variant, layout::Layout<T> layout,
                         uint32_t exclude) {
            TSTRING msg;
            variant.printTo(msg);
            size_t i = 0;
            for (auto &&conn: layout.subs) {
                if (conn->nodeId != 0 && conn->nodeId != exclude) {
                    auto sent = conn->addMessage(msg);
                    if (sent) ++i;
                }
            }
            return i;
        }

        template<class T>
        void routePackage(layout::Layout<T> layout, std::shared_ptr<T> connection,
                          TSTRING pkg, callback::MeshPackageCallbackList<T> cbl, uint32_t receivedAt) {
            using namespace logger;

            static size_t baseCapacity = 512;
            Log(COMMUNICATION, "routePackage(): Recvd from %u: %s\n", connection->nodeId,
                pkg.c_str());
            // Using a ptr so we can overwrite it if we need to grow capacity.
            // Bug in copy constructor with grown capacity can cause segmentation fault
            auto variant =
                    std::make_shared<protocol::Variant>(pkg, pkg.length() + baseCapacity);
            while (variant->error == DeserializationError::NoMemory && baseCapacity <= 20480) {
                // Not enough memory, adapt scaling (variant::capacityScaling) and log the
                // new value
                Log(DEBUG,
                    "routePackage(): parsing failed. err=%u, increasing capacity: %u\n",
                    variant->error, baseCapacity);
                baseCapacity += 256;
                variant =
                        std::make_shared<protocol::Variant>(pkg, pkg.length() + baseCapacity);
            }
            if (variant->error) {
                Log(ERROR,
                    "routePackage(): parsing failed. err=%u, total_length=%d, data=%s<--\n",
                    variant->error, pkg.length(), pkg.c_str());
                return;
            }
            if (!variant) {
                Log(ERROR, "routePackage(): Variant is null!");
                return;
            }

            if ((variant->routing() == SINGLE) && variant->dest() != layout.getNodeId()) {
                // Send on without further processing
                send<T>((*variant), layout);
                return;
            } else if (variant->routing() == BROADCAST) {
                broadcast<T>((*variant), layout, connection->nodeId);
            }

            auto calls = cbl.execute(variant->type(), (*variant), connection, receivedAt);
            if (calls == 0)
                Log(DEBUG, "routePackage(): No callbacks executed; %u, %s\n", variant->type(), pkg.c_str());
        }

        template<class T, class U>
        void handleNodeSync(T &mesh, protocol::NodeTree newTree,
                            std::shared_ptr<U> conn) {
            Log(logger::SYNC, "handleNodeSync(): with %u\n", conn->nodeId);

            if (!conn->validSubs(newTree)) {
                Log(logger::SYNC, "handleNodeSync(): invalid new connection\n");
                conn->close();
                return;
            }

            if (conn->newConnection) {
                auto oldConnection = router::findRoute<U>(mesh, newTree.nodeId);
                if (oldConnection) {
                    Log(logger::SYNC,
                        "handleNodeSync(): already connected to %u. Closing the new "
                        "connection \n",
                        conn->nodeId);
                    conn->close();
                    return;
                }
                auto remoteNodeId = newTree.nodeId;
                mesh.addTask([&mesh, remoteNodeId]() {
                    Log(logger::CONNECTION, "newConnectionTask():\n");
                    Log(logger::CONNECTION, "newConnectionTask(): adding %u now= %u\n",
                        remoteNodeId, mesh.getNodeTime());
                    mesh.newConnectionCallbacks.execute(remoteNodeId);
                });

                // Initially interval is every 10 seconds,
                // this will slow down to TIME_SYNC_INTERVAL
                // after first succesfull sync
                // TODO move it to a new connection callback and use initTimeSync from
                // ntp.hpp
                conn->timeSyncTask.set(10 * TASK_SECOND, TASK_FOREVER, [conn, &mesh]() {
                    Log(logger::S_TIME, "timeSyncTask(): %u\n", conn->nodeId);
                    mesh.startTimeSync(conn);
                });
                mesh.mScheduler->addTask(conn->timeSyncTask);
                if (conn->station)
                    // We are STA, request time immediately
                    conn->timeSyncTask.enable();
                else
                    // We are the AP, give STA the change to initiate time sync
                    conn->timeSyncTask.enableDelayed();
                conn->newConnection = false;
            }

            if (conn->updateSubs(newTree)) {
                auto nodeId = newTree.nodeId;
                mesh.addTask([&mesh, nodeId]() {
                    mesh.changedConnectionCallbacks.execute(nodeId);
                });
            } else {
                conn->nodeSyncTask.delay();
                mesh.stability += std::min(1000 - mesh.stability, (size_t) 25);
            }
        }

        template<class T, typename U>
        callback::MeshPackageCallbackList<U> addPackageCallback(
            callback::MeshPackageCallbackList<U> &&callbackList, T &mesh) {
            // REQUEST type,
            callbackList.onPackage(
                protocol::NODE_SYNC_REQUEST,
                [&mesh](protocol::Variant variant, std::shared_ptr<U> connection,
                        uint32_t receivedAt) {
                    auto newTree = variant.to<protocol::NodeSyncRequest>();
                    handleNodeSync<T, U>(mesh, newTree, connection);
                    send<protocol::NodeSyncReply>(
                        connection->reply(std::move(mesh.asNodeTree())), connection, true);
                    return false;
                });

            // Reply type just handle it
            callbackList.onPackage(
                protocol::NODE_SYNC_REPLY,
                [&mesh](protocol::Variant variant, std::shared_ptr<U> connection,
                        uint32_t receivedAt) {
                    auto newTree = variant.to<protocol::NodeSyncReply>();
                    handleNodeSync<T, U>(mesh, newTree, connection);
                    connection->timeOutTask.disable();
                    return false;
                });

            return callbackList;
        }
    } // namespace router
} // namespace painlessmesh

#endif
