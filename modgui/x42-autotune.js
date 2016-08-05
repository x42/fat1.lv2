function (event) {

	/* top-level entry, called from mod-ui */
	if (event.type == 'start') {
		var ports = event.ports;
		for (var p in ports) {
			if (ports[p].symbol == 'mode') {
				var pk = event.icon.find ('[mod-role=piano-keyboard]');
				if (event.value == 1) {
					pk.css('display', 'none');
				} else {
					pk.css('display', 'block');
				}
			}
		}
	}
	else if (event.type == 'change') {
		if (event.symbol == 'mode') {
			var pk = event.icon.find ('[mod-role=piano-keyboard]');
			if (event.value == 1) {
				pk.css('display', 'none');
			} else {
				pk.css('display', 'block');
			}
		}
	}
}
