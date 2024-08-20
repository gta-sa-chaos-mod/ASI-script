const WebSocket = require('ws');

let client = new WebSocket("ws://localhost:42070");

client.on('open', () => {
    console.log('Connected to server.');
})

client.on('message', (data) => {
    console.log(data.toString());
})

client.on('close', () => {
    console.log('Connection closed.');
})

client.on('error', (err) => {
    console.error(err);
});