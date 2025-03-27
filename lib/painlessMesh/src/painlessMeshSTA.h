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
  void connectToAP();
  static bool compareWiFiAPRecords(WiFi_AP_Record_t a, WiFi_AP_Record_t b, bool useTargetBSSID, const uint8_t* targetBSSID);

  /// Valid APs found during the last scan
  std::list<WiFi_AP_Record_t> lastAPs;

  void setTargetBSSID(const uint8_t *bssid);
  void clearTargetBSSID();

protected:
  TSTRING ssid;
  TSTRING password;
  painlessMesh *mesh;
  uint16_t port;
  std::list<WiFi_AP_Record_t> aps;
  uint8_t targetBSSID[6] = {0}; // Default to an invalid BSSID
  bool useTargetBSSID = false;  // Flag to enable/disable targeting a specific BSSID

  void requestIP(WiFi_AP_Record_t &ap);

  // Manually configure network and ip
  bool manual = false;
  IPAddress manualIP = IPAddress(0, 0, 0, 0);

  friend painlessMesh;
};

#endif
