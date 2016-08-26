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

	function update_midi_mask (data) {
		if (data['mode'] == 2) {
			/* manual mode -- control input applies */
			for (k = 0; k < 12; ++k) {
				mask_key (k, 0);
			}
			return;
		}
		if (data['mode'] == 0 && data['mask'] == 0) {
			/* auto mode -- control applies IFF no midi notes are set */
			for (k = 0; k < 12; ++k) {
				mask_key (k, 0);
			}
			return;
		}
		/* midi rulez */
		for (k = 0; k < 12; ++k) {
			mask_key (k, (data.mask & (1 << k)) != 0 ? -1 : 1);
		}
	}

	function update_note_set (data) {
		for (k = 0; k < 12; ++k) {
			set_key (k, data.set & (1 << k));
		}
	}

	/* top-level entry, called from mod-ui */
	if (event.type == 'start') {
		var ports = event.ports;
		var data = {};
		data.mask = 0;
		data.mode = 0;
		data.set = 0;
		for (var p in ports) {
			if (ports[p].symbol == 'mode') {
				data.mode = event.value;
			}
			else if (ports[p].symbol == 'mask') {
				data.mask = event.value;
			}
			else if (ports[p].symbol == 'set') {
				data.set = event.value;
			}
		}
		update_midi_mask (data);
		update_note_set (data);
		var ds = event.icon.find ('[mod-role=drag-handle]');
		ds.data ('persist', data);
	}
	else if (event.type == 'change') {
		var ds = event.icon.find ('[mod-role=drag-handle]');
		var data = ds.data ('persist');
		if (event.symbol == 'mode') {
			data.mode = event.value;
			update_midi_mask (data);
		}
		else if (event.symbol == 'mask') {
			data.mask = event.value;
			update_midi_mask (data);
		}
		else if (event.symbol == 'set') {
			data.set = event.value;
			update_note_set (data);
		}
		ds.data ('persist', data);
	}
}
