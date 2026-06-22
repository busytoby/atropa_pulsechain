const net = require('net');

console.log("Connecting to Auncient Bridge on port 18080...");
const client = net.createConnection({ port: 18080 }, () => {
    console.log("Connected! Sending scroll and key events...");
    
    // Send mouse scroll event
    client.write("MOUSE_SCROLL 0 2\n");
    // Send key down for navigation key (ArrowDown = 108)
    client.write("KEY_DOWN 108\n");
    // Send key up for navigation key
    client.write("KEY_UP 108\n");
    
    // Close after a brief delay
    setTimeout(() => {
        client.end();
        console.log("Finished sending test events.");
    }, 1000);
});

client.on('error', (err) => {
    console.error("Connection error:", err.message);
});
