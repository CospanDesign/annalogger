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

	var ctx = document.getElementById("main_chart").getContext("2d");
	window.myLine = new Chart(ctx).Line(line_data, {
		responsive: true
	});


	var update_data = function(old_data){
		var labels = old_data["labels"];
		var data = old_data["datasets"][0]["data"];

		labels.shift();
		count++;
		labels.push(count.toString());

		var new_data = data[9] + randomScalingFactor();
		
		data.push(new_data);
		data.shift();
	};

	var options_animation = {
		scaleOverride		: true,
		scaleSteps			: 10,
		scaleStepWidth	: 10,
		scaleStartValue	:	0
	}
	var options_no_animation = {
		animation				:	false,
		scaleOverride		:	true,
		scaleSteps			:	20,
		scaleStepWidth	:	10,
		scaleStartValue	:	0
	}

	var options_no_animation = {animation : false};


	setntervale(function(){
		update_data(line_data);
		windows.myLine(line_data, options_no_animation),
		200
	});,

})	
