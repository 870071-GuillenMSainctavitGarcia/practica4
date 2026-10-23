# Práctica 4: Comunicaciones Inalámbricas Bluetooth (BLE y Bluetooth Classic SPP) en Arduino Nano 33 BLE y ESP32-S3

## Descripción General

Este proyecto explora la conectividad inalámbrica de corto alcance mediante Bluetooth en sus dos vertientes principales: Bluetooth Low Energy (BLE 4.0/5.0) y Bluetooth Classic con perfil SPP (Serial Port Profile). Se aborda desde la emisión de anuncios (Advertising) con UUIDs personalizados y servidores GATT, la manipulación de características leíbles, escribibles y notificables para el control de periféricos (LEDs RGB) y sensores (IMU/Acelerómetro), hasta el control de flujo de notificaciones y la creación de un terminal de chat serie inalámbrico bidireccional entre el microcontrolador y dispositivos móviles.


## ¿Como probar los subapartados del proyecto?

Como cada apartado es un código distinto:

1. Si vas a probar los **Apartados 1, 2, 3 o 4**, abre el archivo de la carpeta correspondiente o ajusta el filtro build_src_filter en el archivo platformio.ini para cargar el firmware en la Arduino Nano 33 BLE (o ESP32-S3 según corresponda).
2. Si vas a probar el canal de chat por Bluetooth Classic (**Apartado 5**):
   * Carga src/apartado5_2/apartado5_2.cpp en el ESP32 (o ESP32-S3 compatible con SPP/BT).
   * Vincula previamente la placa desde los ajustes Bluetooth de tu móvil Android.
   * Abre la app Serial Bluetooth Terminal y conéctate al dispositivo ESP32_Chat_Guillen.
3. Para los apartados BLE (1 a 4), utiliza la aplicación móvil nRF Connect for Mobile o LightBlue para escanear, conectar y manipular los servicios y características GATT.
4. Abre el Monitor Serie a 115200 baudios.


## Estructura del Proyecto

```text
Practica4/
├── src/
│   ├── apartado1_3/           # Anuncio de publicidad BLE con UUID personalizado
│   │   └── apartado1_3.cpp
│   ├── apartado2_2/           # Servidor GATT BLE para control de LEDs RGB (Lectura/Escritura)
│   │   └── apartado2_2.cpp
│   ├── apartado3_2/           # Notificación periódica de acelerómetro + Control de LEDs por BLE
│   │   └── apartado3_2.cpp
│   ├── apartado4_2/           # Control de flujo del acelerómetro mediante comandos START/STOP por BLE
│   │   └── apartado4_2.cpp
│   └── apartado5_2/           # Terminal de Chat bidireccional por Bluetooth Classic (SPP)
│       └── apartado5_2.cpp
├── platformio.ini             # Entornos de compilación y monitorización de PlatformIO
└── README.md                  # Documentación principal
```

## APARTADO 1

Procedimiento: Configuración del periférico BLE en la Arduino Nano 33 BLE para emitir paquetes de publicidad (Advertising) asociando un nombre local y un UUID personalizado de 128 bits derivado de texto en formato hexadecimal.

- **Lógica y Funciones Clave:**

    * BLE.begin(): Inicializa el controlador e interfaz de hardware de Bluetooth Low Energy.
    * BLEService miServicio(SERVICE_UUID): Instancia un servicio GATT utilizando un UUID personalizado de 128 bits.
    * BLE.setLocalName("GUILLEN_Arduino"): Asigna el identificador visible transmitido en el paquete de anuncios para el escaneo de dispositivos centrales.
    * BLE.setAdvertisedService(miServicio) / BLE.addService(miServicio): Registra el servicio tanto en el paquete de emisión publica como en la tabla GATT interna.
    * BLE.advertise(): Activa la emisión continua de tramas de publicidad en segundo plano.
    * BLE.central() / central.connected(): Detecta la conexión de un dispositivo central (teléfono móvil) y monitoriza la sesión mediante la dirección MAC del cliente.

## APARTADO 2

Procedimiento: Despliegue de un servidor GATT con un servicio de control de actuadores que expone una característica leíble y escribible (BLERead | BLEWrite) para conmutar los colores del LED RGB incorporado en la placa según el byte enviado.

- **Lógica y Funciones Clave:**

    * BLEService ledService("180A"): Crea el servicio GATT asignado al estándar de información de dispositivo.
    * BLEByteCharacteristic switchCharacteristic("2A57", BLERead | BLEWrite): Define una característica de un byte habilitada para lectura y escritura desde la app central.
    * switchCharacteristic.written() / switchCharacteristic.value(): Evalúa si se ha recibido una orden de escritura externa y extrae el valor del byte.
    * Sentencia switch(valor): Interpreta el comando recibido para alternar los pines digitales de salida (LEDR, LEDG, LEDB en lógica invertida):
        * Case 01: Enciende el LED Rojo (LEDR = LOW).
        * Case 02: Enciende el LED Verde (LEDG = LOW).
        * Case 03: Enciende el LED Azul (LEDB = LOW).
        * Default: Apaga todos los LEDs.

## APARTADO 3

Procedimiento: Integración simultánea de múltiples servicios GATT (LED y Acelerómetro), implementando la lectura de la IMU LSM9DS1 y la transmisión en tiempo real de las lecturas mediante notificaciones (BLENotify).

- **Lógica y Funciones Clave:**

    * BLEStringCharacteristic accelCharacteristic(ACCEL_CHAR_UUID, BLERead | BLENotify, 32): Instancia la característica de datos con capacidad de notificación remota para cadenas de hasta 32 bytes.
    * IMU.readAcceleration(x, y, z): Obtiene las componentes físicas de aceleración desde el sensor inercial.
    * snprintf(buffer, ...): Da formato a la cadena de texto con la información de los tres ejes (X:%.2f Y:%.2f Z:%.2f).
    * accelCharacteristic.writeValue(buffer): Actualiza el registro de la característica y dispara automáticamente la notificación push al cliente suscrito en nRF Connect.
    * Temporizador con millis(): Garantiza el muestreo y envío de notificaciones a un intervalo constante de 200 ms sin bloquear la ejecución del bucle principal.

## APARTADO 4

Procedimiento: Implementación de una arquitectura de control de flujo basada en dos características BLE separadas: una característica exclusiva de escritura para la recepción de comandos de control y una característica de notificación para la emisión condicional de datos inerciales.

- **Lógica y Funciones Clave:**

    * BLEStringCharacteristic controlCharacteristic(CONTROL_CHAR_UUID, BLEWrite, 10): Característica dedicada exclusivamente a recibir órdenes desde el cliente central.
    * BLEStringCharacteristic dataCharacteristic(DATA_CHAR_UUID, BLERead | BLENotify, 32): Característica dedicada a enviar las muestras del sensor.
    * Máquina de estados (estadoSensor):
        * Control mediante controlCharacteristic.written(): Lee la orden entrante; si detecta START / start cambia a estadoSensor = 1, y si recibe STOP / stop cambia a estadoSensor = 0.
    * Sentencia switch (estadoSensor):
        * Case 0 (STOP): Suspende la lectura de la IMU y la emisión de notificaciones, manteniendo el enlace BLE en espera de nuevas órdenes.
        * Case 1 (START): Ejecuta