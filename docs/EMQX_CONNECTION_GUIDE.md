# EMQX Cloud connection guide

This guide connects the ESP32 and browser dashboard to the same EMQX deployment. EMQX console labels can vary slightly by deployment type and console version.

## 1. Open the deployment

1. Sign in at https://cloud.emqx.com/console/deployments.
2. Open the deployment you want to use.
3. Open its **Overview** or **Connect** page.
4. Find **Connection Information**, **Connection Details**, or the deployment's client connection panel.
5. Copy the broker hostname exactly. Do not include `https://` unless the console explicitly shows a URL.

If the deployment list is empty, create a deployment first. Wait until its status is running before trying to connect.

## 2. Identify the listener addresses

You need two different MQTT transports:

| Client | Transport | Typical port | What to copy |
|---|---|---:|---|
| ESP32 firmware | MQTT over TLS | 8883 | Hostname and TLS port |
| Browser dashboard | MQTT over WebSocket Secure (WSS) | 8084 | Hostname, secure WebSocket port, and exact WebSocket path |

**Confirmed for this project's deployment from the user's EMQX Overview screenshot:** MQTT/TCP `1883`, MQTT/TLS `8883`, WebSocket `8083`, and WebSocket/TLS `8084`. The broker hostname is `f42fe1e0.ala.dedicated.aws.emqxcloud.com`. The screenshot does not show the WebSocket path, so copy that path from the EMQX Connection Guide or the deployment's client connection instructions; do not guess `/mqtt`.

The browser field in `frontend/index.html` expects a complete URL beginning with `wss://`. For this deployment, the confirmed beginning is `wss://f42fe1e0.ala.dedicated.aws.emqxcloud.com:8084`; append the exact WebSocket path shown in EMQX. The screenshot confirms the port but not the path. The general format is:

```text
wss://YOUR_DEPLOYMENT_HOST:YOUR_WSS_PORT/YOUR_WEBSOCKET_PATH
```

The ESP32 firmware expects only the broker hostname in `MQTT_HOST` and the TLS port in `MQTT_TLS_PORT`. This repository is already configured with the confirmed hostname and port `8883`.

## 3. Create MQTT client credentials

The API Key and API Secret are EMQX management/API credentials. They are **not** automatically MQTT client credentials.

In the deployment console, open **Access Control → Authentication** (or the equivalent authentication page), enable an appropriate password-based authentication method if needed, and create separate MQTT identities:

- `energy-esp32` — used only by the ESP32.
- `energy-dashboard` — used only by the browser dashboard.

Use strong, unique passwords. Do not share the passwords in chat, screenshots, source code, or Git commits.

## 4. Apply least-privilege topic permissions

Use the topic contract from [EMQX_SETUP.md](EMQX_SETUP.md).

- ESP32 may publish telemetry, LED reported state, and device status.
- ESP32 may subscribe only to the three LED set topics.
- Dashboard may subscribe to telemetry, LED reported state, and device status.
- Dashboard may publish only the three LED set topics.

If the deployment's authorization UI does not support topic wildcards for a rule, create one rule per topic.

## 5. Configure the ESP32 locally

Edit `esp32/energy_meter.ino` on your computer. Set:

- `WIFI_SSID` and `WIFI_PASSWORD`
- `MQTT_HOST`
- `MQTT_USERNAME` and `MQTT_PASSWORD`
- the trusted EMQX root CA in `ROOT_CA`

Keep these values local. Never commit real credentials or certificates with private keys. Do not replace certificate validation with `setInsecure()` for a deployed device.

The ESP32 connects on MQTT/TLS (normally port 8883) and publishes to `energy/meter/data`.

## 6. Configure the browser dashboard

Open the deployed dashboard over HTTPS, or serve `frontend/` locally. Enter:

1. The complete WSS URL shown by EMQX.
2. The `energy-dashboard` username.
3. Its password.

Click **Connect**. The page should show broker status as connected. When the ESP32 is online and publishing, measurements should update.

A static browser app cannot keep its MQTT password secret from the person using that browser. Use a dedicated, tightly restricted dashboard account for a private demonstration. For a public or multi-user application, put authentication behind a backend and issue short-lived, narrowly scoped credentials.

## 7. Keep management API credentials private

The EMQX API Key and API Secret are for management API operations. Do not place them in:

- ESP32 firmware
- `frontend/app.js` or any other browser file
- public GitHub repositories
- screenshots or support messages

If an API Secret has been exposed, revoke or rotate it in the EMQX console. Store any replacement only in a private server-side secret store or local environment file that is excluded by `.gitignore`.

## 8. Verify the data path

1. Confirm the deployment is running.
2. Confirm MQTT authentication and topic permissions are configured.
3. Confirm the ESP32 connects using TLS and publishes JSON to `energy/meter/data`.
4. Confirm the dashboard connects using WSS and subscribes to the exact same topic.
5. If the broker connects but readings remain blank, check the ESP32 serial monitor and EMQX client/topic inspection tools.

## Troubleshooting

- **Connection refused / timeout:** verify deployment status, hostname, port, network access, and listener type.
- **Not authorized:** verify the MQTT username/password and topic ACLs.
- **Browser WebSocket error:** verify WSS is enabled and that the exact WSS port and path from EMQX are entered.
- **TLS certificate error on ESP32:** use the correct trusted root CA certificate and ensure the device clock is valid.
- **Connected but no readings:** verify both clients use the exact topic `energy/meter/data` and inspect the ESP32 serial output.

## Electrical safety

PZEM-004T measures hazardous mains AC. Do not work on exposed or energized mains wiring. Have installation, isolation, enclosure, overcurrent protection, earthing, and verification completed by a qualified electrician or technician. The dashboard and example firmware are not certified protective equipment.

## This project's broker hostname

The user-provided deployment hostname for this project is:

```text
f42fe1e0.ala.dedicated.aws.emqxcloud.com
```

Use this exact hostname in the ESP32 `MQTT_HOST` setting. Confirmed listener ports from the deployment Overview screenshot: MQTT/TCP `1883`, MQTT/TLS `8883`, WebSocket `8083`, and WebSocket/TLS `8084`. The WebSocket path still needs to be copied from EMQX's client connection instructions. The ESP32 firmware uses the confirmed MQTT/TLS port `8883`.
