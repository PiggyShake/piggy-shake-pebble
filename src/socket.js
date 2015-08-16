var reconnectInterval = 1000 * 3; // milliseconds
var WS;
var connect = function(){
    WS = new WebSocket('ws://piggy-ubuntu.cloudapp.net:8080');
    WS.onopen = function() {
        console.log('socket open');
      WS.send('{"groupID": "' + "12" + '", "devID": "' + Pebble.getWatchToken() + '", "shake": "false"}');
    };
    WS.onerror = function() {
        console.log('socket error');
//         setTimeout(connect, reconnectInterval);
    };
    WS.onclose = function() {
        console.log('socket close');
        setTimeout(connect, reconnectInterval);
    };
    WS.onmessage = function (event) {
      // Shake here
      // Send to Pebble
      console.log("Received a shake from the server.");
      Pebble.sendAppMessage({"shake": true});
    };
};
connect();


// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    WS.send('{"groupID": "' + "12" + '", "devID": "' + Pebble.getWatchToken() + '", "shake": "true"}');
  }                     
);