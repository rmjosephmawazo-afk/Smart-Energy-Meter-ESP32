# Smart Energy Meter ESP32

A real-time electricity monitoring and low-voltage LED control project using **ESP32 + PZEM-004T v3 + EMQX MQTT + Cloudflare Pages**.

## Features

- Live voltage (V), current (A), active power (W), and accumulated energy (kWh).
- Browser dashboard updates through MQTT over secure WebSockets (WSS).
- Three individually controlled indicator LEDs: red, blue, and green.
- LED buttons show the **reported state from the ESP32**. The UI does not claim success just because a button was clicked.
- Device online/offline status, connection status, and command timeout feedback.
- Responsive HTML/CSS/JavaScript frontend.
- ESP32 Arduino firmware example.
- No broker passwords or Wi-Fi credentials are committed to this repository.

## System architecture

```text
PZEM-004T v3 --UART--> ESP32 --MQTT/TLS--> EMQX Cloud
                         ^                    ^
                         |                    |
                   LED outputs          MQTT over WSS
                                              |
Browser dashboard -- HTTPS (Cloudflare Pages)|
```

The browser subscribes to meter telemetry, device status, and LED reported state. It publishes only allow-listed LED ON/OFF requests. The ESP32 validates each request and publishes the resulting state.

## Repository layout

- `frontend/` — browser dashboard.
- `esp32/energy_meter.ino` — ESP32 firmware.
- `docs/EMQX_SETUP.md` — broker and topic configuration.
- `docs/EMQX_CONNECTION_GUIDE.md` — step-by-step console navigation, MQTT credentials, listeners, and connection checks.
- `docs/CLOUDFLARE_DEPLOYMENT.md` — HTTPS hosting steps.
- `docs/SAFETY.md` — electrical safety boundaries.

## 1. Create an EMQX Cloud deployment

1. Create an EMQX Cloud account and a dedicated deployment.
2. Enable MQTT over WebSocket Secure (WSS) for browser clients and TLS MQTT for ESP32.
3. Create separate credentials for the ESP32 and dashboard. Give each identity only the topic permissions it needs.
4. Configure the broker endpoint, WSS path, ports, usernames and passwords as described in [EMQX setup](docs/EMQX_SETUP.md) and the [EMQX connection guide](docs/EMQX_CONNECTION_GUIDE.md).
5. Do not use a public shared broker for real device control.

The dashboard connection fields are intentionally entered at runtime. Do not put privileged broker credentials in JavaScript, Git, screenshots, or public documentation. Browser credentials can be inspected by the browser user; for a multi-user production deployment, use an authenticated backend to issue short-lived, restricted credentials.

## 2. Configure the ESP32

Install Arduino IDE and these libraries:
- **PZEM-004T v3.0** library by mandate/olehs
- **PubSubClient** by Nick O'Leary
- **ArduinoJson** by Benoit Blanchon

Open `esp32/energy_meter.ino`, set your Wi-Fi and EMQX values locally, and provide the correct broker root CA certificate in the firmware. Set the serial pins and LED GPIOs to match your board. Build and upload using the correct ESP32 board selection.

The example firmware publishes JSON telemetry to `energy/meter/data`, listens for the three LED set topics, and publishes confirmed LED states to `energy/led/state`.

**Important:** Do not guess PZEM wiring or work on exposed mains wiring. The PZEM measures hazardous AC circuits. Have the measurement installation, enclosure, isolation, and protection checked and completed by a qualified electrician/technician. See [Safety](docs/SAFETY.md).

## 3. Run the dashboard locally

Open `frontend/index.html` in a modern browser or serve the folder using a local static web server. In the dashboard:
1. Enter the EMQX WSS URL (including the broker's documented WebSocket path, often `/mqtt`).
2. Enter the dashboard-specific MQTT username and password.
3. Click **Connect**.
4. When the ESP32 is online, live readings and LED states appear.
5. Use the LED buttons. A button remains pending until the ESP32 reports the new state. If no state report arrives before the timeout, the dashboard shows that the command was not confirmed.

## 4. Deploy on Cloudflare Pages

Follow [Cloudflare deployment](docs/CLOUDFLARE_DEPLOYMENT.md). Deploy the `frontend` directory as the static site root. Use HTTPS and configure your EMQX WSS endpoint. No private credentials need to be added to the static site.

## MQTT topic contract

| Topic | Direction | Payload |
|---|---|---|
| `energy/meter/data` | ESP32 → dashboard | JSON: `voltage`, `current`, `power`, `energy`, `frequency`, `pf` |
| `energy/led/red/set` | dashboard → ESP32 | `ON` or `OFF` |
| `energy/led/blue/set` | dashboard → ESP32 | `ON` or `OFF` |
| `energy/led/green/set` | dashboard → ESP32 | `ON` or `OFF` |
| `energy/led/state` | ESP32 → dashboard | JSON reported LED states |
| `energy/device/status` | ESP32 → dashboard | `online` / `offline` (last will) |

## Important limitations

- PZEM energy is the meter module's accumulated reading; it is not automatically the remaining prepaid TANESCO LUKU balance.
- The sample firmware does not contain a LUKU token reader or utility-meter integration.
- LED controls are demonstration outputs only. Do not connect them directly to mains loads.
- The WSS credentials entered in a browser are not secret from that browser's user. Use restricted ACLs and a backend-issued short-lived credential flow for production or multiple users.

## Troubleshooting

- **Dashboard cannot connect:** check WSS endpoint, path, TLS, credentials, broker listener and browser console.
- **Dashboard connects but no readings:** verify ESP32 is online and publishing to the exact telemetry topic; check EMQX ACLs.
- **LED button stays pending:** check the set-topic subscription, ESP32 callback, state topic publication, and ACLs.
- **PZEM values are null:** verify the PZEM UART configuration and library version; check the meter module and sensor communication without exposing yourself to AC wiring.
- **Browser says MQTT library missing:** check internet access to the MQTT.js CDN.

## License

Choose and add a license before redistributing or deploying this project.
