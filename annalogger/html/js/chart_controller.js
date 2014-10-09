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

})
