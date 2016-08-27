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
		if (data.mode == 2 || (data.mode == 0 && data.nmask == 0)) {
			/* manual mode -- control input applies */
			/* auto mode -- control applies IFF no midi notes are set */
			for (k = 0; k < 12; ++k) {
				mask_key (k, 0);
			}
			return;
		}
		/* midi rulez */
		for (k = 0; k < 12; ++k) {
			mask_key (k, (Math.floor (data.nmask) & (1 << k)) == 0 ? -1 : 1);
		}
	}

	function update_note_set (data) {
		for (k = 0; k < 12; ++k) {
			set_key (k, Math.floor (data.nset) & (1 << k));
		}
	}

	function update_pitcherror (err) {
		var bar = event.icon.find ('[mod-role=pitch-error]');
		if (Math.abs (err) > 0.33) {
			bar.addClass ('bad');
		} else {
			bar.removeClass ('bad');
		}
		var pos = 63 + 64 * err;
		bar.css ('left', pos + 'px'); /* CSS or style? XXX */
	}

	/* top-level entry, called from mod-ui */
	if (event.type == 'start') {
		var ports = event.ports;
		var data = {};
		data.mode = 0;
		data.nmask = 0;
		data.nset = 0;
		for (var p in ports) {
			if (event.symbol == 'error') {
				update_pitcherror (event.value);
			}
			else if (ports[p].symbol == 'mode') {
				data.mode = event.value;
			}
			else if (ports[p].symbol == 'nmask') {
				data.nmask = event.value;
			}
			else if (ports[p].symbol == 'nset') {
				data.nset = event.value;
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
		if (event.symbol == 'error') {
			update_pitcherror (event.value);
			return;
		}
		else if (event.symbol == 'mode') {
			data.mode = event.value;
			update_midi_mask (data);
		}
		else if (event.symbol == 'nmask') {
			data.nmask = event.value;
			update_midi_mask (data);
		}
		else if (event.symbol == 'nset') {
			data.nset = event.value;
			update_note_set (data);
		}
		ds.data ('persist', data);
	}
}
