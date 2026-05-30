// =====================================================
// SISTEMA DE RIEGO AUTOMÁTICO PROPORCIONAL
// =====================================================

// ================= CONFIGURACIÓN =================

// Sensores de humedad
#define PIN_SENSOR1 A4
#define PIN_SENSOR2 A5

// Puente H
#define PIN_PWM 9
#define PIN_IN1 7
#define PIN_IN2 8

// Botón de parada y LED
#define PIN_BOTON 2
#define PIN_LED 3

// Calibración sensores
// 230 = 100% humedad
// 415 = 0% humedad

#define HUMEDO 230
#define SECO 415

// Objetivo de humedad
#define OBJETIVO 40

// Tiempo entre mediciones
#define TIEMPO_MUESTREO 1000

// Zona muerta para evitar oscilaciones
#define TOLERANCIA 2

// ================= VARIABLES =================

unsigned long tiempoAnterior = 0;

int lectura1 = 0;
int lectura2 = 0;

float humedad1 = 0;
float humedad2 = 0;
float humedadPromedio = 0;

int velocidadBomba = 0;

// ================= SETUP =================

void setup() {

  Serial.begin(9600);

  // Sensores
  pinMode(PIN_SENSOR1, INPUT);
  pinMode(PIN_SENSOR2, INPUT);

  // Puente H
  pinMode(PIN_PWM, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);

  // Botón y LED
  pinMode(PIN_BOTON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);

  // Dirección de giro de la bomba
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);

  apagarBomba();

  Serial.println("=================================");
  Serial.println(" SISTEMA DE RIEGO PROPORCIONAL ");
  Serial.println("=================================");
  Serial.println("H1%\tH2%\tPROM%\tPWM");
}

// ================= LOOP =================

void loop() {

  // Parada de emergencia
  if (boton_parada_presionado()) {
    parada();
  }

  // Muestreo periódico
  if (debo_muestrear()) {

    // Lectura de sensores
    lectura1 = analogRead(PIN_SENSOR1);
    lectura2 = analogRead(PIN_SENSOR2);

    // Conversión a porcentaje
    humedad1 = convertirHumedad(lectura1);
    humedad2 = convertirHumedad(lectura2);

    // Promedio
    humedadPromedio = (humedad1 + humedad2) / 2.0;

    // Control proporcional
    controlarBomba();

    // Mostrar datos
    mostrarDatos();
  }
}

// ================= CONTROL PROPORCIONAL =================

void controlarBomba() {

  // Error respecto al objetivo
  float error = OBJETIVO - humedadPromedio;

  /*
    Si humedad es menor al objetivo:
    activar bomba proporcionalmente
  */

  if (error > TOLERANCIA) {

    /*
      Conversión proporcional:

      Poco error  -> poca velocidad
      Mucho error -> mucha velocidad
    */

    velocidadBomba = map(error * 10, 0, 400, 80, 255);

    // Limitar PWM
    velocidadBomba = constrain(velocidadBomba, 80, 255);

    // Aplicar velocidad
    analogWrite(PIN_PWM, velocidadBomba);

    // Encender LED
    digitalWrite(PIN_LED, HIGH);

  }
  else {

    apagarBomba();
  }
}

// ================= CONVERSIÓN A PORCENTAJE =================

float convertirHumedad(int lectura) {

  float humedad;

  humedad = ((float)(SECO - lectura) / (SECO - HUMEDO)) * 100.0;

  humedad = constrain(humedad, 0, 100);

  return humedad;
}

// ================= FUNCIONES =================

bool debo_muestrear() {

  unsigned long ahora = millis();

  if (ahora - tiempoAnterior >= TIEMPO_MUESTREO) {

    tiempoAnterior = ahora;
    return true;
  }

  return false;
}

// ================= APAGAR BOMBA =================

void apagarBomba() {

  analogWrite(PIN_PWM, 0);

  velocidadBomba = 0;

  digitalWrite(PIN_LED, LOW);
}

// ================= BOTÓN =================

bool boton_parada_presionado() {

  return digitalRead(PIN_BOTON) == LOW;
}

// ================= PARADA DE EMERGENCIA =================

void parada() {

  apagarBomba();

  Serial.println("PARADA DE EMERGENCIA ACTIVADA");

  while (true) {

    digitalWrite(PIN_LED, HIGH);
    delay(200);

    digitalWrite(PIN_LED, LOW);
    delay(200);
  }
}

// ================= MONITOR SERIAL =================

void mostrarDatos() {

  Serial.print(humedad1);
  Serial.print("\t");

  Serial.print(humedad2);
  Serial.print("\t");

  Serial.print(humedadPromedio);
  Serial.print("\t");

  Serial.println(velocidadBomba);
}
