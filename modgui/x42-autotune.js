function (event) {

	/* top-level entry, called from mod-ui */
	if (event.type == 'start') {
		var pk = event.icon.find ('[mod-role=piano-keyboard]');

		var ports = event.ports;
		for (var p in ports) {
			// ports[p].symbol, ports[p].value;
		}
	}
	else if (event.type == 'change') {
		var pk = event.icon.find ('[mod-role=piano-keyboard]');
		// event.symbol, event.value
	}
}
