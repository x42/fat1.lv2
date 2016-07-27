// generated by lv2ttl2c from
// http://gareus.org/oss/lv2/fat1

extern const LV2_Descriptor* lv2_descriptor(uint32_t index);
extern const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index);

static const RtkLv2Description _plugin = {
	&lv2_descriptor,
	&lv2ui_descriptor
	, 0 // uint32_t dsp_descriptor_id
	, 0 // uint32_t gui_descriptor_id
	, "Autotuner" // const char *plugin_human_id
	, (const struct LV2Port[26])
	{
		{ "midiin", MIDI_IN, nan, nan, nan, "MIDI In"},
		{ "in", AUDIO_IN, nan, nan, nan, "Input"},
		{ "out", AUDIO_OUT, nan, nan, nan, "Output"},
		{ "mode", CONTROL_IN, 0.000000, 0.000000, 2.000000, "Mode"},
		{ "channelf", CONTROL_IN, 0.000000, 0.000000, 16.000000, "MIDI Channel (1..16) on which the filter is active; 0: any)."},
		{ "tuning", CONTROL_IN, 440.000000, 400.000000, 480.000000, "Tuning"},
		{ "bias", CONTROL_IN, 0.500000, 0.000000, 1.000000, "Bias"},
		{ "filter", CONTROL_IN, 0.100000, 0.020000, 0.500000, "Filter"},
		{ "corr", CONTROL_IN, 1.000000, 0.000000, 1.000000, "Correlation"},
		{ "offset", CONTROL_IN, 0.000000, -2.000000, 2.000000, "Offset"},
		{ "m00", CONTROL_IN, 1.000000, 0.000000, 1.000000, "C"},
		{ "m01", CONTROL_IN, 1.000000, 0.000000, 1.000000, "C#"},
		{ "m02", CONTROL_IN, 1.000000, 0.000000, 1.000000, "D"},
		{ "m03", CONTROL_IN, 1.000000, 0.000000, 1.000000, "D#"},
		{ "m04", CONTROL_IN, 1.000000, 0.000000, 1.000000, "E"},
		{ "m05", CONTROL_IN, 1.000000, 0.000000, 1.000000, "F"},
		{ "m06", CONTROL_IN, 1.000000, 0.000000, 1.000000, "F#"},
		{ "m07", CONTROL_IN, 1.000000, 0.000000, 1.000000, "G"},
		{ "m08", CONTROL_IN, 1.000000, 0.000000, 1.000000, "G#"},
		{ "m09", CONTROL_IN, 1.000000, 0.000000, 1.000000, "A"},
		{ "m10", CONTROL_IN, 1.000000, 0.000000, 1.000000, "A#"},
		{ "m11", CONTROL_IN, 1.000000, 0.000000, 1.000000, "B"},
		{ "nmask", CONTROL_OUT, nan, 0.000000, 4096.000000, "Note Mask"},
		{ "nset", CONTROL_OUT, nan, 0.000000, 4096.000000, "Note Set"},
		{ "error", CONTROL_OUT, nan, -1.000000, 1.000000, "Pitch Error"},
		{ "latency", CONTROL_OUT, nan, 0.000000, 4096.000000, "latency"},
	}
	, 26 // uint32_t nports_total
	, 1 // uint32_t nports_audio_in
	, 1 // uint32_t nports_audio_out
	, 1 // uint32_t nports_midi_in
	, 0 // uint32_t nports_midi_out
	, 0 // uint32_t nports_atom_in
	, 0 // uint32_t nports_atom_out
	, 23 // uint32_t nports_ctrl
	, 19 // uint32_t nports_ctrl_in
	, 4 // uint32_t nports_ctrl_out
	, 8192 // uint32_t min_atom_bufsiz
	, false // bool send_time_info
};
