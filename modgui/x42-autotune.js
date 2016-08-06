function (event) {

	/* mask midi selected notes */
	function mask_key (id, midimode) {
		var sym = id;
		if (id < 10) { sym = "0" + id; }
		var key = event.icon.find ('[mod-port-symbol=m' + sym +']');
		if (midimode > 0) {
			key.removeClass ('midioff');
			key.addClass ('midion');
		} else if (midimode < 0) {
			key.removeClass ('midion');
			key.addClass ('midioff');
		} else {
			/* note is under user control */
			key.removeClass ('midion');
			key.removeClass ('midioff');
		}
	}

	/* highlight detected note(s) */
	function set_key (id, on) {
		var sym = id;
		if (id < 10) { sym = "0" + id; }
		var key = event.icon.find ('[mod-port-symbol=m' + sym +']');
		if (on) {
			key.addClass ('active');
		} else {
			key.removeClass ('active');
		}
	}

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
			// prepare for output params -- MIDI set keys
			if (event.value == 2) {
				for (k = 0; k < 12; ++k) {
					mask_key (k, 0);
				}
			}
		}
	}
}
