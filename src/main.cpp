#include <ArduinoBLE.h>

// 1. Definimos el UUID personalizado con las letras de tu nombre en HEX
// Ejemplo para "GUILLEN" -> 47 55 49 4C 45 4E
#define SERVICE_UUID "4755494c-4c45-4e6c-d104-768a12141234"

// 2. Creación del servicio BLE usando ese UUID
BLEService miServicio(SERVICE_UUID);

void setup() {
  Serial.begin(9600);
  
  if (!BLE.begin()) {
    Serial.println("¡Error al iniciar BLE!");
    while (1);
  }

  
  BLE.setLocalName("GUILLEN_Arduino");

  // Añadimos el UUID del servicio al paquete de Advertising
  BLE.setAdvertisedService(miServicio);

  // Añadimos el servicio al servidor GATT interno
  BLE.addService(miServicio);

  // Iniciamos la emisión de anuncios (Advertising)
  BLE.advertise();

  Serial.println("Emitiendo publicidad BLE con UUID personalizado...");
}

void loop() {
  // El módulo estará emitiendo automáticamente en segundo plano
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("¡Móvil conectado! Dirección MAC: ");
    Serial.println(central.address());

    // Mientras el móvil siga conectado
    while (central.connected()) {
      // Aquí podemos actualizar el valor o hacer cosas periódicas
      delay(1000);
    }

    Serial.println("Móvil desconectado. Volviendo a emitir publicidad...");
   
   Serial.println("Emitiendo publicidad BLE con UUID personalizado...");
  }
  //Serial.println("Esperando a que un móvil se conecte...");
}