<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
 <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <title>Temperaturer</title>
    <link href="layout.css" rel="stylesheet" type="text/css"></link>
    <!--[if IE]><script language="javascript" type="text/javascript" src="flot/excanvas.min.js"></script><![endif]-->
    <script language="javascript" type="text/javascript" src="flot/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="flot/jquery.flot.js"></script>
    <script language="javascript" type="text/javascript" src="scripts/SunriseSunset.js"></script>
 </head>
    <body>
   

    <?php

$dateTimeZoneStockholm = new DateTimeZone("Europe/Stockholm");
$dateTimeStockholm = new DateTime("now", $dateTimeZoneStockholm);
$GmtOffset=$dateTimeZoneStockholm->getOffset($dateTimeStockholm);


$mysqli = new mysqli("localhost", "username", "password", "database");
$start_time=time()-60*60*24*30;
$result = $mysqli->query("select * from Temp WHERE Time>" . $start_time ." order by Time");





while($row = mysqli_fetch_array($result)) {

 // -6*3600 på tider som är tidigare än 1327960800
 // java använder ms ifrån 1 jan 1970 därav multiplikation med 1000
  $t1[] = array( 1000*($row['Time']+$GmtOffset), $row['t1'] );
  $last_t1=$row['t1'];
  $t2[] = array( 1000*($row['Time']+$GmtOffset), $row['t2'] );
  $last_t2=$row['t2'];
  $t3[] = array( 1000*($row['Time']+$GmtOffset), $row['t3'] );
  $t4[] = array( 1000*($row['Time']+$GmtOffset), $row['t4'] );
}

$result = $mysqli->query("select * from Humidity WHERE Time>" . $start_time ." order by Time");
while($row = mysqli_fetch_array($result)) {
 	
 	if ($row['h1'] < 0) {
      	$row['h1']=$row['h1']+163.84;
    	
    } 
 // -6*3600 på tider som är tidigare än 1327960800
 // java använder ms ifrån 1 jan 1970 därav multiplikation med 1000
  $h1[] = array( 1000*($row['Time']+$GmtOffset), $row['h1'] );
   $last_h1=$row['h1'];
  $h2[] = array( 1000*($row['Time']+$GmtOffset), $row['h2'] );
   $last_h2=$row['h2'];
  $h3[] = array( 1000*($row['Time']+$GmtOffset), abs($row['h3']) );
  $h4[] = array( 1000*($row['Time']+$GmtOffset), abs($row['h4']) );
}


$result = $mysqli->query("select * from Energy WHERE Time>" . $start_time ." order by Time DESC");

$counter=0;
$sum=0;

while($row = mysqli_fetch_array($result)) {

  $sum+=$row['E'];
  $counter++;
  if ($counter==3) $midtime=1000*($row['Time']+$GmtOffset);
  if ($counter==6) {
    $counter = 0;
    $e1[] = array( $midtime , $sum/1000 );
    $sum=0;
  } 
	
 // -6*3600 på tider som är tidigare än 1327960800
 // java använder ms ifrån 1 jan 1970 därav multiplikation med 1000
  // E är wh / 10 min så gånger 6/1000 borde bli kWh/h = kW
}



?>
    
	
	 
		<div style="padding-left:41px;float:left;width:150px;"><h3>Ute <? echo round($last_t1,1) ?> &degC</h3>
		<p>Fuktighet <? echo round($last_h1) ?> %</p></div>
	
		<div style="float:left;width:150px;"><h3>Inne <? echo round($last_t2,1) ?> &degC</h3>
		<p>Fuktighet <? echo round($last_h2) ?> %</p></div>
    	<div style="float:left;width:200px">
    	   		<h3>Sol</h3>
    	   		<a href="http://www.suncalc.net/#/59.088,17.5672,16/2012.10.05/20:18">
    	   		<script type="text/javascript">
    	   		
    	   		
    			var now = new Date();
        		var jarna = new SunriseSunset( now.getFullYear(), now.getMonth()+1, now.getDate(), 59+5/60+17/3600, 17+34/60+2/3600);
        		
        		var sunriseTmp =jarna.sunriseLocalHours(-now.getTimezoneOffset()/60);
        		var sunriseHour = Math.floor(sunriseTmp);
        		sunriseTmp = (sunriseTmp-sunriseHour)*60
        		var sunriseMinute = Math.floor(sunriseTmp);
        		sunriseTmp = (sunriseTmp-sunriseMinute)*60;
        		var sunriseSecond = Math.floor(sunriseTmp);
        		
        		var sunsetTmp =jarna.sunsetLocalHours(-now.getTimezoneOffset()/60);
        		var sunsetHour = Math.floor(sunsetTmp);
        		sunsetTmp = (sunsetTmp-sunsetHour)*60
        		var sunsetMinute = Math.floor(sunsetTmp);
        		sunseteTmp = (sunsetTmp-sunsetMinute)*60;
        		var sunsetSecond = Math.floor(sunsetTmp);
        		
        		
        	    //document.write(sunriseSecond);
        	    if (sunriseSecond >= 30) sunriseMinute++;
        	    if (sunsetSecond >= 30) sunsetMinute++;
        	    
        		document.write('<p>Upp:  ' + sunriseHour + ':' + (sunriseMinute < 10 ? '0' : '') + sunriseMinute + '</>');
        		document.write(' &nbsp;&nbsp;&nbsp;ned:   ' + sunsetHour + ':' + (sunsetMinute < 10 ? '0' : '') + sunsetMinute + '</p>');
        		
        		
    		</script>
    		</a>
    	</div>
    
    <div style="clear:both;height:20px"></div>
    <div id="placeholder" style="width:700px; height:400px;"></div>
    <div style="padding-left:41px;width:700px;float:left"</div>
    <p><button id="month">Senaste månaden</button>
      <button id="week">Senaste veckan</button>
      <button id="day">Senaste dygnet</button></p>
    </div>
    
    <p id="hoverdata"></p>
    <div style="padding-left:41px;width:700px;float:left">
    <p id="choices"></p>
    </div>
<script id="source" language="javascript" type="text/javascript">
var plot;



$(function () {

	var datasets = {
		"t1" : {
		lines: { show: true, width: 1 },
			label : "Temperatur utomhus",
			data: <?php echo json_encode($t1); ?> 
		},
		"t2" : {
			label : "Temperatur innomhus",
			data: <?php echo json_encode($t2); ?> 
		},
		"t4" : {
			label : "Temperatur köksgolv",
			data: <?php echo json_encode($t4); ?> 
		},
		"h1" : {
			label : "Luftfuktighet utomhus",
			data: <?php echo json_encode($h1); ?>,
			yaxis: 2  
		},
		"h2" : {
			label : "Luftfuktighet innomhus",
			data: <?php echo json_encode($h2); ?>,
			yaxis: 2 
		},
		"h4" : {
			label : "Luftfuktighet köksgolv",
			data: <?php echo json_encode($h4); ?>,
			yaxis: 2 
		},
		"e1" : {
		bars: { show: true },
			label : "Energiförbruking",
			data: <?php echo json_encode($e1); ?>,
			yaxis: 3
		},
		
	};
	var enabled = {
		"t1" : true,
		"t2" : false,
		"t4" : false,
		"h1" : true,
		"h2" : false,
		"h4" : false,
		"e1" : true
	};
	var timerange = 60*60*24*7;
	document.getElementById("week").disabled = true;

    var i = 0;
    $.each(datasets, function(key, val) {
        val.color = i;
        ++i;
    });

    // insert checkboxes 
    var choiceContainer = $("#choices");
    $.each(datasets, function(key, val) {
        choiceContainer.append('&nbsp;&nbsp; <input type="checkbox" name="' + key +
                               (enabled[key] == true ? '" checked="checked"' : '" ') + 'id="id' + key + '">' +
                               '<label for="id' + key + '">'
                                + val.label + '</label>');
    });
    choiceContainer.find("input").click(plotAccordingToChoices);

    function plotAccordingToChoices() {
    	var data = [];

    	choiceContainer.find("input:checked").each(function () {
            var key = $(this).attr("name");
            if (key && datasets[key])
                data.push(datasets[key]);
        });

        if (data.length > 0)
           var now = new Date();
           var minTime = now.getTime()-timerange*1000
            $.plot($("#placeholder"), data,
            	{
            		        		
               		xaxes: [ { mode: 'time', min: minTime  } ],
               		yaxes: [
               		{ min: -25, max: 35, tickFormatter: function (v) { return v + " &degC"; }},
               		{ min: 0, max: 100, position: "right", tickFormatter: function (v) { return v + " %"; }},
               		{ min: 0, max: 10, position: "right", tickFormatter: function (v) { return v + " kWh"; }}
               		],
               		legend: { position: 'sw'},
               		grid: { hoverable: true }
               	}
            );
    }

    plotAccordingToChoices();
    
    
 
               
	        

	    
		$("#month").click(function () {
        timerange= 60*60*24*31;
        document.getElementById("day").disabled = false;
        document.getElementById("month").disabled = true;
        document.getElementById("week").disabled = false;
        plotAccordingToChoices();
    });

   
	$("#week").click(function () {
       timerange=60*60*24*7;
        document.getElementById("day").disabled = false;
        document.getElementById("month").disabled = false;
        document.getElementById("week").disabled = true;
        plotAccordingToChoices();
    });
       
    $("#day").click(function () {
       timerange=60*60*24;
        document.getElementById("day").disabled = true;
        document.getElementById("month").disabled = false;
        document.getElementById("week").disabled = false;
        plotAccordingToChoices();     
    });
    
    
	function showTooltip(x, y, contents) {
        $('<div id="tooltip">' + contents + '</div>').css( {
            position: 'absolute',
            display: 'none',
            top: y + 5,
            left: x + 5,
            border: '1px solid #fdd',
            padding: '2px',
            'background-color': '#fee',
            opacity: 0.80
        }).appendTo("body").fadeIn(200);
    }

    var previousPoint = null;
    $("#outside").bind("plothover", function (event, pos, item) {
        $("#x").text(pos.x.toFixed(2));
        $("#y").text(pos.y.toFixed(2));

        if (true) {
            if (item) {
                if (previousPoint != item.dataIndex) {
                    previousPoint = item.dataIndex;
                    
                    $("#tooltip").remove();
                    var x = new Date(item.datapoint[0]),
                        y = item.datapoint[1].toFixed(2);
                    
                    showTooltip(item.pageX, item.pageY,
                                item.series.label + " " + x.getHours() + ":" + x.getMinutes() + " = " + y + " &degC");
                }
            }
            else {
                $("#tooltip").remove();
                previousPoint = null;            
            }
        }
    });

    



});
</script>






 </body>
</html>
