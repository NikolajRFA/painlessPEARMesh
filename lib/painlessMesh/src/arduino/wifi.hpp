#ifndef _PAINLESS_MESH_ARDUINO_WIFI_HPP_
#define _PAINLESS_MESH_ARDUINO_WIFI_HPP_

#include "painlessmesh/configuration.hpp"

#include "painlessmesh/logger.hpp"
#ifdef PAINLESSMESH_ENABLE_ARDUINO_WIFI
#include "painlessMeshSTA.h"

#include "painlessmesh/callback.hpp"
#include "painlessmesh/logger.hpp"
#include "painlessmesh/mesh.hpp"
#include "painlessmesh/router.hpp"
#include "painlessmesh/tcp.hpp"

extern painlessmesh::logger::LogClass Log;

namespace painlessmesh {
namespace wifi {
class Mesh : public painlessmesh::Mesh<Connection> {
 public:
  /** Initialize the mesh network
   *
   * Add this to your setup() function. This routine does the following things:
   *
   * - Starts a wifi network
   * - Begins searching for other wifi networks that are part of the mesh
   * - Logs on to the best mesh network node it finds… if it doesn’t find
   * anything, it starts a new search in 5 seconds.
   *
   * @param ssid The name of your mesh.  All nodes share same AP ssid. They are
   * distinguished by BSSID.
   * @param password Wifi password to your mesh.
   * @param port the TCP port that you want the mesh server to run on. Defaults
   * to 5555 if not specified.
   * @param connectMode Switch between WIFI_AP, WIFI_STA and WIFI_AP_STA
   * (default) mode
   */
  void init(TSTRING ssid, TSTRING password, uint16_t port = 5555,
            WiFiMode_t connectMode = WIFI_AP_STA, uint8_t channel = 1,
            uint8_t hidden = 0, uint8_t maxconn = MAX_CONN) {
    using namespace logger;
    // Init random generator seed to generate delay variance
    randomSeed(millis());

    // Shut Wifi down and start with a blank slage
    if (WiFi.status() != WL_DISCONNECTED) WiFi.disconnect();

    Log(STARTUP, "init(): %d\n",
#if ESP_ARDUINO_VERSION_MAJOR >= 3
        // Disable autoconnect
        WiFi.setAutoReconnect(false));
#else
        // Disable autoconnect
        WiFi.setAutoConnect(false));
#endif
    WiFi.persistent(false);

    // start configuration
    if (!WiFi.mode(connectMode)) {
      Log(GENERAL, "WiFi.mode() false");
    }

    _meshSSID = ssid;
    _meshPassword = password;
    _meshChannel = channel;
    _meshHidden = hidden;
    _meshMaxConn = maxconn;
    _meshPort = port;

    uint8_t MAC[] = {0, 0, 0, 0, 0, 0};
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    esp_read_mac(MAC, ESP_MAC_WIFI_SOFTAP);
#else
    if (WiFi.softAPmacAddress(MAC) == 0) {
      Log(ERROR, "init(): WiFi.softAPmacAddress(MAC) failed.\n");
    }
#endif
    Log(DEBUG, "My MAC is %x:%x:%x:%x:%x:%x\n", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
    uint32_t nodeId = tcp::encodeNodeId(MAC);
    if (nodeId == 0) Log(ERROR, "NodeId set to 0\n");

    this->init(nodeId);

    tcpServerInit();
    eventHandleInit();

    _apIp = IPAddress(0, 0, 0, 0);

    if (connectMode & WIFI_AP) {
      apInit(nodeId);  // setup AP
    }
    if (connectMode & WIFI_STA) {
      this->initStation();
    }
  }

  /** Initialize the mesh network
   *
   * Add this to your setup() function. This routine does the following things:
   *
   * - Starts a wifi network
   * - Begins searching for other wifi networks that are part of the mesh
   * - Logs on to the best mesh network node it finds… if it doesn’t find
   * anything, it starts a new search in 5 seconds.
   *
   * @param ssid The name of your mesh.  All nodes share same AP ssid. They are
   * distinguished by BSSID.
   * @param password Wifi password to your mesh.
   * @param port the TCP port that you want the mesh server to run on. Defaults
   * to 5555 if not specified.
   * @param connectMode Switch between WIFI_AP, WIFI_STA and WIFI_AP_STA
   * (default) mode
   */
  void init(TSTRING ssid, TSTRING password, Scheduler *baseScheduler,
            uint16_t port = 5555, WiFiMode_t connectMode = WIFI_AP_STA,
            uint8_t channel = 3, uint8_t hidden = 0,
            uint8_t maxconn = MAX_CONN) {
    this->setScheduler(baseScheduler);
    init(ssid, password, port, connectMode, channel, hidden, maxconn);
  }

  /**
   * Connect (as a station) to a specified network and ip
   *
   * You can pass {0,0,0,0} as IP to have it connect to the gateway
   *
   * This stops the node from scanning for other (non specified) nodes
   * and you should probably also use this node as an anchor: `setAnchor(true)`
   */
  void stationManual(TSTRING ssid, TSTRING password, uint16_t port = 0,
                     IPAddress remote_ip = IPAddress(0, 0, 0, 0)) {
    // Set station config
    stationScan.manualIP = remote_ip;

    // Start scan
    stationScan.init(this, ssid, password, port);
    stationScan.manual = true;
  }

  void setDontConnect(bool dontConnect) { stationScan.dontConnect = dontConnect; }

  void initStation() {
    stationScan.init(this, _meshSSID, _meshPassword, _meshPort);
    mScheduler->addTask(stationScan.task);
    stationScan.task.enable();

    this->droppedConnectionCallbacks.push_back(
        [this](uint32_t nodeId, bool station) {
          if (station) {
            if (WiFi.status() == WL_CONNECTED) WiFi.disconnect();
            this->stationScan.connectToAP();
          }
        });
  }

  void tcpServerInit() {
    using namespace logger;
    Log(GENERAL, "tcpServerInit():\n");
    _tcpListener = new AsyncServer(_meshPort);
    painlessmesh::tcp::initServer<Connection, painlessmesh::Mesh<Connection>>(
        (*_tcpListener), (*this));
    Log(STARTUP, "AP tcp server established on port %d\n", _meshPort);
    return;
  }

  void tcpConnect() {
    using namespace logger;
    // TODO: move to Connection or StationConnection?
    Log(GENERAL, "tcpConnect():\n");
    if (stationScan.manual && stationScan.port == 0)
      return;  // We have been configured not to connect to the mesh

    // TODO: We could pass this to tcpConnect instead of loading it here
    if (WiFi.status() == WL_CONNECTED && WiFi.localIP()) {
      AsyncClient *pConn = new AsyncClient();

      IPAddress ip = WiFi.gatewayIP();
      if (stationScan.manualIP) {
        ip = stationScan.manualIP;
      }

      painlessmesh::tcp::connect<Connection, painlessmesh::Mesh<Connection>>(
          (*pConn), ip, stationScan.port, (*this));
    } else {
      Log(ERROR, "tcpConnect(): err Something un expected in tcpConnect()\n");
    }
  }

  bool setHostname(const char *hostname) {
#ifdef ESP8266
    return WiFi.hostname(hostname);
#elif defined(ESP32)
    if (strlen(hostname) > 32) {
      return false;
    }
    return WiFi.setHostname(hostname);
#endif  // ESP8266
  }

  IPAddress getStationIP() { return WiFi.localIP(); }
  IPAddress getAPIP() { return _apIp; }

  void stop() {
    // remove all WiFi events
#ifdef ESP32
    WiFi.removeEvent(eventScanDoneHandler);
    WiFi.removeEvent(eventSTAStartHandler);
    WiFi.removeEvent(eventSTADisconnectedHandler);
    WiFi.removeEvent(eventSTAGotIPHandler);
#elif defined(ESP8266)
    eventSTAConnectedHandler = WiFiEventHandler();
    eventSTADisconnectedHandler = WiFiEventHandler();
    eventSTAGotIPHandler = WiFiEventHandler();

    stationScan.asyncTask.setCallback(NULL);
    mScheduler->deleteTask(stationScan.asyncTask);
#endif  // ESP32
    // Stop scanning task
    stationScan.task.setCallback(NULL);
    mScheduler->deleteTask(stationScan.task);
    painlessmesh::Mesh<Connection>::stop();

    // Shutdown wifi hardware
    if (WiFi.status() != WL_DISCONNECTED) WiFi.disconnect();

    // Delete the tcp server
    delete _tcpListener;
  }

 protected:
  friend class ::StationScan;
  TSTRING _meshSSID;
  TSTRING _meshPassword;
  uint8_t _meshChannel;
  uint8_t _meshHidden;
  uint8_t _meshMaxConn;
  uint16_t _meshPort;

  IPAddress _apIp;
  StationScan stationScan;

  void init(Scheduler *scheduler, uint32_t id) {
    painlessmesh::Mesh<Connection>::init(scheduler, id);
  }

  void init(uint32_t id) { painlessmesh::Mesh<Connection>::init(id); }

  void apInit(uint32_t nodeId) {
    _apIp = IPAddress(10, (nodeId & 0xFF00) >> 8, (nodeId & 0xFF), 1);
    IPAddress netmask(255, 255, 255, 0);

    WiFi.softAPConfig(_apIp, _apIp, netmask);
    WiFi.softAP(_meshSSID.c_str(), _meshPassword.c_str(), _meshChannel,
                _meshHidden, _meshMaxConn);
  }
  void eventHandleInit() {
    using namespace logger;
#ifdef ESP32
    eventScanDoneHandler = WiFi.onEvent(
        [this](WiFiEvent_t event, WiFiEventInfo_t info) {
          if (this->semaphoreTake()) {
            Log(DEBUG, "Semaphore taken by STA scan done\n");
            Log(CONNECTION,
                "eventScanDoneHandler: ARDUINO_EVENT_WIFI_SCAN_DONE\n");
            this->stationScan.scanComplete();
            Log(DEBUG, "Semaphore given by STA scan done\n");
            this->semaphoreGive();
          }
        },
#if ESP_ARDUINO_VERSION_MAJOR >= 2
        WiFiEvent_t::ARDUINO_EVENT_WIFI_SCAN_DONE);
#else
        WiFiEvent_t::SYSTEM_EVENT_SCAN_DONE);
#endif

    eventSTAStartHandler = WiFi.onEvent(
        [this](WiFiEvent_t event, WiFiEventInfo_t info) {
          if (this->semaphoreTake()) {
            Log(DEBUG, "Semaphore taken by STA Start\n");
            Log(CONNECTION,
                "eventSTAStartHandler: ARDUINO_EVENT_WIFI_STA_START\n");
            Log(DEBUG, "Semaphore given by STA Start\n");
            this->semaphoreGive();
          }
        },
#if ESP_ARDUINO_VERSION_MAJOR >= 2
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_START);
#else
        WiFiEvent_t::SYSTEM_EVENT_STA_START);
#endif

    eventSTADisconnectedHandler = WiFi.onEvent(
        [this](WiFiEvent_t event, WiFiEventInfo_t info) {
          if (this->semaphoreTake()) {
            Log(DEBUG, "Semaphore taken by STA disconnection\n");
            Log(CONNECTION,
                "eventSTADisconnectedHandler: "
                "ARDUINO_EVENT_WIFI_STA_DISCONNECTED\n");
            Log(ERROR, "disconnectd from %u\nreason: %i\n", tcp::encodeNodeId(info.wifi_sta_disconnected.bssid), info.wifi_sta_disconnected.reason);

            this->droppedConnectionCallbacks.execute(0, true);
            Log(DEBUG, "Semaphore given by STA disconnection\n");
            this->semaphoreGive();
          }
        },
#if ESP_ARDUINO_VERSION_MAJOR >= 2
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
#else
        WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
#endif

    eventSTAGotIPHandler = WiFi.onEvent(
        [this](WiFiEvent_t event, WiFiEventInfo_t info) {
          Log(CONNECTION, "Trying to create STA connection\n");
          if (this->semaphoreTake()) {
            Log(DEBUG, "Semaphore taken by STA connection\n");
            Log(CONNECTION,
                "eventSTAGotIPHandler: ARDUINO_EVENT_WIFI_STA_GOT_IP\n");
            this->tcpConnect();  // Connect to TCP port
            Log(DEBUG, "Semaphore given by STA connection\n");
            this->semaphoreGive();
          }
        },
#if ESP_ARDUINO_VERSION_MAJOR >= 2
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
#else
        WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
#endif

#elif defined(ESP8266)
    eventSTAConnectedHandler = WiFi.onStationModeConnected(
        [&](const WiFiEventStationModeConnected &event) {
          // Log(CONNECTION, "Event: Station Mode Connected to \"%s\"\n",
          // event.ssid.c_str());
          Log(CONNECTION, "Event: Station Mode Connected\n");
        });

    eventSTADisconnectedHandler = WiFi.onStationModeDisconnected(
        [&](const WiFiEventStationModeDisconnected &event) {
          Log(CONNECTION, "Event: Station Mode Disconnected\n");
          this->droppedConnectionCallbacks.execute(0, true);
        });

    eventSTAGotIPHandler =
        WiFi.onStationModeGotIP([&](const WiFiEventStationModeGotIP &event) {
          Log(CONNECTION,
              "Event: Station Mode Got IP (IP: %s  Mask: %s  Gateway: %s)\n",
              event.ip.toString().c_str(), event.mask.toString().c_str(),
              event.gw.toString().c_str());
          this->tcpConnect();  // Connect to TCP port
        });
#endif  // ESP32
    return;
  }

#ifdef ESP32
  WiFiEventId_t eventScanDoneHandler;
  WiFiEventId_t eventSTAStartHandler;
  WiFiEventId_t eventSTADisconnectedHandler;
  WiFiEventId_t eventSTAGotIPHandler;
#elif defined(ESP8266)
  WiFiEventHandler eventSTAConnectedHandler;
  WiFiEventHandler eventSTADisconnectedHandler;
  WiFiEventHandler eventSTAGotIPHandler;
#endif  // ESP8266
  AsyncServer *_tcpListener;
};
}  // namespace wifi
};  // namespace painlessmesh

#endif

#endif
