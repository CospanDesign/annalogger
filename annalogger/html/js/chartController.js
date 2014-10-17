$(document).ready(function() {
	//Run as soon sa HTML page is ready

	/*
	var mchart = document.getElementById("main_chart").getContext('2d');
	var newmchart = new Chart(mchart).Line(line_data);

	*/
	var randomScalingFactor = function(){ return Math.round(Math.random()*100)};
	var time_pos = 10;
	var line_data = {
		labels : ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10"],
		datasets : [
			{
				fillColor 						: "rgba(172, 194, 132, 0.4)",
				strokeColor 					: "rgba(220,220,220,1)",
				pointColor 						: "rgba(220,220,220,1)",
				pointStrokeColor 			: "rgba(255, 255, 255, 1)",
				pointHighlightStroke 	: "rgba(220,220,220,1)",
				data 									: [	0,
																	0,
																	0,
																	0,
																	0,
																	0,
																	0,
																	0,
																	0,
																	0
																	//randomScalingFactor()
																]
			}
		]
	}

	var options_animation = {
		scaleOverride		: true,
		scaleSteps			: 10,
		scaleStepWidth	: 10,
    responsive      : true,
		scaleStartValue	:	0
	}
	var options_no_animation = {
		scaleOverride		:	true,
		animation				:	false,
		scaleSteps			: 10,
		scaleStepWidth	:	10,
    responsive      : true,
    bezierCurve     : false,
		scaleStartValue	:	0
	}


  
  //var date = Date().now().toString()

	var ctx = document.getElementById("main_chart").getContext("2d");

  ctx.canvas.width = window.innerWidth;
  ctx.canvas.height = 200;
	var line_chart = new Chart(ctx);
  var lchart = line_chart.Line(line_data, options_no_animation);

	var update_data = function(old_data){
		var labels = old_data["labels"];
		var data = old_data["datasets"][0]["data"];

		labels.shift();
		time_pos++;
		labels.push(time_pos.toString());
    //date = Date().now().toString()
		//labels.push(date);

		var new_data = randomScalingFactor();

		data.push(new_data);
		data.shift();
	};


	setInterval(function(){

		update_data(line_data);

    for (var i = 0; i < line_data["labels"].length; i++){
      lchart.datasets[0].points[i].value = line_data["datasets"][0]["data"][i];
    }
    lchart.labels = line_data["labels"];
    lchart.update();
		//line_chart.Line(line_data, options_no_animation)
    //line_chart.Line.update();
	  }, 200
  );




  //Web Socket Connection
  function StartWS(url) {
		sl_ws = new WebSocket(url);
		sl_ws.binaryType = 'arraybuffer';
  
		sl_ws.onopen = function() {
		alert("WebSocket Connected");
		// TBD - Send Start Command on Socket
		};
  
		sl_ws.onerror = function() {
		    alert("WebSocket Error");
		};
  
		sl_ws.onmessage = function(event) {
		   //alert("WebSocket Data Received");
		   var wsRecvMsg = event.data;
		   var cameraStream = new Uint8Array(wsRecvMsg);
		   var camerascreen = document.getElementById('camerascreen');
		   camerascreen.src = 'data:image/jpg;base64,'+encode(cameraStream);
			//camerascreen.src = encode(cameraStream);
		   //camerascreen.src = images/camera2.jpg;
		   //alert ('Server Says:  '+wsRecvMsg+' ');
		   //Receive and Handle Data
		   
		};
  
		sl_ws.onclose = function() {
			alert("WebSocket Closed");
		};
	
  }

  function StopWS() {
		//Close Websocket
		sl_ws.close();
		alert("WebSocket Closed");
  }

  function SendMessage() {
		//Send Data to host
		var data = $('#wsMessage').val();
		sl_ws.send(data);
  }


  //var connect_button = document.getElementById("wsConnectButton");
  //var disconnect_button = document.getElementById("wsDisconnectButton");

  if (window.location.hostname == ""){
    document.getElementById("wsURL").value = "ws://127.0.0.1/?encoding=text";
  }
  else {
    document.getElementById("wsURL").value = "ws://" + window.location.hostname + "/?encoding=text";
  }
  //Connect click events
  document.getElementById("wsConnectButton").onclick = function() {
    alert ("Connect Clicked");
    var url = document.getElementById("wsURL").value;
    StartWS(url);
  }

  document.getElementById("wsDisconnectButton").onclick = function() {
    alert ("Disconnect Clicked");
    StopWS();
  }

})
