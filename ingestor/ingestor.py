# serial_to_influx.py (Linux/Docker + muestreo cada 30 min, snapshot completo por N sensores)

import os
import csv
import time
import serial
from influxdb_client import InfluxDBClient, Point, WriteOptions

# =============== CONFIGURACIÓN (ENV) =================
SERIAL_PORT = os.getenv("SERIAL_PORT", "/dev/ttyUSB0")
BAUDRATE = int(os.getenv("BAUDRATE", "115200"))

INFLUX_URL = os.getenv("INFLUX_URL", "http://influxdb:8086")
INFLUX_TOKEN = os.getenv("INFLUX_TOKEN", "")
INFLUX_ORG = os.getenv("INFLUX_ORG", "mi_org")
INFLUX_BUCKET = os.getenv("INFLUX_BUCKET", "temperaturas")

MEASUREMENT = os.getenv("MEASUREMENT", "ds18b20")

# muestreo: 1800 = 30 min, 3600 = 1 h
SAMPLE_SECONDS = int(os.getenv("SAMPLE_SECONDS", "1800"))

# NUEVO: cuántos sensores esperás ver antes del primer snapshot (4 ahora, 28 después)
EXPECT_SENSORS = int(os.getenv("EXPECT_SENSORS", "4"))

# NUEVO: si no aparecen todos los sensores, igual guardá algo tras X segundos (evita “quedarse esperando”)
STARTUP_MAX_WAIT = int(os.getenv("STARTUP_MAX_WAIT", "30"))  # segundos

USE_TCORR = True
DISCARD_INVALID = True
RANGE_MIN_C = -55.0
RANGE_MAX_C = 125.0
# ====================================================

def to_float(s):
    try:
        return float(s)
    except Exception:
        return None

def parse_line(line: str):
    """
    Parsea una línea CSV del Arduino:
    t_ms,sensor,id,raw_c,tcorr_c
    """
    line = line.strip()
    if not line or line.startswith("#"):
        return None

    try:
        row = next(csv.reader([line]))
    except Exception:
        return None

    if len(row) != 5:
        return None

    t_ms, sensor, dev_id, raw_c, t_corr = [c.strip() for c in row]

    # ---- Cabecera ----
    if [c.lower() for c in row] == ["t_ms", "sensor", "id", "raw_c", "tcorr_c"]:
        return None
    if sensor.upper() == "SENSOR":
        return None

    raw_val = None if raw_c.upper() == "NA" else to_float(raw_c)
    corr_val = None if t_corr.upper() == "NA" else to_float(t_corr)

    return {
        "sensor": sensor,
        "id": dev_id,
        "raw_c": raw_val,
        "t_corr_c": corr_val
    }

def is_valid_value(v):
    return (v is not None) and (RANGE_MIN_C <= v <= RANGE_MAX_C)

def write_snapshot(write_api, latest: dict):
    """Escribe en Influx un snapshot de los últimos valores vistos por sensor."""
    # Importante: iteramos por sensor estable, para debug
    for sensor_key in sorted(latest.keys()):
        rec = latest[sensor_key]
        value = rec["t_corr_c"] if USE_TCORR else rec["raw_c"]
        if not is_valid_value(value):
            continue

        p = (
            Point(MEASUREMENT)
            .tag("sensor", rec["sensor"])
            .tag("id", rec["id"])
            .field("temperature_c", float(value))
        )

        if is_valid_value(rec["raw_c"]):
            p = p.field("raw_c", float(rec["raw_c"]))

        write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=p)

def main():
    print(f"[ingestor] Serial {SERIAL_PORT} @ {BAUDRATE}")
    print(f"[ingestor] Influx {INFLUX_URL} org={INFLUX_ORG} bucket={INFLUX_BUCKET}")
    print(f"[ingestor] Snapshot cada {SAMPLE_SECONDS}s")
    print(f"[ingestor] Esperando {EXPECT_SENSORS} sensores (max wait {STARTUP_MAX_WAIT}s)")

    client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
    write_api = client.write_api(write_options=WriteOptions(batch_size=1))

    latest = {}  # sensor -> último registro válido

    # NUEVO: control de primer snapshot
    start_time = time.time()
    first_snapshot_done = False
    last_write = time.time()  # para que el ritmo normal sea consistente

    with serial.Serial(SERIAL_PORT, BAUDRATE, timeout=2) as ser:
        while True:
            # 1) leer línea del serial
            try:
                raw = ser.readline().decode(errors="ignore")
            except Exception as e:
                print("[ingestor] Error serial:", e)
                time.sleep(1.0)
                continue

            # 2) parsear y actualizar "latest"
            rec = parse_line(raw)
            if rec is not None:
                value = rec["t_corr_c"] if USE_TCORR else rec["raw_c"]
                if (not DISCARD_INVALID) or is_valid_value(value):
                    latest[rec["sensor"]] = rec

            now = time.time()

            # 3) primer snapshot: cuando ya vimos todos los sensores esperados
            if not first_snapshot_done:
                enough = (EXPECT_SENSORS <= 0) or (len(latest) >= EXPECT_SENSORS)
                waited_too_long = (now - start_time >= STARTUP_MAX_WAIT)

                if enough or (latest and waited_too_long):
                    print(f"[ingestor] Primer snapshot: {len(latest)} sensores (enough={enough}, timeout={waited_too_long})")
                    try:
                        write_snapshot(write_api, latest)
                    except Exception as e:
                        print("[ingestor] Error Influx (primer snapshot):", e)

                    first_snapshot_done = True
                    last_write = now  # a partir de acá corre el periodo normal
                else:
                    continue  # todavía juntando sensores

            # 4) snapshots normales cada SAMPLE_SECONDS
            if latest and (now - last_write >= SAMPLE_SECONDS):
                print(f"[ingestor] Guardando snapshot ({len(latest)} sensores)")
                try:
                    write_snapshot(write_api, latest)
                except Exception as e:
                    print("[ingestor] Error Influx:", e)
                last_write = now

if __name__ == "__main__":
    while True:
        try:
            main()
        except Exception as e:
            print("[ingestor] Error general:", e)
            time.sleep(2.0)

