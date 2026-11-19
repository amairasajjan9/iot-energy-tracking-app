#ifndef Network_H_
#define Network_H_

#include <WiFi.h>
#include <Firebase_ESP_Client.h>

class Network{
    private:
        FirebaseData fbdo;
        FirebaseAuth auth;
        FirebaseConfig config;

        void firebaseInit();
        friend void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info);
        friend void WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
        friend void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
        friend void FirestoreTokenStatusCallback2(TokenInfo info);
        void initWifi2();
        bool validDate(int date, int month, int year, int hour, int min);
    

    public:
    
    bool isFirst;
        Network();
        void initWifi(int wifi_indicator_pin);
        void firestoreDataUpdate(double units);
        void get_network_info();
};
#endif