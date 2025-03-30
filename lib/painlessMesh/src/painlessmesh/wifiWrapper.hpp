#include <vector>
#include <WiFi.h>

class WiFiWrapper
{
public:
    // Virtual destructor for proper cleanup in derived classes
    virtual ~WiFiWrapper() = default;

    // Virtual methods to allow mocking in tests
    virtual int scanNetworks()
    {
        return WiFi.scanNetworks();
    }

    virtual String SSID(int i)
    {
        return WiFi.SSID(i);
    }
};

class MockWiFiWrapper : public WiFiWrapper
{
public:
    int scanNetworks() override
    {
        return mockSSIDs.size();
    }

    String SSID(int i) override
    {
        return (i < mockSSIDs.size()) ? mockSSIDs[i] : "";
    }

private:
    std::vector<String> mockSSIDs = {"painless123", "otherNetwork", "painless456"};
};