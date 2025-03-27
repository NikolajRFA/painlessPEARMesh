#include "painlessMesh.h"
#include "meshConstants.h"

void changeAP(painlessMesh &mesh, String ssid)
{
    mesh.closeConnectionSTA();
    mesh.stationManual(ssid, MESH_PASSWORD, MESH_PORT);
}