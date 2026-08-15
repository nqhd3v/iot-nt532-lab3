const mqtt = require("mqtt");
const readline = require("readline");

const BROKER = "mqtt://192.168.1.34:1883";
const TOPIC = "mmcl/nhom1/led";

const client = mqtt.connect(BROKER, { clientId: "pc-nhom1" });
const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
});

client.on("connect", () => {
  console.log("Connected:", BROKER);
  console.log('Enter "on" or "off" then Enter (Ctrl+C to Exit).');
  rl.prompt();
});

rl.on("line", (line) => {
  const cmd = line.trim().toUpperCase();

  if (cmd !== "ON" && cmd !== "OFF") {
    console.log("ON/OFF is acceptable.");
  } else {
    client.publish(TOPIC, cmd);
    console.log(`Published "${cmd}" to topic ${TOPIC}`);
  }
  rl.prompt();
});

client.on("error", (err) => {
  console.error("Error:", err.message);
});
