//************************************************************
// this is a simple example that uses the painlessMesh library to
// connect to a node on another network. Please see the WIKI on gitlab
// for more details
// https://gitlab.com/painlessMesh/painlessMesh/wikis/bridge-between-mesh-and-another-network
//************************************************************
#include "../lib/painlessMesh/src/painlessMesh.h"
#include "deviceIds.h"
#include "../lib/painlessMesh/src/painlessmesh/tcp.hpp"
#include "meshConstants.h"
#include "meshHelpers.hpp"

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain
void logConnections();

void reconfigure();

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);
Task taskLogConnections(TASK_SECOND * 10, TASK_FOREVER, &logConnections);
Task taskReconfigure(TASK_HOUR, TASK_ONCE, &reconfigure);

void sendMessage() {
    uint32_t nodeId = 3206793885;
    String jsonString = buildNewParentJson(nodeId);
    //if (mesh.getNodeId() == CHIP1) mesh.sendPear(3211408993, jsonString);
    taskSendMessage.setInterval(TASK_SECOND * 20);
}

void logConnections() {
    Serial.print("TOPOLOGY: ");
    Serial.println(mesh.subConnectionJson(true));
    Serial.printf("Number of nodes in mesh: %i\n", countUniqueNodeIds(mesh.subConnectionJson()));
}

void reconfigure() {
    Serial.println("reconfigure():");
    mesh.setTargetNodeId(3206773453);
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg) {
    Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("!!!CONNECTIONS CHANGED!\n");
    logConnections();
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
    Serial.begin(115200);

    //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
    mesh.setDebugMsgTypes(PEAR | CONNECTION | ERROR | STARTUP | DEBUG | GENERAL | MESH_STATUS);
    // set before init() so that you can see startup messages

    mesh.init(MESH_PREFIX1, MESH_PASSWORD, &userScheduler, MESH_PORT);
    uint32_t nodeId = mesh.getNodeId();
    Serial.print("Node id: ");
    Serial.println(nodeId);
    if (nodeId == CHIP1) {
        Pear::getInstance().setRootNodeId(CHIP1); // hacky initialisation of pear
        Pear::getInstance().pearNodeTreeMap.insert({
            CHIP1,
            std::make_shared<PearNodeTree>(std::make_shared<protocol::NodeTree>(mesh.asNodeTree()), 9999, 9999)});
        mesh.setDontConnect(true);
        mesh.setRoot();
    }
    mesh.initPearTasks();
    mesh.setContainsRoot();

    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

    userScheduler.addTask(taskSendMessage);
    userScheduler.addTask(taskLogConnections);
    //userScheduler.addTask(taskReconfigure);
    taskLogConnections.enable();
    taskSendMessage.enable();
    //taskReconfigure.enableDelayed(3 * TASK_MINUTE);
}

void loop() {
    // it will run the user scheduler as well
    mesh.update();
}
