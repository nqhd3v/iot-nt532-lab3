const mqtt = require("mqtt");

const BROKER = "mqtt://192.168.1.34:1883";
const TOPIC_VALUE = "mmcl/nhom1/dht/value";
const TOPIC_DETECTED = "mmcl/nhom1/dht/detected";

const TEMP_MIN = 25,
  TEMP_MAX = 27; // do C
const HUMI_MIN = 40,
  HUMI_MAX = 70; // %

let lastAlarm = null; // chi publish khi trang thai thay doi

const client = mqtt.connect(BROKER, { clientId: "pc-bme680-nhom1" });

client.on("connect", () => {
  console.log("Connected to Broker:", BROKER);
  client.subscribe(TOPIC_VALUE, () => {
    console.log("Subscribed:", TOPIC_VALUE);
  });
});

client.on("message", (topic, payload) => {
  if (topic !== TOPIC_VALUE) return;

  let data;
  try {
    data = JSON.parse(payload.toString());
  } catch (e) {
    console.log("Payload invalid:", payload.toString());
    return;
  }

  const { temperature, humidity, pressure } = data;

  // de bai chi xet nhiet do va do am, ap suat chi de in ra cho de quan sat
  const abnormal =
    temperature < TEMP_MIN ||
    temperature > TEMP_MAX ||
    humidity < HUMI_MIN ||
    humidity > HUMI_MAX;

  const time = new Date().toLocaleTimeString();
  const state = abnormal ? "DANGER" : "NORMAL";
  console.log(
    `${time} | ${temperature.toFixed(1)} C | ` +
      `${humidity.toFixed(1)} % | ${pressure.toFixed(1)} hPa | ${state}`,
  );

  if (abnormal !== lastAlarm) {
    lastAlarm = abnormal;
    client.publish(TOPIC_DETECTED, JSON.stringify({ detected: abnormal }));
    console.log(`--> Publish {"detected": ${abnormal}} to ${TOPIC_DETECTED}`);
  }
});

client.on("error", (err) => {
  console.error("Error:", err.message);
});
