// Configuration settings
if (localStorage.getItem("username") === null) {
  localStorage.setItem("username", "unknown");
}
if (localStorage.getItem("groupID") === null) {
  localStorage.setItem("groupID", "Default");
}

var reconnectInterval = 1000 * 3; // milliseconds
var WS;
var connect = function(){
    WS = new WebSocket('ws://piggy-ubuntu.cloudapp.net:8080');
    WS.onopen = function() {
        console.log('socket open');
      WS.send('{"groupID": "' + localStorage.getItem("groupID") + '", "devID": "' + Pebble.getWatchToken() + '", "username": "' + localStorage.getItem("username") + '", "shake": "false"}');
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

// Listen for configuration menu launch
Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('https://piggyshake.com/config.html#username=' + encodeURI(localStorage.getItem("username")) + '&channel=' + encodeURI(localStorage.getItem("groupID")));
});

// Listen for configuration menu close
Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  var config_data = JSON.parse(decodeURIComponent(e.response));
  console.log('Config window returned: ', JSON.stringify(config_data));
  
  localStorage.setItem("username", config_data.username);
  localStorage.setItem("groupID", config_data.channel);
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    WS.send('{"groupID": "' + localStorage.getItem("groupID") + '", "devID": "' + Pebble.getWatchToken() + '", "username": "' + localStorage.getItem("username") + '", "shake": "true"}');
  }                     
);