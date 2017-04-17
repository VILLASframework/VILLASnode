// global variables
var api;
var connection;
var timer;

var nodes = [];
var currentNode;

var paused = false;
var sequence = 0;

var plotData = [];
var plotOptions = {
	xaxis: {
		mode: 'time'
	},
	legend: {
		show: true
	}
};

var updateRate = 25;
var redrawPlot = true;

var xDelta  = 5000;
var xPast   = xDelta * 0.9;
var xFuture = xDelta * 0.1;

$(document).ready(function() {
	api = new Api('v1', apiConnected);
	
	$('#play').click(function(e, ui) {
		connection = wsConnect(currentNode);
		paused = false;
	});
	
	$('#pause').click(function(e, ui) {
		connection.close();
		paused = true;
	});
	
	$('#timespan').slider({
		min : 1000,
		max : 10000,
		value : xDelta,
		slide : function(e, ui) {
			updatePlotWindow(ui.value);
		}
	});
	
	$('#updaterate').slider({
		min : 1,
		max : 50,
		value : updateRate,
		slide : function(e, ui) {
			clearInterval(timer);
			timer = setInterval(updatePlot, 1000.0 / updateRate);
			updateRate = ui.value;
		}
	});

	$('.inputs #slider').slider({
		min : 0,
		max : 100,
		slide : sendData
	});
	
	$('.inputs-checkboxes input').checkboxradio()
		.each(function(idx, elm) {
			$(elm).change(sendData);
		});
	
	timer = setInterval(updatePlot, 1000.0 / updateRate);
});

$(window).on('beforeunload', function() {
	connection.close();
	api.close();
});

function sendData()
{
	var slider = $('.inputs #slider');
	var checkboxes = $('.inputs-checkboxes  input');

	var data = [ $(slider).slider('value'), 0 ];

	for (var i = 0; i < checkboxes.length; i++)
		data[1] += (checkboxes[i].checked ? 1 : 0) << i;

	var msg = new Msg({
		timestamp : Date.now(),
		sequence  : sequence++,
		id        : currentNode.id,
		data      : data
	});

	console.log('Sending message', msg);

	connection.send(msg.toArrayBuffer());
}

function apiConnected()
{
	api.request('nodes', {},
		function(response) {
			nodes = response;
			
			console.log("Found " + nodes.length + " nodes:", nodes);

			for (var i = 0; i < nodes.length; i++)
				if (nodes[i].name == getParameterByName('node'))
					currentNode = nodes[i];

			if (currentNode === undefined)
				currentNode = nodes[0];

			if (currentNode !== undefined) {
				updateNodeList();

				connection = wsConnect(currentNode);
			}
		}
	);
}

function updateNodeList()
{
	$('.node-selector').empty();

	nodes.forEach(function(node, index) {
		if (node.type == 'websocket') {
			$('.node-selector').append(
				$('<button>')
					.addClass(node.name == currentNode.name ? 'ui-state-active' : '')
					.text(node.description ? node.description : node.name)
					.click(function() {
						var url = node.name;
						window.location = '?node=' + node.name;
					})
			);
		}
	});
	
	$('.node-selector').buttonset();
}

function updatePlotWindow(delta)
{
	xDelta  = delta
	xPast   = xDelta * 0.9;
	xFuture = xDelta * 0.1;
}

function updatePlot()
{
	var data = [];
	
	if (!redrawPlot)
		return;

	// add data to arrays
	for (var i = 0; i < plotData.length; i++) {
		var seriesOptions = nodes
		
		data[i] = {
			data : plotData[i],
			shadowSize : 0,
			label : 'Index ' + String(i),
			lines : {
				lineWidth: 2
			}
		}
		
		if (currentNode.series !== undefined && currentNode.series[i] !== undefined)
			$.extend(true, data[i], currentNode.series[i]);
	}
	
	var options = {
		xaxis: {
			min: Date.now() - xPast,
			max: Date.now() + xFuture
		},
		grid: {
			markings: [
				{ xaxis: { from: Date.now(), to: Date.now() }, color: '#ff0000' }
			]
		}
	}

	/* update plot */
	$.plot('.plot-container div', data, $.extend(true, options, plotOptions));
	
	redrawPlot = false;
}

function wsConnect(node)
{
	var url = wsUrl('');
	var conn = new WebSocket(url, 'live');

	conn.binaryType = 'arraybuffer';
	
	conn.onopen = function() {
		$('#status')
			.text('Connected')
			.css('color', 'green');
			
		console.log('WebSocket connection established');
	};

	conn.onclose = function(error) {
		console.log('WebSocket connection closed', error);
		
		$('#status')
			.text('Disconnected (' + error.reason + ')')
			.css('color', 'red');

		/* Try connect if close reason was CLOSE_NORMAL */
		if (error.code == 1000 || error.code == 1001) {
			setTimeout(function() {
				wsConnect(currentNode);
			}, 1000);
		}
	};

	conn.onerror = function(error) {
		console.log('WebSocket connection error', error);
		
		$('#status').text('Status: Error: ' + error.message);
	};

	conn.onmessage = function(e) {
		var msgs = Msg.fromArrayBufferVector(e.data);
		
		console.log('Received ' + msgs.length + ' messages with ' + msgs[0].data.length + ' values from id ' + msgs[0].id + ' with timestamp ' + msgs[0].timestamp);

		for (var j = 0; j < plotData.length; j++) {
			// remove old
			while (plotData[j].length > 0 && plotData[j][0][0] < (Date.now() - xPast))
				plotData[j].shift()
		}

		for (var j = 0; j < msgs.length; j++) {
			var msg = msgs[j];
			
			if (msg.id != currentNode.id)
				continue;

			// add empty arrays for data series
			while (plotData.length < msg.length)
				plotData.push([]);

			// add data to arrays
			for (var i = 0; i < msg.length; i++)
				plotData[i].push([msg.timestamp, msg.data[i]]);
		}
		
		redrawPlot = true;
	};
	
	return conn;
};

/* Helpers */
function wsUrl(endpoint)
{
	var l = window.location;
	var url = '';

	if (l.protocol === 'https:')
		url += 'wss://';
	else
		url += 'ws://';
	
	url += l.hostname;
	
	if ((l.port) && (l.port != 80) && (l.port != 443))
		url += ':'+ l.port;

	url += '/' + endpoint;
	
	return url;
}

/* Some helpers */
