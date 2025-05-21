#ifndef _PAINLESS_MESH_TCP_HPP_
#define _PAINLESS_MESH_TCP_HPP_

#include <list>

#include "Arduino.h"
#include "painlessmesh/configuration.hpp"

#include "painlessmesh/logger.hpp"

namespace painlessmesh {
namespace tcp {
inline uint32_t encodeNodeId(const uint8_t *hwaddr) {
  using namespace painlessmesh::logger;
  Log(GENERAL, "encodeNodeId():\n");
  uint32_t value = 0;

  value |= hwaddr[2] << 24;  // Big endian (aka "network order"):
  value |= hwaddr[3] << 16;
  value |= hwaddr[4] << 8;
  value |= hwaddr[5];
  return value;
}

inline void decodeNodeId(uint32_t encodedValue, uint8_t* hwaddr)
{
  using namespace painlessmesh::logger;
  Log(GENERAL, "decodeNodeId():\n");

  hwaddr[0] = 0x00; // First two address bytes are not taken into account in nodeIds.
  hwaddr[1] = 0x00;
  hwaddr[2] = (encodedValue >> 24) & 0xFF;
  hwaddr[3] = (encodedValue >> 16) & 0xFF;
  hwaddr[4] = (encodedValue >> 8) & 0xFF;
  hwaddr[5] = encodedValue & 0xFF;
}

template <class T, class M>
void initServer(AsyncServer &server, M &mesh) {
  using namespace logger;
  server.setNoDelay(true);

  server.onClient(
      [&mesh](void *arg, AsyncClient *client) {
        if (mesh.semaphoreTake()) {
          Log(DEBUG, "Semaphore taken by AP connection\n");
          Log(CONNECTION, "New AP connection incoming\n");
          auto conn = std::make_shared<T>(client, &mesh, false);
          conn->initTasks();
          mesh.subs.push_back(conn);
          mesh.semaphoreGive();
          Log(DEBUG, "Semaphore given by AP connection\n");
        }
      },
      NULL);
  server.begin();
}

template <class T, class M>
void connect(AsyncClient &client, IPAddress ip, uint16_t port, M &mesh) {
  using namespace logger;
  client.onError([&mesh](void *, AsyncClient *client, int8_t err) {
    if (mesh.semaphoreTake()) {
      Log(DEBUG, "Semaphore taken by tcp_err()\n");
      Log(CONNECTION, "tcp_err(): error trying to connect %d\n", err);
      mesh.droppedConnectionCallbacks.execute(0, true);
      Log(DEBUG, "Semaphore given by tcp_err()\n");
      mesh.semaphoreGive();
    }
  });

  client.onConnect(
      [&mesh](void *, AsyncClient *client) {
        if (mesh.semaphoreTake()) {
          Log(DEBUG, "Semaphore taken by tcp_connect()\n");
          Log(CONNECTION, "New STA connection incoming\n");
          auto conn = std::make_shared<T>(client, &mesh, true);
          conn->initTasks();
          mesh.subs.push_back(conn);
          mesh.setStationId(conn->nodeId);
          Log(DEBUG, "Semaphore given by tcp_connect()\n");
          mesh.semaphoreGive();
        }
      },
      NULL);

  Log(CONNECTION, "Connecting to %s:%d\n", ip.toString().c_str(), port);
  client.connect(ip, port);
}
}  // namespace tcp
}  // namespace painlessmesh
#endif
