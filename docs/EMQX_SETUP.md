# EMQX Cloud setup

## Create the deployment

1. Create a dedicated EMQX Cloud deployment.
2. Enable MQTT over TLS for the ESP32 client (commonly port 8883).
3. Enable MQTT over WebSocket Secure for the browser dashboard. Copy the exact WSS URL and path shown by your deployment; the path is often `/mqtt`, but use the value EMQX provides.
4. Create distinct usernames/passwords for the ESP32 and dashboard.
5. Use TLS certificates and authenticated clients. Do not use a public anonymous broker for real equipment.

## Topic access policy

Apply least-privilege ACLs. Suggested permissions:

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

## Browser connection

Use the deployment's WSS endpoint in the dashboard, including the correct port and WebSocket path. The browser uses MQTT.js. Do not copy the ESP32 credentials into the dashboard; create a separate restricted dashboard identity.

**Security note:** credentials entered into a static browser app are visible to the person using that browser and can be inspected in developer tools. This is suitable only for a trusted single-user demonstration with restricted ACLs. For a public or multi-user system, put authentication in a backend and issue short-lived, narrowly scoped broker credentials.
