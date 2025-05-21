#ifndef _PAINLESS_MESH_STA_H_
#define _PAINLESS_MESH_STA_H_

#include "painlessmesh/configuration.hpp"

#include "painlessmesh/mesh.hpp"

#include <list>

typedef struct
{
  uint8_t bssid[6];
  TSTRING ssid;
  int8_t rssi;
} WiFi_AP_Record_t;

class StationScan
{
public:
  Task task; // Station scanning for connections

#ifdef ESP8266
  Task asyncTask;
#endif

  StationScan() {}
  void init(painlessmesh::wifi::Mesh *pMesh, TSTRING ssid, TSTRING password,
            uint16_t port);
  void stationScan();
  void scanComplete();
  void filterAPs();
  bool checkStation();

  uint32_t getStationIdFromSubs() const;

  void connectToAP();
  static bool compareWiFiAPRecords(const WiFi_AP_Record_t& a, const WiFi_AP_Record_t& b, bool useTargetNodeId, uint32_t targetNodeId);
  static bool containsTargetNodeId(const std::list<WiFi_AP_Record_t>& aps, uint32_t targetNodeId);
  
  /// Valid APs found during the last scan
  std::list<WiFi_AP_Record_t> lastAPs;

protected:
  TSTRING ssid;
  TSTRING password;
  painlessMesh *mesh;
  uint16_t port;
  std::list<WiFi_AP_Record_t> aps;
  static std::shared_ptr<WiFi_AP_Record_t> targetRecord;

  void requestIP(WiFi_AP_Record_t &ap);

  void removeStationFromAvailableNetworksIfInNodeSubs() const;

  bool dontConnect = false;
  // Manually configure network and ip
  bool manual = false;
  IPAddress manualIP = IPAddress(0, 0, 0, 0);

  friend painlessMesh;
};

#endif
