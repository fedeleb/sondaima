#include <OneWire.h>
#include <DallasTemperature.h>

/* ================= CONFIG ================= */
const uint32_t PERIODO_MS   = 5000;   // cada cuánto imprimir (tu salida actual). Podés subirlo luego.
const uint16_t RES_MS_12BIT = 750;    // tiempo de conversión 12-bit
const uint8_t  RES_BITS     = 12;

// Pines por nivel (ajustá si hace falta)
const uint8_t PIN_L1   = 7;  // Nivel 1
const uint8_t PIN_L0L2 = 6;  // Nivel 0 + Nivel 2 (bus compartido)
const uint8_t PIN_L3   = 5;  // Nivel 3
const uint8_t PIN_L4   = 4;  // Nivel 4
const uint8_t PIN_L5   = 3;  // Nivel 5  <<< si no es D3, cambialo acá
const uint8_t PIN_L6   = 2;  // Nivel 6

const uint8_t N_TOTAL = 28;

/* ============ BUS 1-WIRE POR PIN ============ */
OneWire ow_l1(PIN_L1);
OneWire ow_l0l2(PIN_L0L2);
OneWire ow_l3(PIN_L3);
OneWire ow_l4(PIN_L4);
OneWire ow_l5(PIN_L5);
OneWire ow_l6(PIN_L6);

DallasTemperature dt_l1(&ow_l1);
DallasTemperature dt_l0l2(&ow_l0l2);
DallasTemperature dt_l3(&ow_l3);
DallasTemperature dt_l4(&ow_l4);
DallasTemperature dt_l5(&ow_l5);
DallasTemperature dt_l6(&ow_l6);

enum BusId : uint8_t { BUS_L1, BUS_L0L2, BUS_L3, BUS_L4, BUS_L5, BUS_L6 };

struct SensorMap {
  BusId bus;
  DeviceAddress rom;
  double a;
  double b;
};

/* ================== UTILIDADES ================== */
void printAddress(const DeviceAddress addr) {
  for (uint8_t i = 0; i < 8; i++) {
    if (addr[i] < 16) Serial.print('0');
    Serial.print(addr[i], HEX);
  }
}

DallasTemperature* busToDT(BusId b) {
  switch (b) {
    case BUS_L1:   return &dt_l1;
    case BUS_L0L2: return &dt_l0l2;
    case BUS_L3:   return &dt_l3;
    case BUS_L4:   return &dt_l4;
    case BUS_L5:   return &dt_l5;
    case BUS_L6:   return &dt_l6;
  }
  return &dt_l0l2;
}

const char* busName(BusId b) {
  switch (b) {
    case BUS_L1:   return "L1(D7)";
    case BUS_L0L2: return "L0+L2(D6)";
    case BUS_L3:   return "L3(D5)";
    case BUS_L4:   return "L4(D4)";
    case BUS_L5:   return "L5(D3)";
    case BUS_L6:   return "L6(D2)";
  }
  return "?";
}

/* ===========================================================
   MAPEO: S0..S27 (en ese orden) = Sensores #1..#28 de tu tabla
   Bus por nivel:
     Nivel 0: S0..S3   -> BUS_L0L2
     Nivel 1: S4..S7   -> BUS_L1
     Nivel 2: S8..S11  -> BUS_L0L2
     Nivel 3: S12..S15 -> BUS_L3
     Nivel 4: S16..S19 -> BUS_L4
     Nivel 5: S20..S23 -> BUS_L5
     Nivel 6: S24..S27 -> BUS_L6
   =========================================================== */
SensorMap S[N_TOTAL] = {
  // NIVEL 0 (sensores 1..4) - bus compartido D6
  {BUS_L0L2, {0x28,0x59,0xA6,0x50,0x00,0x00,0x00,0xD1}, 0.9755347,  0.9443761}, // S0  (#1)
  {BUS_L0L2, {0x28,0xA3,0x2A,0x51,0x00,0x00,0x00,0x09}, 0.9755347,  0.8468226}, // S1  (#2)
  {BUS_L0L2, {0x28,0xDC,0xCC,0x50,0x00,0x00,0x00,0x7C}, 0.9695672,  0.9820309}, // S2  (#3)
  {BUS_L0L2, {0x28,0xF1,0xCD,0x53,0x00,0x00,0x00,0xC5}, 0.9735872,  0.8885135}, // S3  (#4)

  // NIVEL 1 (sensores 5..8) - D7
  {BUS_L1,   {0x28,0x7F,0xA4,0x50,0x00,0x00,0x00,0x56}, 0.9779287,  0.4207131}, // S4  (#5)
  {BUS_L1,   {0x28,0x28,0x6C,0x54,0x00,0x00,0x00,0xEF}, 0.9729730,  0.8594595}, // S5  (#6)
  {BUS_L1,   {0x28,0x49,0xAB,0x52,0x00,0x00,0x00,0x61}, 0.9746193,  0.5512690}, // S6  (#7)
  {BUS_L1,   {0x28,0x39,0x14,0x51,0x00,0x00,0x00,0x51}, 0.9729730,  0.3729730}, // S7  (#8)

  // NIVEL 2 (sensores 9..12) - bus compartido D6
  {BUS_L0L2, {0x28,0x0A,0x84,0x50,0x00,0x00,0x00,0xDD}, 0.9707549,  0.3516373},  // S8  (#9)
  {BUS_L0L2, {0x28,0x21,0xCE,0xBF,0x00,0x00,0x00,0x14}, 0.9780102, -0.1893578},  // S9  (#10)
  {BUS_L0L2, {0x28,0x81,0xD5,0xBC,0x00,0x00,0x00,0xCC}, 0.9639814,  0.5252215},  // S10 (#11)
  {BUS_L0L2, {0x28,0xC0,0x94,0x50,0x00,0x00,0x00,0xF1}, 0.9726928,  0.5070161},  // S11 (#12)

  // NIVEL 3 (sensores 13..16) - D5
  {BUS_L3,   {0x28,0xB0,0xB0,0xBD,0x00,0x00,0x00,0x5A}, 0.9775634,  0.3721198}, // S12 (#13)
  {BUS_L3,   {0x28,0xBC,0x97,0xBB,0x00,0x00,0x00,0x87}, 0.9773330,  0.6518462}, // S13 (#14)
  {BUS_L3,   {0x28,0x91,0x9F,0x52,0x00,0x00,0x00,0x9F}, 0.9755179,  0.5743384}, // S14 (#15)
  {BUS_L3,   {0x28,0xBD,0x68,0x53,0x00,0x00,0x00,0x2C}, 0.9764982,  0.4086370}, // S15 (#16)

  // NIVEL 4 (sensores 17..20) - D4
  {BUS_L4,   {0x28,0x90,0xD7,0x52,0x00,0x00,0x00,0x7F}, 0.9950889, -0.0974713}, // S16 (#17)
  {BUS_L4,   {0x28,0xFE,0x88,0x55,0x00,0x00,0x00,0xD3}, 0.9822030,  0.0293807}, // S17 (#18)
  {BUS_L4,   {0x28,0x17,0x2D,0x55,0x00,0x00,0x00,0x8D}, 0.9814145,  0.2992039}, // S18 (#19)
  {BUS_L4,   {0x28,0xEF,0xEF,0xBD,0x00,0x00,0x00,0xAA}, 0.9764559,  0.3330595}, // S19 (#20)

  // NIVEL 5 (sensores 21..24) - D3
  {BUS_L5,   {0x28,0x38,0xCD,0x54,0x00,0x00,0x00,0x4A}, 0.9672253, -2.1652659}, // S20 (#21)
  {BUS_L5,   {0x28,0x4C,0xD4,0xBD,0x00,0x00,0x00,0x5B}, 0.9639791, -2.0694363}, // S21 (#22)
  {BUS_L5,   {0x28,0x6C,0x5E,0x54,0x00,0x00,0x00,0x41}, 0.9715026, -1.8252918}, // S22 (#23)
  {BUS_L5,   {0x28,0x49,0x9E,0x53,0x00,0x00,0x00,0xB8}, 0.9654220, -1.7708591}, // S23 (#24)

  // NIVEL 6 (sensores 25..28) - D2
  {BUS_L6,   {0x28,0x50,0xE9,0xBD,0x00,0x00,0x00,0x15}, 0.9740506,  0.1767759}, // S24 (#25)
  {BUS_L6,   {0x28,0x04,0x34,0xBC,0x00,0x00,0x00,0xDA}, 0.9797209, -0.3870959}, // S25 (#26)
  {BUS_L6,   {0x28,0x45,0x5C,0x55,0x00,0x00,0x00,0x6D}, 0.9754170,  0.2793772}, // S26 (#27)
  {BUS_L6,   {0x28,0x1F,0xCF,0xBC,0x00,0x00,0x00,0xAF}, 0.9711882,  0.2569655}, // S27 (#28)
};

/* ================== SETUP ================== */
void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println(F("# Escaneando bus 1-Wire..."));

  // Iniciar buses
  dt_l1.begin();   dt_l1.setWaitForConversion(false);
  dt_l0l2.begin(); dt_l0l2.setWaitForConversion(false);
  dt_l3.begin();   dt_l3.setWaitForConversion(false);
  dt_l4.begin();   dt_l4.setWaitForConversion(false);
  dt_l5.begin();   dt_l5.setWaitForConversion(false);
  dt_l6.begin();   dt_l6.setWaitForConversion(false);

  // Setear resolucion y chequear conectividad
  uint8_t nOk = 0;
  for (uint8_t i=0; i<N_TOTAL; i++) {
    DallasTemperature* dt = busToDT(S[i].bus);
    dt->setResolution(S[i].rom, RES_BITS);
  }

  // “Sensores encontrados” = cuántos responden
  for (uint8_t i=0; i<N_TOTAL; i++) {
    DallasTemperature* dt = busToDT(S[i].bus);
    if (dt->isConnected(S[i].rom)) nOk++;
  }

  Serial.print(F("# Sensores encontrados: "));
  Serial.println(nOk);

  for (uint8_t i=0; i<N_TOTAL; i++) {
    Serial.print(F("# S")); Serial.print(i);
    Serial.print(F(" ID="));
    printAddress(S[i].rom);
    Serial.print(F("  res=12bit  a=")); Serial.print(S[i].a, 6);
    Serial.print(F("  b=")); Serial.println(S[i].b, 6);
  }

  // Cabecera CSV EXACTA
  Serial.println(F("t_ms,SENSOR,ID,RAW_C,TCORR_C"));
}

/* ================== LOOP ================== */
void loop() {
  static uint32_t t0 = 0;
  uint32_t now = millis();
  if (now - t0 < PERIODO_MS) return;
  t0 = now;

  // Disparar conversión en todos los buses
  dt_l1.requestTemperatures();
  dt_l0l2.requestTemperatures();
  dt_l3.requestTemperatures();
  dt_l4.requestTemperatures();
  dt_l5.requestTemperatures();
  dt_l6.requestTemperatures();

  delay(RES_MS_12BIT);

  // Emitir una línea CSV por sensor S0..S27
  for (uint8_t i=0; i<N_TOTAL; i++) {
    DallasTemperature* dt = busToDT(S[i].bus);
    float raw = dt->getTempC(S[i].rom);

    // inválidas típicas DS18B20: -127 o 85
    if (raw <= -100.0f || raw == 85.0f) {
      Serial.print(now); Serial.print(F(",S")); Serial.print(i); Serial.print(F(","));
      printAddress(S[i].rom);
      Serial.println(F(",NA,NA"));
      continue;
    }

    double tCorr = S[i].a * raw + S[i].b;

    Serial.print(now); Serial.print(F(",S")); Serial.print(i); Serial.print(F(","));
    printAddress(S[i].rom); Serial.print(F(","));
    Serial.print(raw, 4); Serial.print(F(","));
    Serial.println(tCorr, 4);
  }
}
