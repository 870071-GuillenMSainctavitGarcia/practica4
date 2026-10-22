#include "BluetoothSerial.h"

// Creamos el objeto para la comunicación Bluetooth Classic (SPP)
BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);

  // Iniciamos el Bluetooth Classic con el nombre que aparecerá al buscar dispositivos
  SerialBT.begin("ESP32_Chat_Guillen"); 
  Serial.println("El dispositivo Bluetooth Classic está listo. Emparejar desde el móvil.");
}

void loop() {
  // 1. Si el MÓVIL envía texto al ESP32 por la app
  if (SerialBT.available()) {
    String mensajeRecibido = SerialBT.readString();
    
    Serial.print("Móvil dice: ");
    Serial.print(mensajeRecibido); // Se muestra en el Monitor Serie del ordenador
  }

  // 2. Si TÚ escribes texto en el Monitor Serie del ordenador para enviarlo al MÓVIL
  if (Serial.available()) {
    String mensajeAEnviar = Serial.readString();
    
    SerialBT.print(mensajeAEnviar); // Se envía por Bluetooth al móvil
  }
}