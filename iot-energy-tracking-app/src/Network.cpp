#include "Network.h"
#include "addons/TokenHelper.h"
#include "time.h"
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <WiFi.h>

#define WIFI_SSID1 "realme 8i"
#define WIFI_PASSWORD1 "Motiv@123"

#define WIFI_SSID2 "Galaxy A03s5168"
#define WIFI_PASSWORD2 "8968456606"

// #define WIFI_SSID2 "Galaxy A03s5168"
// #define WIFI_PASSWORD2 "8968456606"

#define API_KEY "AIzaSyDb8bxKaVa8oJkw5MjUsC-r1xdkcWbchx0"
#define FIREBASE_PROJECT_ID "iot-energy-tracking-app"
#define USER_EMAIL "esp32@green.com"
#define USER_PASSWORD "green@123"

static Network *instance = NULL;



RTC_DATA_ATTR int lastdate = -1;
RTC_DATA_ATTR int lastmonth = -1;
RTC_DATA_ATTR int lastyear = -1;
RTC_DATA_ATTR int lasthour = -1;
RTC_DATA_ATTR int lastmin = -1;

bool isFirst = true;
bool gotDate = false;

int wifi_ind_pin = 27;
int tries = 0;
int tried = 1;

Network::Network()
{
    instance = this;
}

void FirestoreTokenStatusCallback2(TokenInfo info)
{
    Serial.printf("token info: %s, status: %s\n", getTokenType(info), getTokenStatus(info));
    Serial.print("\n");
}
void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("Wifi Connected But wait for the local IP Address");
    Serial.print("\n");
    digitalWrite(wifi_ind_pin, HIGH);
    delay(200);
    digitalWrite(wifi_ind_pin, LOW);
}

void WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("Local IP Address: ");
    Serial.print(WiFi.localIP());
    Serial.print("\n");
    Serial.printf("Wifi Initialized");

    digitalWrite(wifi_ind_pin, HIGH);
    delay(100);
    digitalWrite(wifi_ind_pin, LOW);
    delay(100);
    digitalWrite(wifi_ind_pin, HIGH);
    delay(100);
    digitalWrite(wifi_ind_pin, LOW);

    Serial.printf("Firebase initialization started");

    instance->firebaseInit();

    Serial.printf("Firebase initialization completed");
}

void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("WiFi Disconnected");
    Serial.print("\n");
    digitalWrite(wifi_ind_pin, HIGH);
    delay(200);
    digitalWrite(wifi_ind_pin, LOW);
    delay(200);
    tries++;

    if (tries >= 20)
    {
        if (tried == 1)
        {
            tried = 2;
            tries = 0;
            WiFi.begin(WIFI_SSID2, WIFI_PASSWORD2);
        }
        else
        {
            tried = 1;
            tries = 0;
            WiFi.begin(WIFI_SSID1, WIFI_PASSWORD1);
        }
    }
}

void Network::initWifi(int temp_wifi_indicator_pin)
{
    wifi_ind_pin = temp_wifi_indicator_pin;
    WiFi.disconnect();
    WiFi.onEvent(WiFiEventConnected, arduino_event_id_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiEventGotIP, arduino_event_id_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiEventDisconnected, arduino_event_id_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    tried = 2;
    tries = 0;
    WiFi.begin(WIFI_SSID1, WIFI_PASSWORD1);
}

void Network::get_network_info()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("[*] Network information for ");
        Serial.println(WiFi.SSID());

        Serial.print("[+] BSSID : ");
        Serial.println(WiFi.BSSIDstr());
        Serial.print("[+] Gateway IP : ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("[+] Subnet Mask : ");
        Serial.println(WiFi.subnetMask());
        Serial.print((String) "[+] RSSI : ");
        Serial.println(WiFi.RSSI() + " dB");
        Serial.print("[+] ESP32 IP : ");
        Serial.println(WiFi.localIP());
    }
}

void Network::initWifi2()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(WIFI_SSID2, WIFI_PASSWORD2);
    }
}

void Network::firebaseInit()
{
    config.api_key = API_KEY;

    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    config.token_status_callback = FirestoreTokenStatusCallback2;

    // Firebase.begin(&config, &auth);
}
tm getTime()
{
    time_t now;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        gotDate = false;
        return (timeinfo);
    }
    else
    {
        gotDate = true;
        return (timeinfo);
    }

    //   time(&now);
    //   return now;
}

bool Network::validDate(int date, int month, int year, int hour, int min)
{
    if (isFirst)
    {
        Serial.println("is first");
        isFirst = false;
        return true;
    }

    if(
        (lastdate == -1) || 
        (lastmonth == -1) ||
        (lastyear == -1)|| 
        (lasthour == -1)||
        (lastmin == -1)
    )
    {
        Serial.println("something is -1");
                    Serial.println(lasthour);
            Serial.println(lastmin);
            Serial.println(lastdate);
            Serial.println(lastmonth);
            Serial.println(lastyear);
        return true;
    }

    if ((lastyear < year) ||
        ((lastyear == year) && (lastmonth < month)) ||
        ((lastyear == year) && (lastmonth == month) && (lastdate < date)))
    {
        Serial.println("date is changed");
        Serial.println("--------------");
        Serial.println(lastyear);
        Serial.println("--------------");
        return true;
    }
    else if (
        ((lastyear == year) && (lastmonth == month) && (lastdate == date)) &&
        ((lasthour < hour)))
    {

        Serial.println("hour is changed");
        Serial.println("--------------");
        Serial.println(lasthour);
        Serial.println("-@@@@@@-");

        Serial.println(hour);
        Serial.println("--------------");
        return true;
    }
    
    
        Serial.println("Not the right time");
        return false;
    
}

void Network::firestoreDataUpdate(double units)
{
    digitalWrite(wifi_ind_pin, HIGH);
    String documentPath = "Data/first123";
    String mask = "";
    tm timeInfo = getTime();
    digitalWrite(wifi_ind_pin, LOW);
    int date = timeInfo.tm_mday;
    int month = timeInfo.tm_mon+1;
    int year = timeInfo.tm_year+1900;
    int additional = 0;
    int min = 0;
    int hour = 0;

    if ((timeInfo.tm_min + 30) >= 60)
    {
        additional = 1;
        min = (timeInfo.tm_min + 30) % 60;
    }
    else
    {
        min = (timeInfo.tm_min + 30);
    }                                           // to ist
    hour = (timeInfo.tm_hour + 5) + additional; // to ist
                                                //     char timeHour[3];
                                                // strftime(timeHour,3, "%H", &timeInfo);

    // char *str = strchr("", date);
    // str = strcat(str, ":");
    // str = strchr(str, month);
    // str = strcat(str, ":");
    // str = strchr(str, year);

    // const char* str2 = strcat(str, ":");
    Serial.println("----------------");
    Serial.print("date ");
    Serial.print(date);
    Serial.print(" month ");
    Serial.print(month + 1);
    Serial.print(" year ");
    Serial.println(year);
    Serial.print(" hour ");
    Serial.println(hour);
    Serial.print(" min ");
    Serial.println(min);

    Serial.println("----------------");
    bool temp = validDate(date, month, year, hour, min);

    if ((WiFi.status() == WL_CONNECTED) && Firebase.ready() && temp && gotDate)
    {
        digitalWrite(wifi_ind_pin, HIGH);

        FirebaseJson obj;
        obj.set("values/[0]/mapValue/fields/date/integerValue", date);
        obj.set("values/[0]/mapValue/fields/month/integerValue", (month));
        obj.set("values/[0]/mapValue/fields/year/integerValue", (year));
        obj.set("values/[0]/mapValue/fields/hour/integerValue", hour);
        obj.set("values/[0]/mapValue/fields/minutes/integerValue", min);
        obj.set("values/[0]/mapValue/fields/units/doubleValue", units);

        FirebaseJsonArray dataArr;
        dataArr.add(obj);

        FirebaseJson content;

        Serial.print("Commit a document (append array)... ");

        // The dyamic array of write object firebase_firestore_document_write_t.
        std::vector<struct firebase_firestore_document_write_t> writes;

        // A write object that will be written to the document.
        struct firebase_firestore_document_write_t transform_write;

        // Set the write object write operation type.
        // firebase_firestore_document_write_type_update,
        // firebase_firestore_document_write_type_delete,
        // firebase_firestore_document_write_type_transform
        transform_write.type = firebase_firestore_document_write_type_transform;

        // Set the document path of document to write (transform)
        transform_write.document_transform.transform_document_path = "Data/first123";

        // Set a transformation of a field of the document.
        struct firebase_firestore_document_write_field_transforms_t field_transforms;

        // Set field path to write.
        field_transforms.fieldPath = "historicalData";

        // Set the transformation type.
        // firebase_firestore_transform_type_set_to_server_value,
        // firebase_firestore_transform_type_increment,
        // firebase_firestore_transform_type_maaximum,
        // firebase_firestore_transform_type_minimum,
        // firebase_firestore_transform_type_append_missing_elements,
        // firebase_firestore_transform_type_remove_all_from_array
        field_transforms.transform_type = firebase_firestore_transform_type_append_missing_elements;

        // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
        // FirebaseJson content;

        // content.set("values/[0]/mapValue/fields/id/stringValue", "item" + String(count));
        // content.set("values/[0]/mapValue/fields/status/booleanValue", count % 2 == 0);

        // Set the transformation content.
        field_transforms.transform_content = obj.raw();

        // Add a field transformation object to a write object.
        transform_write.document_transform.field_transforms.push_back(field_transforms);

        // Add a write object to a write array.
        writes.push_back(transform_write);

        if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of firebase_firestore_document_write_t */, "" /* transaction */))
        {

            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());

            lastdate = date;
            lastmonth = month;
            lastyear = year;
            lasthour = hour;
            lastmin = min;
            Serial.println(lasthour);
            Serial.println(lastmin);
            Serial.println(lastdate);
            Serial.println(lastmonth);
            Serial.println(lastyear);
            Serial.println("lasts updated");
        }
        else
        {

            Serial.println(fbdo.errorReason());
        }
        digitalWrite(wifi_ind_pin, LOW);
    }
    else
    {
        // instance->firebaseInit();
        Serial.println("in firestoreDataUpdate() ");
        digitalWrite(wifi_ind_pin, HIGH);
        Serial.println(WiFi.status());
        Serial.println(WL_CONNECTED);
        Serial.println(WL_DISCONNECTED);
        Serial.printf("gotDate %d", gotDate);
        Serial.printf("temp %d", temp);
        Serial.println("in firestoreDataUpdate() ");

        if (WiFi.status() != WL_CONNECTED)
        {
        Serial.println("in firestoreDataUpdate() if condition");
            instance->initWifi2();
        }
        Firebase.begin(&config, &auth);
        digitalWrite(wifi_ind_pin, LOW);
    }
}
