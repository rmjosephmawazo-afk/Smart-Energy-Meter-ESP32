# Frontend

Static HTML/CSS/JavaScript dashboard. MQTT.js is loaded from its public CDN and connects to EMQX using MQTT over WSS.

Open `index.html` through a local static web server or deploy this directory to Cloudflare Pages. Enter the WSS endpoint and dashboard-only credentials in the connection panel.

The UI only changes an LED's displayed confirmed state when the ESP32 publishes `energy/led/state`. A click sends an ON/OFF request; it is not itself treated as confirmation.
