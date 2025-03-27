//
//  painlessMeshSTA.cpp
//
//
//  Created by Bill Gray on 7/26/16.
//
//
#include "painlessmesh/configuration.hpp"

#ifdef PAINLESSMESH_ENABLE_ARDUINO_WIFI

#include <Arduino.h>
#include <algorithm>
#include <memory>

#include "arduino/wifi.hpp"

#include "painlessmesh/layout.hpp"
#include "painlessmesh/tcp.hpp"

extern painlessmesh::logger::LogClass Log;

//***********************************************************************
// Calculate NodeID from a hardware MAC address
void ICACHE_FLASH_ATTR StationScan::init(painlessmesh::wifi::Mesh *pMesh,
                                         TSTRING pssid, TSTRING ppassword,
                                         uint16_t pport) {
  ssid = pssid;
  password = ppassword;
  mesh = pMesh;
  port = pport;

  task.set(SCAN_INTERVAL, TASK_FOREVER, [this]() { stationScan(); });
}

// Starts scan for APs whose name is Mesh SSID
void ICACHE_FLASH_ATTR StationScan::stationScan() {
  using namespace painlessmesh::logger;
  Log(CONNECTION, "stationScan(): %s\n", ssid.c_str());

#ifdef ESP32
  WiFi.scanNetworks(true, true, false, 300U,  0);
#elif defined(ESP8266)
  // WiFi.scanNetworksAsync([&](int networks) { this->scanComplete(); }, true);
  // Try 600 times (60 seconds). If not completed after that, give up
  asyncTask.set(100 * TASK_MILLISECOND, 600, [this]() {
    auto num = WiFi.scanComplete();
    if (num == WIFI_SCAN_FAILED || num > 0) {
      this->asyncTask.disable();
      this->scanComplete();
    }
  });
  mesh->mScheduler->addTask(asyncTask);
  asyncTask.enableDelayed();
  WiFi.scanNetworks(true, true, 0);
#endif

  task.delay(10 * SCAN_INTERVAL);  // Scan should be completed by then and next
                                   // step called. If not then we restart here.
  return;
}

void ICACHE_FLASH_ATTR StationScan::scanComplete() {
  using namespace painlessmesh::logger;
  Log(CONNECTION, "scanComplete(): Scan finished\n");

  aps.clear();
  Log(CONNECTION, "scanComplete():-- > Cleared old APs.\n");

  auto num = WiFi.scanComplete();
  if (num == WIFI_SCAN_FAILED) {
    Log(ERROR, "wifi scan failed. Retrying....\n");
    task.forceNextIteration();
    return;
  } else if (num == WIFI_SCAN_RUNNING) {
    Log(ERROR,
        "scanComplete should never be called when scan is still running.\n");
    return;
  }

  Log(CONNECTION, "scanComplete(): num = %d\n", num);

  for (auto i = 0; i < num; ++i) {
    WiFi_AP_Record_t record;
    record.ssid = WiFi.SSID(i);

    if(WiFi.channel(i) != mesh->_meshChannel){
      continue;
    }

    if (record.ssid != ssid) {
      if (record.ssid.equals("") && mesh->_meshHidden) {
        // Hidden mesh
        record.ssid = ssid;
      } else {
        continue;
      }
    }

    record.rssi = WiFi.RSSI(i);
    if (record.rssi == 0) continue;

    memcpy((void *)&record.bssid, (void *)WiFi.BSSID(i), sizeof(record.bssid));
    aps.push_back(record);
    char macBuffer[17];
    sprintf(macBuffer, "%02x:%02x:%02x:%02x:%02x:%02x", record.bssid[0], record.bssid[1], record.bssid[2], record.bssid[3], record.bssid[4], record.bssid[5]);
    Log(CONNECTION, "\tfound : %s, %ddBm, bssid: %s\n", record.ssid.c_str(),
        (int16_t)record.rssi, macBuffer);
  }

  Log(CONNECTION, "\tFound %d nodes\n", aps.size());

  task.yield([this]() {
    bool targetBSSIDFound = false;
    if (useTargetBSSID) 
    {
      targetBSSIDFound = containsTargetBSSID(aps, targetBSSID);
      if (targetBSSIDFound) {
        Log(CONNECTION, "Target BSSID was found\n");
      }
      else 
      {
        Log(CONNECTION, "Target BSSID was not found\n");
      }
    }
    // Task filter all unknown
    if(!targetBSSIDFound) filterAPs();

    lastAPs = aps;

    // Next task is to sort by strength
    task.yield([this] {

      aps.sort([this](WiFi_AP_Record_t a, WiFi_AP_Record_t b) {
        return compareWiFiAPRecords(a, b, useTargetBSSID, targetBSSID);
      });
      // Next task is to connect to the top ap
      task.yield([this]() { connectToAP(); });
    });
  });
}

bool StationScan::containsTargetBSSID(const std::list<WiFi_AP_Record_t> &aps, const uint8_t *targetBSSID)
{
  using namespace painlessmesh::logger;
  Log(DEBUG, "Target BSSID: %i:%i:%i:%i:%i:%i\n", targetBSSID[0], targetBSSID[1], targetBSSID[2], targetBSSID[3], targetBSSID[4], targetBSSID[5]);
  for (const auto &ap : aps)
  {
    Log(DEBUG, "Current BSSID: %i:%i:%i:%i:%i:%i\n", ap.bssid[0], ap.bssid[1], ap.bssid[2], ap.bssid[3], ap.bssid[4], ap.bssid[5]);
    if (memcmp(ap.bssid, targetBSSID, sizeof(ap.bssid)) == 0)
    {
      return true; // Found the target BSSID
    }
  }
  return false; // Target BSSID not found
}

void ICACHE_FLASH_ATTR StationScan::filterAPs() {
  auto ap = aps.begin();
  while (ap != aps.end()) {
    auto apNodeId = painlessmesh::tcp::encodeNodeId(ap->bssid);
    if (painlessmesh::router::findRoute<painlessmesh::Connection>(
            (*mesh), apNodeId) != NULL) {
      ap = aps.erase(ap);
    } else {
      ap++;
    }
  }
}

bool ICACHE_FLASH_ATTR StationScan::compareWiFiAPRecords(WiFi_AP_Record_t a, WiFi_AP_Record_t b, bool useTargetBSSID, const uint8_t* targetBSSID) {
  if (useTargetBSSID) {
      if (memcmp(a.bssid, targetBSSID, sizeof(a.bssid)) == 0) return true;
      if (memcmp(b.bssid, targetBSSID, sizeof(b.bssid)) == 0) return false;
  }
  return a.rssi > b.rssi;
}

void ICACHE_FLASH_ATTR StationScan::requestIP(WiFi_AP_Record_t &ap) {
  using namespace painlessmesh::logger;
  Log(CONNECTION, "connectToAP(): Best AP is %u<---\n",
      painlessmesh::tcp::encodeNodeId(ap.bssid));
  WiFi.begin(ap.ssid.c_str(), password.c_str(), mesh->_meshChannel, ap.bssid);
  return;
}

void ICACHE_FLASH_ATTR StationScan::connectToAP() {
  using namespace painlessmesh;
  using namespace painlessmesh::logger;
  // Next task will be to rescan
  task.setCallback([this]() { stationScan(); });

  if (manual) {
    if ((WiFi.SSID() == ssid) && WiFi.status() == WL_CONNECTED) {
      Log(CONNECTION,
          "connectToAP(): Already connected using manual connection. "
          "Disabling scanning.\n");
      task.disable();
      return;
    } else {
      if (WiFi.status() == WL_CONNECTED) {
        mesh->closeConnectionSTA();
        task.enableDelayed(10 * SCAN_INTERVAL);
        return;
      } else if (aps.empty() || !ssid.equals(aps.begin()->ssid)) {
        task.enableDelayed(SCAN_INTERVAL);
        return;
      }
    }
  }

  if (aps.empty()) {
    // No unknown nodes found
    if (WiFi.status() == WL_CONNECTED &&
        !(mesh->shouldContainRoot && !layout::isRooted(mesh->asNodeTree()))) {
      // if already connected -> scan slow
      Log(CONNECTION,
          "connectToAP(): Already connected, and no unknown nodes found: "
          "scan rate set to slow\n");
      task.delay(random(2, 4) * SCAN_INTERVAL);
    } else {
      // else scan fast (SCAN_INTERVAL)
      Log(CONNECTION,
          "connectToAP(): No unknown nodes found scan rate set to "
          "normal\n");
      task.setInterval(0.5 * SCAN_INTERVAL);
    }
    mesh->stability += min(1000 - mesh->stability, (size_t)25);
  } else {
    if (WiFi.status() == WL_CONNECTED) {
      Log(CONNECTION,
          "connectToAP(): Unknown nodes found. Current stability: %s\n",
          String(mesh->stability).c_str());

      int prob = mesh->stability;
      if (!mesh->shouldContainRoot)
        // Slower when part of bigger network
        prob /= 2 * (1 + layout::size(mesh->asNodeTree()));
      if ((!layout::isRooted(mesh->asNodeTree()) && random(0, 1000) < prob) || useTargetBSSID) {
        Log(CONNECTION, "connectToAP(): Reconfigure network: %s\n",
            String(prob).c_str());
        // close STA connection, this will trigger station disconnect which
        // will trigger connectToAP()
        mesh->closeConnectionSTA();
        mesh->stability = 0;  // Discourage switching again
        // wifiEventCB should be triggered before this delay runs out
        // and reset the connecting
        task.delay(3 * SCAN_INTERVAL);
      } else {
        if (mesh->shouldContainRoot)
          // Increase scanning rate, because we want to find root
          task.delay(0.5 * SCAN_INTERVAL);
        else
          task.delay(random(2, 4) * SCAN_INTERVAL);
      }
    } else {
      // Else try to connect to first
      auto ap = aps.front();
      aps.pop_front();  // drop bestAP from mesh list, so if doesn't work out,
                        // we can try the next one
      requestIP(ap);
      // Trying to connect, if that fails we will reconnect later
      Log(CONNECTION,
          "connectToAP(): Trying to connect, scan rate set to "
          "4*normal\n");
      task.delay(2 * SCAN_INTERVAL);
    }
  }
}

void StationScan::setTargetBSSID(const uint8_t* bssid) {
  using namespace painlessmesh::logger;
  memcpy(targetBSSID, bssid, sizeof(targetBSSID));
  useTargetBSSID = true;
  Log(DEBUG, "TargetBSSID is set to %i:%i:%i:%i:%i:%i\n", targetBSSID[0], targetBSSID[1], targetBSSID[2], targetBSSID[3], targetBSSID[4], targetBSSID[5]);
}

void StationScan::clearTargetBSSID() {
  useTargetBSSID = false;
}

#endif
