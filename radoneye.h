//initial code from https://community.home-assistant.io/u/wettermann/

#include "esphome.h"
#include "BLEDevice.h"

/************************************************************/
//put here your MAC Adress from the radoneye sensor
static String My_BLE_Address = "c1:96:0d:53:e9:a6";
/************************************************************/
//radon1 da:b9:2c:5a:a9:75 lowercase!!!!


// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001523-1212-efde-1523-785feabcd123");
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID("00001525-1212-efde-1523-785feabcd123");
static BLEUUID    char24UUID("00001524-1212-efde-1523-785feabcd123");
static BLEUUID    char23UUID("00001523-1212-efde-1523-785feabcd123");

static union { char c[4]; uint32_t b; float f; } radonval;
static union { char c[2]; uint16_t b; } pulsval;
static float radonnow; 
static float radonday;
static float radonmonth;
static int puls;
static int puls10; 

static BLERemoteService* pRemoteService;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteCharacteristic* p2RemoteCharacteristic;
static BLEClient*  pClient;

class RadonEye : public PollingComponent, public Sensor {
 public:
   Sensor *radon_now = new Sensor();
   Sensor *radon_day = new Sensor();
   Sensor *radon_month = new Sensor();
   Sensor *radon_puls = new Sensor();
   Sensor *radon_puls10 = new Sensor();
  // constructor
  RadonEye() : PollingComponent(60000) {}

  void setup() override {
    // This will be called by App.setup()
	ESP_LOGD("custom","setup start");
	Serial.begin(115200);
	BLEDevice::init("");
    //Convert Mac Adress in correct format
    BLEAddress bleadr = BLEAddress(My_BLE_Address.c_str());//"c1:96:0d:53:e9:a6");
    ESP_LOGD("custom"," - setup connect to mac %s", My_BLE_Address.c_str());
    pClient  = BLEDevice::createClient();
    ESP_LOGD("custom"," - setup- Created client");
    // Connect to the Radoneye  BLE Server.
    pClient->connect(bleadr, BLE_ADDR_TYPE_RANDOM);
    if(pClient->isConnected()) ESP_LOGD("custom"," - setup- Connected to Radoneye");
    else  ESP_LOGD("custom"," - setup- Connect Failed!!!");
}
  
  void update() override {
    // This will be called every "update_interval" milliseconds.
    ESP_LOGD("custom","update - time %i", (int)millis()/1000);
    if(!pClient->isConnected()) setup();
    
    if(pClient->isConnected())
    {
        // Obtain a reference to the service we are after in the remote BLE server.
        pRemoteService = pClient->getService(serviceUUID);
        if (pRemoteService == nullptr) {
            ESP_LOGD("custom","Failed to find our service UUID: %s", serviceUUID.toString().c_str());
        }

        // Obtain a reference to the characteristic in the service of the remote BLE server.
        pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
        if (pRemoteCharacteristic == nullptr) {
            ESP_LOGD("custom","Failed to find our characteristic UUID: %s", charUUID.toString().c_str());
        }
        
        // Obtain a reference to the characteristic to write to in the service of the remote BLE server.
        p2RemoteCharacteristic = pRemoteService->getCharacteristic(char24UUID);
        if (p2RemoteCharacteristic == nullptr) {
            ESP_LOGD("custom","Failed to find our characteristic 1524 UUID: %s", charUUID.toString().c_str());
        }
        //write 0x50 to request new data from sensor
        if(p2RemoteCharacteristic->canWrite()) {
            p2RemoteCharacteristic->writeValue(0x50);
            ESP_LOGD("custom","write Value to our characteristic 1524 with 0x50");
        }
        if(pRemoteCharacteristic->canRead()) {
            std::string value = pRemoteCharacteristic->readValue();
            ESP_LOGD("custom","result bytes: %02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X %02X%02X %02X%02X", value[0],value[1],value[2],value[3], value[4],value[5],value[6],value[7], value[8],value[9],value[10],value[11], value[12],value[13],value[14],value[15], value[16],value[17],value[18],value[19]);
            //on 0x50: 50100AD7 63406666 46400000 00000100 15000000
            //on 0x51: 510E0200 1D400000 0C4A0800 71595341 15000000 (Peakvalue12-15, Time since start 4-8)?
            //11tage 9h 37min 44s =985064 Peak 488 
            //30s entspricht Byte[4] +1 
            radonval.c[0] = value[2];
            radonval.c[1] = value[3];
            radonval.c[2] = value[4];
            radonval.c[3] = value[5];
            radonnow = radonval.f;
            radonval.c[0] = value[6];
            radonval.c[1] = value[7];
            radonval.c[2] = value[8];
            radonval.c[3] = value[9];
            radonday = radonval.f;
            radonval.c[0] = value[10];
            radonval.c[1] = value[11];
            radonval.c[2] = value[12];
            radonval.c[3] = value[13];
            radonmonth = radonval.f;
            pulsval.c[0] = value[14];
            pulsval.c[1] = value[15];
            puls = pulsval.b;
            pulsval.c[0] = value[16];
            pulsval.c[1] = value[17];
            puls10 = pulsval.b;
            ESP_LOGD("custom","Radonnow %.0f Day %.0f Month %.0f Pulse %i Puls10min %i ", radonnow, radonday, radonmonth, puls, puls10);
        }
        radon_now->publish_state(radonnow);
        radon_day->publish_state(radonday);
        radon_month->publish_state(radonmonth);
        radon_puls->publish_state(puls);
        radon_puls10->publish_state(puls10);
        ESP_LOGD("custom","Published new values to frontend");
    } else {
        ESP_LOGD("custom","Not connected to sensor");
    }
  }
};
