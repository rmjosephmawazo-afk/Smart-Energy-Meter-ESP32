# EMQX Cloud setup

## Confirmed connection settings for this deployment

The user's EMQX Deployment Overview screenshot confirms these listeners:

| Client | Protocol | Port | Endpoint |
|---|---|---:|---|
| ESP32 | MQTT over TLS | `8883` | `f42fe1e0.ala.dedicated.aws.emqxcloud.com:8883` |
| Browser | MQTT over WSS | `8084` | `wss://f42fe1e0.ala.dedicated.aws.emqxcloud.com:8084/mqtt` |

EMQX's Cloud JavaScript SDK guide documents `/mqtt` as the WebSocket path. The dashboard's WSS URL is already prefilled in `frontend/index.html`.

## Create MQTT client authentication

1. In the EMQX Cloud console, open the deployment.
2. Select **Access Control → Authentication** or **Client Authentication**.
3. If no authentication mechanism exists, add the built-in database/password authentication method and enable it.
4. Add a dedicated username and strong password for the ESP32 (for example, `energy-esp32`).
5. Add a different dedicated username and strong password for the dashboard (for example, `energy-dashboard`).
6. Keep the passwords private. Do not paste them into chat or commit them to GitHub.
7. Verify the authentication mechanism is enabled before testing clients.

The EMQX management API key/secret is not an MQTT username/password. Use only the two MQTT client accounts in the firmware and dashboard.

## Topic access policy

Open **Access Control → Authorization** (or the deployment's topic permissions page) and apply least-privilege ACLs. Suggested permissions for the two MQTT users:

| Client | Publish | Subscribe |
|---|---|---|
| ESP32 | `energy/meter/data`, `energy/led/state`, `energy/device/status` | `energy/led/+/set` |
| Dashboard | `energy/led/+/set` | `energy/meter/data`, `energy/led/state`, `energy/device/status` |

If your ACL system does not support wildcards in publish rules, explicitly list the three LED set topics. Do not grant either client access to unrelated topics.

## Payload examples

Telemetry:
```json
{"voltage":230.4,"current":1.25,"power":287.0,"energy":12.345,"frequency":50.0,"pf":0.99}
```

LED command payload is exactly `ON` or `OFF`. The ESP32 ignores any other command.

Reported state:
```json
{"red":true,"blue":false,"green":false}
```

Device status: `online` or `offline`.

## Download and install the ESP32 CA certificate

1. Open the deployment's **Overview** page.
2. Under **MQTT Connection Information**, select **CA Certificate** and use its download icon.
3. Open the downloaded certificate as text. Copy the complete PEM certificate, including the `-----BEGIN CERTIFICATE-----` and `-----END CERTIFICATE-----` lines.
4. On your own computer, paste it into the `ROOT_CA` raw string in `esp32/energy_meter.ino`, replacing the placeholder text. Do not post it publicly.
5. Keep `tlsClient.setCACert(ROOT_CA)` enabled. Do not use `setInsecure()` to bypass certificate verification.

## Browser connection

Use the prefilled deployment WSS endpoint: `wss://f42fe1e0.ala.dedicated.aws.emqxcloud.com:8084/mqtt`. The browser uses MQTT.js. Do not copy the ESP32 credentials into the dashboard; create a separate restricted dashboard identity.

**Security note:** credentials entered into a static browser app are visible to the person using that browser and can be inspected in developer tools. This is suitable only for a trusted single-user demonstration with restricted ACLs. For a public or multi-user system, put authentication in a backend and issue short-lived, narrowly scoped broker credentials.
