# DIFUSIVIDAD TÉRMICA APARENTE DEL SUELO MEDIANTE ATENUACIÓN DE LA AMPLITUD TÉRMICA DIARIA: ESTUDIO COMPARATIVO SAN JUAN - HUELVA

Este repositorio contiene el stack de software para la recolección, almacenamiento y visualización de datos provenientes de una sonda de medición de temperaturas subterráneas instalada en la Facultad de Ingeniería de la Universidad Nacional de San Juan, Argentina. El sistema está diseñado para el monitoreo continuo del perfil térmico del suelo, generando datos útiles para estudios de difusividad térmica y análisis de sistemas termodinámicos.

## Diseño Físico del sistema

El hardware de medición está estructurado para garantizar alta confiabilidad y tolerancia a fallos en las lecturas, utilizando sensores digitales DS18B20 distribuidos estratégicamente,  conectados mediante protocolo 1-Wire a una placa de Desarrollo Arduino UNO seleccionada para tal fin:
* **Profundidad de medición:** Desde la superficie hasta los 3 metros de profundidad.
* **Resolución espacial:** Un nivel de medición cada 50 cm.
* **Redundancia:** 4 sensores independientes por cada nivel, lo que permite promediar lecturas y descartar valores anómalos.

## Arquitectura de Software

El proyecto utiliza una arquitectura basada en microservicios gestionada con Docker Compose:
* **Ingestor (Python):** Script encargado de leer las tramas de datos a través del puerto serial.
* **Telegraf:** Agente de recolección que recibe, procesa y formatea las métricas.
* **InfluxDB:** Base de datos de series temporales (TSDB) optimizada para almacenar el volumen de mediciones.
* **Grafana:** Plataforma de visualización para monitorear las fluctuaciones de temperatura.

## Instrucciones de instalación

Sigue estos pasos para clonar y levantar el entorno localmente:

1. **Clonar el repositorio:**
   git clone https://github.com/fedeleb/sondaima.git

2. **Ingresar al directorio:**
   cd sondaima

3. **Configurar el entorno:**
   Crea tu archivo de credenciales locales a partir de la plantilla:
   cp .env.example .env

4. **Levantar los contenedores:**
   docker compose up -d

## Acceso a los servicios
* **Grafana:** `http://localhost:3000` (Ver credenciales en el archivo `.env`)
* **InfluxDB:** `http://localhost:8086`

## Script de Recolección de datos para Arduino

El proyecto utiliza un Arduino UNO para la recolección y envío de datos a través del Puerto Serial. Para ello, el usuario debe cargarle a Arduino el Script diseñado para tal fin, el cual se encuentra en este repositorio dentro de la carpeta "Arduino".
