(() => {
  "use strict";
  const $ = (id) => document.getElementById(id);
  const TOPICS = {
    telemetry: "energy/meter/data",
    state: "energy/led/state",
    status: "energy/device/status",
    leds: ["red", "blue", "green"]
  };
  const confirmed = { red: null, blue: null, green: null };
  const pending = new Set();
  let client = null;
  let commandTimer = null;

  function setConnection(connected, message) {
    $("brokerDot").classList.toggle("ok", connected);
    $("brokerStatus").textContent = connected ? "Broker connected" : "Disconnected";
    $("connectBtn").disabled = connected;
    $("disconnectBtn").disabled = !connected;
    TOPICS.leds.forEach((led) => { $(`${led}State`).textContent = confirmed[led] === null ? "Unknown" : (confirmed[led] ? "ON" : "OFF"); });
    $("connectionMessage").textContent = message;
    renderButtons();
  }
  function renderButtons() {
    TOPICS.leds.forEach((led) => {
      const btn = document.querySelector(`[data-led-button="${led}"]`);
      const card = document.querySelector(`[data-led="${led}"]`);
      const isPending = pending.has(led);
      btn.disabled = !client || !client.connected || isPending || confirmed[led] === null;
      btn.classList.toggle("pending", isPending);
      btn.textContent = isPending ? "Waiting for ESP32…" : (confirmed[led] ? "Turn OFF" : "Turn ON");
      card.classList.toggle("is-on", confirmed[led] === true);
      $(`${led}State`).textContent = isPending ? "Command sent · awaiting confirmation" : (confirmed[led] === null ? "Unknown" : (confirmed[led] ? "ON · confirmed" : "OFF · confirmed"));
    });
  }
  function parsePayload(message) {
    const raw = message.toString();
    try { return JSON.parse(raw); } catch { return raw; }
  }
  function updateTelemetry(data) {
    if (!data || typeof data !== "object") return;
    const fields = { voltage: "voltage", current: "current", power: "power", energy: "energy", frequency: "frequency", pf: "pf" };
    Object.entries(fields).forEach(([element, key]) => {
      const value = data[key];
      $(element).textContent = Number.isFinite(Number(value)) ? Number(value).toFixed(element === "energy" ? 3 : 2) : "—";
    });
    $("lastUpdate").textContent = new Date().toLocaleTimeString();
  }
  function updateLedState(data) {
    if (!data || typeof data !== "object") return;
    TOPICS.leds.forEach((led) => {
      if (typeof data[led] === "boolean") {
        confirmed[led] = data[led];
        pending.delete(led);
      }
    });
    if (data.red !== undefined || data.blue !== undefined || data.green !== undefined) {
      $("commandMessage").textContent = "LED states synchronized from the ESP32.";
    }
    renderButtons();
  }
  function handleMessage(topic, message) {
    const data = parsePayload(message);
    if (topic === TOPICS.telemetry) updateTelemetry(data);
    else if (topic === TOPICS.state) updateLedState(data);
    else if (topic === TOPICS.status) {
      const online = data === "online" || (data && data.status === "online");
      $("deviceDot").classList.toggle("ok", online);
      $("deviceStatus").textContent = online ? "Device online" : "Device offline";
    }
  }
  function disconnect() {
    if (commandTimer) clearTimeout(commandTimer);
    pending.clear();
    if (client) {
      const old = client;
      client = null;
      old.end(true);
    }
    setConnection(false, "Disconnected. Broker credentials are not stored.");
  }
  function connect() {
    if (!window.mqtt) {
      $("connectionMessage").textContent = "MQTT.js failed to load. Check internet access and reload the page.";
      return;
    }
    const url = $("brokerUrl").value.trim();
    const username = $("mqttUser").value.trim();
    const password = $("mqttPass").value;
    if (!url.startsWith("wss://") || !username || !password) {
      $("connectionMessage").textContent = "Enter a valid wss:// URL, username and password.";
      return;
    }
    if (client) disconnect();
    $("connectionMessage").textContent = "Connecting securely to EMQX…";
    const id = "energy-dashboard-" + Math.random().toString(16).slice(2, 10);
    client = window.mqtt.connect(url, {
      clientId: id, username, password, clean: true,
      connectTimeout: 10000, reconnectPeriod: 3000,
      keepalive: 30
    });
    client.on("connect", () => {
      setConnection(true, "Connected. Waiting for ESP32 telemetry and reported LED states.");
      client.subscribe([TOPICS.telemetry, TOPICS.state, TOPICS.status], { qos: 0 }, (err) => {
        if (err) $("connectionMessage").textContent = "Connected, but subscription failed: " + err.message;
      });
    });
    client.on("message", handleMessage);
    client.on("reconnect", () => $("connectionMessage").textContent = "Connection lost; trying to reconnect…");
    client.on("error", (err) => $("connectionMessage").textContent = "MQTT error: " + err.message);
    client.on("close", () => {
      if (client) setConnection(false, "Broker connection closed.");
    });
    renderButtons();
  }
  function sendLedCommand(led) {
    if (!client || !client.connected || confirmed[led] === null || pending.has(led)) return;
    const desired = confirmed[led] ? "OFF" : "ON";
    pending.add(led);
    renderButtons();
    $("commandMessage").textContent = `Sent ${desired} request to ${led} LED. Waiting for ESP32 confirmation…`;
    client.publish(`energy/led/${led}/set`, desired, { qos: 1, retain: false }, (err) => {
      if (err) {
        pending.delete(led); renderButtons();
        $("commandMessage").textContent = "Command publish failed: " + err.message;
      }
    });
    window.setTimeout(() => {
      if (pending.has(led)) {
        pending.delete(led); renderButtons();
        $("commandMessage").textContent = `${led} LED command was not confirmed. Check ESP32 connection and MQTT topic permissions.`;
      }
    }, 5000);
  }
  $("connectBtn").addEventListener("click", connect);
  $("disconnectBtn").addEventListener("click", disconnect);
  document.querySelectorAll("[data-led-button]").forEach((button) => {
    button.addEventListener("click", () => sendLedCommand(button.dataset.ledButton));
  });
  setConnection(false, "Enter your EMQX WSS endpoint and dashboard credentials to connect.");
  renderButtons();
})();
