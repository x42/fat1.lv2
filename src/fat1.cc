/* fat1 -- LV2 autotuner
 *
 * Copyright (C) 2016 Robin Gareus <robin@gareus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LV2_1_18_6
#include <lv2/atom/atom.h>
#include <lv2/atom/forge.h>
#include <lv2/core/lv2.h>
#include <lv2/log/logger.h>
#include <lv2/midi/midi.h>
#else
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/log/logger.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#endif

#include "fat1.h"
#include "retuner.h"

static pthread_mutex_t fftw_planner_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned int    instance_count    = 0;

typedef struct {
	const LV2_Atom_Sequence* midiin;

	/* URI mapping */
	LV2_URID midi_MidiEvent;
	LV2_URID atom_Object;
	LV2_URID fat_panic;
	LV2_URID atom_eventTransfer;

	/* LV2 Output */
	LV2_Log_Log*   log;
	LV2_Log_Logger logger;

	/* I/O Ports */
	float* port[FAT_LAST];

	/* internal state */
	LV2AT::Retuner* retuner;

	int   notes[12];
	int   notemask;
	int   midimask;
	int   midichan;
	float pitchbend;
	float latency;

	bool panic_btn;
	bool microtonal;
	bool scales;

	uint32_t noteset_update_interval;
	uint32_t noteset_update_counter;
	int      noteset;
} Fat1;

/* *****************************************************************************
 * helper functions
 */

static void
clear_midimask (Fat1* self)
{
	for (int i = 0; i < 12; ++i) {
		self->notes[i] = 0;
	}
	self->midimask  = 0;
	self->pitchbend = 0;
}

static void
update_midimask (Fat1* self)
{
	self->midimask = 0;
	int b, i;
	for (i = 0, b = 1; i < 12; i++, b <<= 1) {
		if (self->notes[i])
			self->midimask |= b;
	}
}

static int
midi_14bit (const uint8_t* const b)
{
	return (b[1]) | (b[2] << 7);
}

static int
parse_midi (Fat1*                self,
            uint32_t             tme,
            const uint8_t* const msg,
            const uint32_t       size)
{
	if (size != 3) {
		return 0;
	}

	if ((self->midichan < 0) || ((msg[0] & 0x0f) == self->midichan)) {
		const uint32_t n = msg[1];
		switch (msg[0] & 0xf0) {
			case 0xe0:
				self->pitchbend = (midi_14bit (msg) - 8192) / 8192.f;
				break;
			case 0xb0:
				if ((n == 123 || n == 120) && msg[2] == 0) { // midi-panic
					clear_midimask (self);
				}
				break;
			case 0x90:
				if (msg[2] > 0) {
					self->notes[n % 12] += 1;
					return 1;
				}
				// no break -- fallthrough, note-on velocity 0
			case 0x80:
				if (self->notes[n % 12]) {
					self->notes[n % 12] -= 1;
					return 1;
				}
			default:
				break;
		}
	}
	return 0;
}

static int
set_scale (int scale_id) {
	if (scale_id <= 0 || scale_id > 24) {
		/* chromatic */
		return 4095;
	}

	static const bool western[12] = {
		1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1
	};

	if (scale_id > 12) {
		scale_id = 1 + (scale_id + 2) % 12;
	}

	int b, i;
	int notemask = 0;
	for (i = 0, b = 1; i < 12; i++, b <<= 1) {
		int s = (i + 13 - scale_id) % 12;
		if (western[s]) {
			notemask |= b;
		}
	}

	return notemask;
}

/* *****************************************************************************
 * LV2 Plugin
 */

static LV2_Handle
instantiate (const LV2_Descriptor*     descriptor,
             double                    rate,
             const char*               bundle_path,
             const LV2_Feature* const* features)
{
	Fat1*         self = (Fat1*)calloc (1, sizeof (Fat1));
	LV2_URID_Map* map  = NULL;

	if (0 == strcmp (descriptor->URI, FAT1_URI)) {
		self->microtonal = false;
		self->scales     = false;
	} else if (0 == strcmp (descriptor->URI, FAT1_URI "#microtonal")) {
		self->microtonal = true;
		self->scales     = false;
	} else if (0 == strcmp (descriptor->URI, FAT1_URI "#scales")) {
		self->microtonal = false;
		self->scales     = true;
	} else {
		free (self);
		return 0;
	}

	int i;
	for (i = 0; features[i]; ++i) {
		if (!strcmp (features[i]->URI, LV2_URID__map)) {
			map = (LV2_URID_Map*)features[i]->data;
		} else if (!strcmp (features[i]->URI, LV2_LOG__log)) {
			self->log = (LV2_Log_Log*)features[i]->data;
		}
	}

	lv2_log_logger_init (&self->logger, map, self->log);

	if (!map) {
		lv2_log_error (&self->logger, "Fat1.lv2 error: Host does not support urid:map\n");
		free (self);
		return NULL;
	}

	pthread_mutex_lock (&fftw_planner_lock);
	self->retuner = new LV2AT::Retuner (rate);
	++instance_count;
	pthread_mutex_unlock (&fftw_planner_lock);

	self->notemask = 0xfff;
	self->midichan = -1;
	clear_midimask (self);

	self->midi_MidiEvent     = map->map (map->handle, LV2_MIDI__MidiEvent);
	self->atom_Object        = map->map (map->handle, LV2_ATOM__Object);
	self->fat_panic          = map->map (map->handle, FAT1_URI "#panic");
	self->atom_eventTransfer = map->map (map->handle, LV2_ATOM__eventTransfer);

	// compare Retuner c'tor
	if (rate < 64000) {
		self->latency = 1024;
	} else if (rate < 128000) {
		self->latency = 2048;
	} else {
		self->latency = 4096;
	}
	self->noteset_update_interval = rate / 20;
	self->noteset_update_counter  = self->noteset_update_interval;
	return (LV2_Handle)self;
}

static void
connect_port (LV2_Handle instance,
              uint32_t   port,
              void*      data)
{
	Fat1* self = (Fat1*)instance;

	if (port == FAT_MIDI_IN) {
		self->midiin = (const LV2_Atom_Sequence*)data;
	} else if (port < FAT_LAST) {
		self->port[port] = (float*)data;
	}
}

static void
connect_port_scales (LV2_Handle instance,
                     uint32_t   port,
                     void*      data)
{
	Fat1* self = (Fat1*)instance;

	if (port == FAT_MIDI_IN) {
		self->midiin = (const LV2_Atom_Sequence*)data;
	} else if (port <= FAT_NOTE) {
		self->port[port] = (float*)data;
	} else {
		switch (port) {
			case 13:
				self->port[FAT_MASK] = (float*)data;
				break;
			case 14:
				self->port[FAT_NSET] = (float*)data;
				break;
			case 15:
				self->port[FAT_BEND] = (float*)data;
				break;
			case 16:
				self->port[FAT_ERRR] = (float*)data;
				break;
			case 17:
				self->port[FAT_LTNC] = (float*)data;
				break;
			default:
				break;
		}
	}
}

static void
run (LV2_Handle instance, uint32_t n_samples)
{
	Fat1* self = (Fat1*)instance;

	self->retuner->set_fastmode (*self->port[FAT_FAST]);

	if (*self->port[FAT_FAST]) {
		*self->port[FAT_LTNC] = self->latency / 8;
	} else {
		*self->port[FAT_LTNC] = self->latency;
	}

	if (self->retuner->upsample ()) {
		/* add 32 samples for resampler's latency */
		*self->port[FAT_LTNC] += 32;
	}

	if (!self->midiin || n_samples == 0) {
		return;
	}

	/* set mode and midi-channel */
	const int mchn = rint (*self->port[FAT_MCHN]);
	if (mchn <= 0 || mchn > 16) {
		self->midichan = -1;
	} else {
		self->midichan = mchn - 1;
	}

	int mode = rint (*self->port[FAT_MODE]);
	if (mode < 0)
		mode = 0;
	if (mode > 2)
		mode = 2;

	if (mode == 2) {
		clear_midimask (self);
	}

	/* Process incoming midi events */
	bool            update_midi = false;
	LV2_Atom_Event* ev          = lv2_atom_sequence_begin (&(self->midiin)->body);
	while (!lv2_atom_sequence_is_end (&(self->midiin)->body, (self->midiin)->atom.size, ev)) {
		if (ev->body.type == self->midi_MidiEvent) {
			update_midi |= parse_midi (self, ev->time.frames, (uint8_t*)(ev + 1), ev->body.size);

		} else if (ev->body.type == self->atom_Object) {
			const LV2_Atom_Object* obj = (LV2_Atom_Object*)&ev->body;
			if (obj->body.otype == self->fat_panic) {
				clear_midimask (self);
			}
		}
		ev = lv2_atom_sequence_next (ev);
	}
	if (update_midi) {
		update_midimask (self);
	}

	/* prepare key-range midi/fixed */
	if (mode != 1) {
		if (self->scales) {
			self->notemask = set_scale (rint (*self->port[FAT_NOTE]));
		} else {
			self->notemask = 0;
			for (int i = 0; i < 12; ++i) {
				if (*self->port[FAT_NOTE + i] > 0) {
					self->notemask |= 1 << i;
				}
			}
		}
	}

	int notes;
	switch (mode) {
		case 1:
			notes = self->midimask;
			break;
		case 2:
			notes           = self->notemask;
			self->pitchbend = 0.f;
			break;
		default:
			notes = self->midimask ? self->midimask : self->notemask;
	}

	/* push config to retuner */
	self->retuner->set_refpitch (*self->port[FAT_TUNE]);
	self->retuner->set_notebias (*self->port[FAT_BIAS]);
	self->retuner->set_corrfilt (*self->port[FAT_FILT]);
	self->retuner->set_corrgain (*self->port[FAT_CORR]);
	self->retuner->set_corroffs (*self->port[FAT_OFFS] + *self->port[FAT_PBST] * self->pitchbend);
	self->retuner->set_notemask (notes);

	if (self->microtonal) {
		for (int i = 0; i < 12; ++i) {
			self->retuner->set_notescale (i, *self->port[FAT_SCALE + i]);
		}
	}

	/* process */
	self->retuner->process (n_samples, self->port[FAT_INPUT], self->port[FAT_OUTPUT]);

	self->noteset_update_counter += n_samples;

	if (self->noteset_update_counter > self->noteset_update_interval) {
		self->noteset                = self->retuner->get_noteset ();
		self->noteset_update_counter = 0;
	}

	/* report */
	*self->port[FAT_MASK] = notes;
	*self->port[FAT_NSET] = self->noteset;
	*self->port[FAT_ERRR] = self->retuner->get_error ();
	*self->port[FAT_BEND] = self->pitchbend;
}

static void
cleanup (LV2_Handle instance)
{
	Fat1* self = (Fat1*)instance;
	pthread_mutex_lock (&fftw_planner_lock);
	delete self->retuner;
	if (instance_count > 0) {
		--instance_count;
	}
#ifdef WITH_STATIC_FFTW_CLEANUP
	/* use this only when statically linking to a local fftw!
	 *
	 * "After calling fftw_cleanup, all existing plans become undefined,
	 *  and you should not attempt to execute them nor to destroy them."
	 * [http://www.fftw.org/fftw3_doc/Using-Plans.html]
	 *
	 * If libfftwf is shared with other plugins or the host this can
	 * cause undefined behavior.
	 */
	if (instance_count == 0) {
		fftwf_cleanup ();
	}
#endif
	pthread_mutex_unlock (&fftw_planner_lock);
	free (instance);
}

static void
activate (LV2_Handle instance)
{
	Fat1* self = (Fat1*)instance;
	clear_midimask (self);
}

const void*
extension_data (const char* uri)
{
	return NULL;
}

static const LV2_Descriptor descriptor = {
	FAT1_URI,
	instantiate,
	connect_port,
	activate,
	run,
	NULL,
	cleanup,
	extension_data
};

static const LV2_Descriptor descriptor_microtonal = {
	FAT1_URI "#microtonal",
	instantiate,
	connect_port,
	activate,
	run,
	NULL,
	cleanup,
	extension_data
};

static const LV2_Descriptor descriptor_scales = {
	FAT1_URI "#scales",
	instantiate,
	connect_port_scales,
	activate,
	run,
	NULL,
	cleanup,
	extension_data
};

#undef LV2_SYMBOL_EXPORT
#ifdef _WIN32
#  define LV2_SYMBOL_EXPORT __declspec(dllexport)
#else
#  define LV2_SYMBOL_EXPORT __attribute__ ((visibility ("default")))
#endif
LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor (uint32_t index)
{
	switch (index) {
		case 0:
			return &descriptor;
		case 1:
			return &descriptor_microtonal;
		case 2:
			return &descriptor_scales;
		default:
			return NULL;
	}
}
