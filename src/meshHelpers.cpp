#include "painlessMesh.h"
#include "meshConstants.h"

void changeAP(painlessMesh &mesh, String ssid)
{
    mesh.closeConnectionSTA();
    mesh.stationManual(ssid, MESH_PASSWORD, MESH_PORT);
}

int getRootNodeId(TSTRING connections)
{
    const size_t ROOT_NODE_ID_LENGTH = 10; // The length of the root node ID
    const size_t ROOT_NODE_ID_OFFSET = 13; // Offset from "root" to the start of the ID
    std::string connectionsStr = connections.c_str();
    // Find the position of "root"
    size_t rootStringStartPosition = connectionsStr.find("root");
    if (rootStringStartPosition == std::string::npos)
    {
        return -1; 
    }

    // Extract the root node ID (assuming a fixed format)
    size_t idStartPos = rootStringStartPosition - ROOT_NODE_ID_OFFSET;
    if (idStartPos > connectionsStr.size())
    {
        return -2;
    }

    std::string rootNodeIdStr = connectionsStr.substr(idStartPos, ROOT_NODE_ID_LENGTH);
    uint32_t rootNodeId = static_cast<uint32_t>(std::stoul(rootNodeIdStr));

    return rootNodeId;
}
