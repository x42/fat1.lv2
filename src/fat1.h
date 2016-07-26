#define FAT1_URI "http://gareus.org/oss/lv2/fat1"

typedef enum {
	FAT_MIDI_IN = 0,
	FAT_INPUT, FAT_OUTPUT,
	FAT_MODE,
	FAT_MCHN,
	FAT_TUNE,
	FAT_BIAS,
	FAT_FILT,
	FAT_CORR,
	FAT_OFFS, // 9

	FAT_NOTE = 10, // + 12 notes
	FAT_MASK = 22,
	FAT_NSET,
	FAT_ERRR,
	FAT_LTNC,
	FAT_LAST
} PortIndex;
