#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>


#define ACCEL_CHAR_UUID "4755494c-4c45-4e6c-d104-768a12148888"
#define SERVICE_UUID "4755494c-4c45-4e6c-d104-768a12141234"

BLEService LedService("180A"); // BLE LED Service
BLEService AccelService(SERVICE_UUID); // BLE Accelerometer Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLEWrite);
BLEStringCharacteristic accelCharacteristic(ACCEL_CHAR_UUID, BLERead | BLENotify, 32);

unsigned long prevMillis = 0;
const long interval = 200;

struct DatosAcel {
  float ax, ay, az;
};
volatile DatosAcel ultimaLectura;

void setup() {
  Serial.begin(115200);
  while (!Serial);
   
  if (!IMU.begin()) {
    Serial.println("¡Error al inicializar la IMU!");
    while (1);
  }
  // set LED's pin to output mode
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(LED_BUILTIN, LOW);         // when the central disconnects, turn off the LED
  digitalWrite(LEDR, HIGH);               // will turn the LED off
  digitalWrite(LEDG, HIGH);               // will turn the LED off
  digitalWrite(LEDB, HIGH);                // will turn the LED off

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Nano 33 BLE Sense");
  BLE.setAdvertisedService(LedService);
  BLE.setAdvertisedService(AccelService);

  // add the characteristic to the service
  LedService.addCharacteristic(switchCharacteristic);
  AccelService.addCharacteristic(accelCharacteristic);

  // add service
  BLE.addService(LedService);
  BLE.addService(AccelService);

  // set the initial value for the characteristic:
  switchCharacteristic.writeValue(0);
  accelCharacteristic.writeValue("Initial data");

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral and Accelerometer Service");
}

void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);            // turn on the LED to indicate the connection

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) { // si hace write en la característica entra en el if
        switch (switchCharacteristic.value()) {   // any value other than 0
          case 01:
            Serial.println("Red LED on");
            digitalWrite(LEDR, LOW);            // will turn the LED on
            digitalWrite(LEDG, HIGH);         // will turn the LED off
            digitalWrite(LEDB, HIGH);         // will turn the LED off
            break;
          case 02:
            Serial.println("Green LED on");
            digitalWrite(LEDR, HIGH);         // will turn the LED off
            digitalWrite(LEDG, LOW);        // will turn the LED on
            digitalWrite(LEDB, HIGH);        // will turn the LED off
            break;
          case 03:
            Serial.println("Blue LED on");
            digitalWrite(LEDR, HIGH);         // will turn the LED off
            digitalWrite(LEDG, HIGH);       // will turn the LED off
            digitalWrite(LEDB, LOW);         // will turn the LED on
            break;
          default:
            Serial.println(F("LEDs off"));
            digitalWrite(LEDR, HIGH);          // will turn the LED off
            digitalWrite(LEDG, HIGH);        // will turn the LED off
            digitalWrite(LEDB, HIGH);         // will turn the LED off
            break;
        }
      }
      
         unsigned long currentMillis = millis();

      // Temporizador para leer el sensor periódicamente
      if (currentMillis - prevMillis >= interval) {
        prevMillis = currentMillis;

        float x, y, z;
        if (IMU.accelerationAvailable()) {
          IMU.readAcceleration(x, y, z);

          // Formateamos los 3 ejes float en una cadena legible
          char buffer[32];
          snprintf(buffer, sizeof(buffer), "X:%.2f Y:%.2f Z:%.2f", x, y, z);

          // ¡CLAVE! Actualiza el valor para LECTURA y envía la NOTIFICACIÓN automáticamente
          accelCharacteristic.writeValue(buffer);

          Serial.print("Notificando a nRF Connect: ");
          Serial.println(buffer);
        }
      }
      
   
    
  }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, LOW);         // when the central disconnects, turn off the LED
    digitalWrite(LEDR, HIGH);          // will turn the LED off
    digitalWrite(LEDG, HIGH);        // will turn the LED off
    digitalWrite(LEDB, HIGH);         // will turn the LED off
  }
}

