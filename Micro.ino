#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <time.h>
#include <ArduinoJson.h>
#include "SparkFun_SCD30_Arduino_Library.h"
#include <algorithm>

// === Pines Motores ===
#define M1_IN1  4
#define M1_IN2  5
#define M2_IN1  7
#define M2_IN2  6
#define M3_IN1 12
#define M3_IN2 11
#define M4_IN1 13
#define M4_IN2 14

// === Sensores ===
#define TRIG_FRENTE 41
#define ECHO_FRENTE 40
#define PIN_SENSOR_IR     36
#define PIN_SENSOR_IR_IZQ  1

// === SCD30 I2C ===
#define SDA_PIN 8
#define SCL_PIN 9

// === Firebase ===
const char* ssid = "Pepito";
const char* password = "123456789";
const char* firebaseHost = "https://espcar32-default-rtdb.firebaseio.com";
const char* firebasePath = "/datos.json"; // donde se suben datos ambientales

// === Variables ===
SCD30 scd30;
int espera = 100;
int giro = 450;
bool yaAvanzoPorNAN = false;
int memoria = 1;

// === Sensor ultrasónico ===
const uint8_t MUESTRAS_POR_SENSOR = 3;
const uint32_t TIMEOUT_US = 25000UL;
const uint16_t ESPERA_ENTRE_SENSORES_MS = 60;
const float FACTOR_CM_POR_US = 0.0343f / 2.0f;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Pines motores
  int motores[] = {M1_IN1, M1_IN2, M2_IN1, M2_IN2, M3_IN1, M3_IN2, M4_IN1, M4_IN2};
  for (int i = 0; i < 8; i++) {
    pinMode(motores[i], OUTPUT);
    digitalWrite(motores[i], LOW);
  }

  // Pines sensores
  pinMode(TRIG_FRENTE, OUTPUT); digitalWrite(TRIG_FRENTE, LOW);
  pinMode(ECHO_FRENTE, INPUT);
  pinMode(PIN_SENSOR_IR, INPUT);
  pinMode(PIN_SENSOR_IR_IZQ, INPUT);

  // I2C + SCD30
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!scd30.begin()) {
    Serial.println("❌ Sensor SCD30 no detectado.");
    while (true);
  }

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("🔌 Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n✅ WiFi conectado.");

  // Hora por NTP
  configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("⏳ Sincronizando hora");
  while (time(nullptr) < 100000) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n⏰ Hora sincronizada.");
}

void loop() {
  // === Subida de datos del sensor SCD30 ===
  if (scd30.dataAvailable()) {
    float co2 = scd30.getCO2();
    float temp = scd30.getTemperature();
    float hum = scd30.getHumidity();

    String json = "{";
    json += "\"co2\":" + String(co2, 2) + ",";
    json += "\"temperatura\":" + String(temp, 2) + ",";
    json += "\"humedad\":" + String(hum, 2) + ",";
    json += "\"timestamp\":\"" + getTimestamp() + "\"";
    json += "}";

    HTTPClient http;
    String url = String(firebaseHost) + firebasePath;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.POST(json);
    http.end();
    Serial.println("📤 Datos ambientales enviados.");
  }

  // === Control desde Firebase ===
  String modo = leerFirebase("/modo");

  if (modo == "manual") {
    String cmd = leerFirebase("/comando");
    ejecutarComando(cmd);
  } else if (modo == "automatico") {
    ejecutarLogicaAutonoma();
  }

  String ejecutar = leerFirebase("/ejecutarRuta");
  if (ejecutar == "true") {
    ejecutarRuta();
  }

  delay(500);
}

// === Lógica autónoma por sensores ===
void ejecutarLogicaAutonoma() {
  float frente = sensorFrente();
  memoria = digitalRead(PIN_SENSOR_IR) == LOW ? 1 : 0;
  int izquierda = digitalRead(PIN_SENSOR_IR_IZQ) == LOW ? 1 : 0;

  if (memoria == 0) {
    avanzar();
    girarDerecha();
    girarDerecha();
  }

  if ((frente < 15) && memoria == 1) {
    girarIzquierda();
    girarIzquierda();
  }

  if (frente >= 10) {
    avanzar();
  }

  if ((izquierda == 0) && (memoria == 0)){
    girarIzquierda();
    girarIzquierda();
    avanzar();
    avanzar();
    avanzar();
  }

  if (isnan(frente)){
    avanzar();
    girarIzquierda();
    avanzar();
  }
}

// === Ejecutar comando manual ===
void ejecutarComando(String cmd) {
  if (cmd == "adelante") avanzar();
  else if (cmd == "atras") retroceder();
  else if (cmd == "izquierda") girarIzquierda();
  else if (cmd == "derecha") girarDerecha();
  else if (cmd == "detener") detener();
}

// === Ejecutar ruta completa desde Firebase ===
void ejecutarRuta() {
  HTTPClient http;
  String url = String(firebaseHost) + "/ruta.json";
  http.begin(url);
  int code = http.GET();

  if (code > 0) {
    String payload = http.getString();
    Serial.println("📥 Ruta recibida: " + payload);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    for (JsonVariant paso : doc.as<JsonArray>()) {
      String cmd = paso.as<String>();
      Serial.println("➡️ Paso: " + cmd);
      ejecutarComando(cmd);
      delay(500);
    }

    // Desactivar bandera ejecutarRuta
    HTTPClient put;
    put.begin(String(firebaseHost) + "/ejecutarRuta.json");
    put.addHeader("Content-Type", "application/json");
    put.PUT("false");
    put.end();
  }

  http.end();
}

// === Movimiento ===
void avanzar() {
  Serial.println("➡️ AVANZAR");
  digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, HIGH); digitalWrite(M2_IN2, LOW);
  digitalWrite(M3_IN1, HIGH); digitalWrite(M3_IN2, LOW);
  digitalWrite(M4_IN1, HIGH); digitalWrite(M4_IN2, LOW);
  delay(espera);
  detener();
}

void retroceder() {
  Serial.println("⬅️ RETROCEDER");
  digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN1, LOW); digitalWrite(M2_IN2, HIGH);
  digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, HIGH);
  digitalWrite(M4_IN1, LOW); digitalWrite(M4_IN2, HIGH);
  delay(espera);
  detener();
}

void girarIzquierda() {
  Serial.println("↩️ GIRAR IZQUIERDA");
  digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN1, HIGH); digitalWrite(M2_IN2, LOW);
  digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, HIGH);
  digitalWrite(M4_IN1, HIGH); digitalWrite(M4_IN2, LOW);
  delay(giro);
  detener();
}

void girarDerecha() {
  Serial.println("↪️ GIRAR DERECHA");
  digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, LOW); digitalWrite(M2_IN2, HIGH);
  digitalWrite(M3_IN1, HIGH); digitalWrite(M3_IN2, LOW);
  digitalWrite(M4_IN1, LOW); digitalWrite(M4_IN2, HIGH);
  delay(giro);
  detener();
}

void detener() {
  Serial.println("🛑 DETENER");
  int motores[] = {M1_IN1, M1_IN2, M2_IN1, M2_IN2, M3_IN1, M3_IN2, M4_IN1, M4_IN2};
  for (int i = 0; i < 8; i++) digitalWrite(motores[i], LOW);
}

// === Sensor ultrasónico ===
float medirDistanciaCm(uint8_t trig, uint8_t echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  unsigned long dur = pulseIn(echo, HIGH, TIMEOUT_US);
  if (dur == 0) return NAN;

  float distancia = dur * FACTOR_CM_POR_US;
  if (distancia < 0.5f || distancia > 450.0f) return NAN;
  return distancia;
}

float medirMediana(uint8_t trig, uint8_t echo, uint8_t muestras) {
  float valores[muestras];
  uint8_t validas = 0;

  for (uint8_t i = 0; i < muestras; i++) {
    float d = medirDistanciaCm(trig, echo);
    if (!isnan(d)) {
      valores[validas++] = d;
    }
    delay(10);
  }

  if (validas == 0) return NAN;

  std::sort(valores, valores + validas);

  if (validas % 2 == 1) return valores[validas / 2];
  else return (valores[validas / 2 - 1] + valores[validas / 2]) / 2.0;
}

float sensorFrente() {
  float d = medirMediana(TRIG_FRENTE, ECHO_FRENTE, MUESTRAS_POR_SENSOR);
  delay(ESPERA_ENTRE_SENSORES_MS);
  return d;
}

// === Hora actual ===
String getTimestamp() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  char buffer[25];
  sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02d",
          p_tm->tm_year + 1900, p_tm->tm_mon + 1, p_tm->tm_mday,
          p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
  return String(buffer);
}

// === Leer datos desde Firebase ===
String leerFirebase(String path) {
  HTTPClient http;
  String url = String(firebaseHost) + path + ".json";
  http.begin(url);
  int code = http.GET();
  String data = "";

  if (code > 0) {
    data = http.getString();
    data.replace("\"", "");
  }

  http.end();
  return data;
}
