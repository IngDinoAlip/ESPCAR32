# ESPCAR32

Sistema de control y monitoreo ambiental para carro robotico autonomo basado en ESP32.

## Descripcion

ESPCAR32 es un proyecto que integra un vehiculo robotico de 4 ruedas controlado por un ESP32, con capacidades de monitoreo ambiental y navegacion autonoma. El sistema incluye:

- Control manual y automatico del vehiculo
- Monitoreo de calidad del aire (CO2, temperatura, humedad) mediante sensor SCD30
- Dashboard web en tiempo real con graficas
- Sistema de rutas programables
- Integracion con Firebase Realtime Database
- Navegacion autonoma con sensores ultrasonicos e infrarrojos

## Caracteristicas

### Hardware
- Microcontrolador: ESP32
- 4 motores DC con puente H
- Sensor de CO2, temperatura y humedad: SCD30
- Sensor ultrasonico HC-SR04 (frontal)
- 2 sensores infrarrojos para deteccion de linea
- Conectividad WiFi

### Software
- Control manual via interfaz web
- Modo autonomo con evasion de obstaculos
- Sistema de rutas programables
- Dashboard con graficas en tiempo real usando Chart.js
- Almacenamiento de datos en Firebase
- Sincronizacion de tiempo via NTP

## Estructura del Proyecto

```
ESPCAR32/
├── Micro.ino          # Codigo principal del ESP32
├── index.html         # Dashboard de datos ambientales
├── control.html       # Interfaz de control del vehiculo
├── script.js          # Logica del dashboard
├── control.js         # Logica del control manual
└── style.css          # Estilos de las interfaces web
```

## Configuracion

### Requisitos

#### Hardware
- ESP32 (cualquier variante compatible)
- Sensor SCD30
- Sensor ultrasonico HC-SR04
- 2 sensores infrarrojos
- 4 motores DC con driver/puente H
- Fuente de alimentacion adecuada

#### Software
- Arduino IDE o PlatformIO
- Librerias de Arduino:
  - WiFi.h
  - HTTPClient.h
  - Wire.h
  - ArduinoJson
  - SparkFun_SCD30_Arduino_Library

### Instalacion

1. Clona este repositorio:
```bash
git clone https://github.com/tu-usuario/ESPCAR32.git
```

2. Abre `Micro.ino` en Arduino IDE

3. Instala las librerias necesarias desde el gestor de librerias de Arduino

4. Configura las credenciales de WiFi y Firebase en el codigo:
```cpp
const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";
const char* firebaseHost = "https://tu-proyecto.firebaseio.com";
```

5. Configura Firebase:
   - Crea un proyecto en Firebase Console
   - Habilita Realtime Database
   - Configura las reglas de seguridad segun tus necesidades
   - Actualiza la URL en `script.js` y `control.js`

6. Sube el sketch al ESP32

7. Abre los archivos HTML en un navegador web

## Uso

### Dashboard Ambiental (index.html)
- Visualiza graficas en tiempo real de CO2, temperatura y humedad
- Muestra historial de registros en tabla
- Acceso al control del vehiculo

### Control del Vehiculo (control.html)
- **Modo Manual**: Control directo con botones (adelante, atras, izquierda, derecha, detener)
- **Rutas Programadas**: Crea secuencias de movimientos, guardalas y ejecutalas

### Modo Automatico
El vehiculo navega autonomamente usando:
- Sensor ultrasonico frontal para deteccion de obstaculos
- Sensores IR para seguimiento de linea
- Algoritmo de evasion de obstaculos

## Conexiones de Pines

### Motores
- Motor 1: IN1=4, IN2=5
- Motor 2: IN1=7, IN2=6
- Motor 3: IN1=12, IN2=11
- Motor 4: IN1=13, IN2=14

### Sensores
- Ultrasonico: TRIG=41, ECHO=40
- IR Derecho: Pin 36
- IR Izquierdo: Pin 1
- SCD30 I2C: SDA=8, SCL=9

## Estructura de Datos en Firebase

```json
{
  "modo": "manual" | "automatico",
  "comando": "adelante" | "atras" | "izquierda" | "derecha" | "detener",
  "ejecutarRuta": true | false,
  "ruta": ["adelante", "derecha", "adelante", ...],
  "datos": {
    "co2": 400.0,
    "temperatura": 25.0,
    "humedad": 60.0,
    "timestamp": "2026-02-19T03:29:00"
  }
}
```

## Contribuciones

Las contribuciones son bienvenidas. Por favor:
1. Haz fork del proyecto
2. Crea una rama para tu feature (`git checkout -b feature/nueva-funcionalidad`)
3. Commit tus cambios (`git commit -m 'Agrega nueva funcionalidad'`)
4. Push a la rama (`git push origin feature/nueva-funcionalidad`)
5. Abre un Pull Request

## Licencia

Este proyecto esta licenciado bajo la Licencia MIT. Ver el archivo `LICENSE` para mas detalles.

## Autor

Desarrollado para proyectos de robotica educativa y monitoreo ambiental.

## Soporte

Para reportar problemas o sugerencias, por favor abre un issue en el repositorio.
