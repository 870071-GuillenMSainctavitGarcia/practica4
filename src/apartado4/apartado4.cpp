#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

// UUIDs para el servicio y sus dos características
#define SERVICE_UUID    "4755494c-4c45-4e6c-d104-768a12141234"
#define CONTROL_CHAR_UUID "4755494c-4c45-4e6c-d104-768a12145555" // Solo Escritura
#define DATA_CHAR_UUID    "4755494c-4c45-4e6c-d104-768a12148888" // Lectura y Notificación

BLEService AccelService(SERVICE_UUID);

// CARACTERÍSTICA 1: Solo Escritura (para recibir START / STOP)
BLEStringCharacteristic controlCharacteristic(CONTROL_CHAR_UUID, BLEWrite, 10);

// CARACTERÍSTICA 2: Lectura y Notificación (para enviar los datos del acelerómetro)
BLEStringCharacteristic dataCharacteristic(DATA_CHAR_UUID, BLERead | BLENotify, 32);

unsigned long prevMillis = 0;
const long interval = 200;

void setup() {
  Serial.begin(115200);

  if (!IMU.begin()) {
    Serial.println("¡Error al inicializar la IMU!");
    while (1);
  }

  if (!BLE.begin()) {
    Serial.println("¡Error al iniciar BLE!");
    while (1);
  }

  // Nombre corto para no sobrepasar los 31 bytes del paquete de anuncio
  BLE.setLocalName("Nano33"); 
  BLE.setAdvertisedService(AccelService);

  // Añadimos las DOS características al servicio
  AccelService.addCharacteristic(controlCharacteristic);
  AccelService.addCharacteristic(dataCharacteristic);
  
  BLE.addService(AccelService);

  // Valores iniciales
  controlCharacteristic.writeValue("STOP");
  dataCharacteristic.writeValue("X:0.00 Y:0.00 Z:0.00");

  BLE.advertise();
  Serial.println("Servidor BLE activo y anunciado correctamente...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Conectado a central: ");
    Serial.println(central.address());

    int estadoSensor = 0; // 0 = Inactivo (STOP), 1 = Activo (START)

    while (central.connected()) {
      
      // 1. COMPROBAMOS SI SE HA ESCRITO EN LA CARACTERÍSTICA DE CONTROL
      if (controlCharacteristic.written()) {
        String comando = controlCharacteristic.value();
        Serial.print("Comando de control recibido: ");
        Serial.println(comando);

        if (comando.equals("START") || comando.equals("start")) {
          Serial.println("-> ACTIVANDO lectura y notificaciones del acelerómetro.");
          estadoSensor = 1; 
        } 
        else if (comando.equals("STOP") || comando.equals("stop")) {
          Serial.println("-> DESACTIVANDO lectura y notificaciones del acelerómetro.");
          estadoSensor = 0; 
        } 
      }

      // 2. SEGÚN EL ESTADO, NOTIFICAMOS LOS DATOS
      switch (estadoSensor) {   
        case 0:
          // Desactivado: No hace nada, espera órdenes
          break;

        case 1: {
          // Activo: Lee el sensor cada 200 ms y actualiza la característica de datos
          unsigned long currentMillis = millis();

          if (currentMillis - prevMillis >= interval) {
            prevMillis = currentMillis;

            float x, y, z;
            if (IMU.accelerationAvailable()) {
              IMU.readAcceleration(x, y, z);

              char buffer[32];
              snprintf(buffer, sizeof(buffer), "X:%.2f Y:%.2f Z:%.2f", x, y, z);

              // Enviamos los datos por la característica de DATOS
              dataCharacteristic.writeValue(buffer);

              Serial.print("Notificando acelerómetro: ");
              Serial.println(buffer);
            }
          }
          break;
        }

        default:
          break;
      } 

    } // Fin del while (central.connected())

    Serial.print("Desconectado de: ");
    Serial.println(central.address());
  }
}
