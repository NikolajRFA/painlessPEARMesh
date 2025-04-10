#ifndef _PAINLESS_MESH_PEAR_HPP_
#define _PAINLESS_MESH_PEAR_HPP_

#include <Arduino.h>

namespace painlessmesh {

class Pear {
  public:
  uint8_t noOfVerifiedDevices = 0;
  uint8_t energyProfile = 200; // Why 200 you might ask...

  void run(){
    // Create a list of all devices - devices
    // for each device in devices:
    if(noOfVerifiedDevices < 10){
      // if(deviceExceedsLimit(device)) updateParent(device);
    }
  }

  bool deviceExceedsLimit(uint32_t deviceId){
    // get the device using the id
    // if(device.transmissions > device.threshold) return true;
    noOfVerifiedDevices++;
    return false;
  }

  void updateParent(uint32_t deviceId){
    // get the device using the id
    // create a descending list of subs sorted by transmission - nodesToReroute
    // for each nodeToReroute:
    // for each visibleNode in nodeToReroute's visible nodes (getAvailableNetworks):
    // if visibleNode is within limits set nodeToReroute.newParent = visibleNode
  }
};

}

#endif //_PAINLESS_MESH_PEAR_HPP_
