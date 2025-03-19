//************************************************************
// this is a simple example that uses the painlessMesh library to
// connect to a node on another network. Please see the WIKI on gitlab
// for more details
// https://gitlab.com/painlessMesh/painlessMesh/wikis/bridge-between-mesh-and-another-network
//************************************************************
#include "painlessMesh.h"
#include "deviceIds.h"
#include "../lib/painlessMesh/src/painlessmesh/tcp.hpp"

#define MESH_PREFIX1 "painless1"
#define MESH_PREFIX2 "painless2"
#define MESH_PREFIX3 "painless3"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

#define STATION_SSID "painless1"
#define STATION_PASSWORD "somethingSneaky"
#define STATION_PORT 5555
uint8_t station_ip[4] = {0, 0, 0, 0}; // IP of the server

// prototypes
void receivedCallback(uint32_t from, String &msg);
void changedConnectionCallback();

uint32_t chipId;

Scheduler userScheduler;

painlessMesh mesh;
void sendMessage();
Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

void setup()
{
  WiFi.begin();
  uint8_t MAC[] = {0, 0, 0, 0, 0, 0};
  WiFi.softAPmacAddress(MAC);
  WiFi.disconnect();

  chipId = 0;
  chipId |= MAC[2] << 24;  // Big endian (aka "network order"):
  chipId |= MAC[3] << 16;
  chipId |= MAC[4] << 8;
  chipId |= MAC[5];

  Serial.begin(115200);
  Serial.print("Device id: ");
  Serial.println(chipId);
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for you other
  // network (STATION_SSID)

  if (chipId == CHIP1)
  {
    mesh.init(MESH_PREFIX1, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  }
  else if (chipId == CHIP2)
  {
    mesh.init(MESH_PREFIX2, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  }
  else if (chipId == CHIP3)
  {
    mesh.init(MESH_PREFIX3, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  }

  if (chipId == CHIP2)
  {
    mesh.stationManual(MESH_PREFIX1, STATION_PASSWORD, STATION_PORT, station_ip);
  }
  else if (chipId == CHIP3)
  {
    mesh.stationManual(MESH_PREFIX1, STATION_PASSWORD, STATION_PORT, station_ip);
  }

  if (chipId == CHIP1)
  {
    mesh.setRoot(true);
  }
  //  This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  mesh.onReceive(&receivedCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  if (chipId == CHIP2)
  {
    Serial.println("Adding send message task");
    userScheduler.addTask(taskSendMessage);
    taskSendMessage.enable();
  }

}

void loop()
{
  mesh.update();
  userScheduler.execute();
}

void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
}

void changedConnectionCallback()
{
  Serial.println(mesh.subConnectionJson());
}

void sendMessage()
{
  Serial.println("Entered send message");
  String msg = "Hello from 2";

  Serial.println("Sending message");
  bool messageSent = mesh.sendSingle(CHIP3, msg);
  if (messageSent) {
    Serial.println("Message send successfully");
  }
  else
  {
    Serial.println("Something went wrong when sending message");
  }

  taskSendMessage.setInterval( random(TASK_SECOND * 1, TASK_SECOND * 5));  
}