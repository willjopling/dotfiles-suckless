#include <stdio.h>
#include <alsa/asoundlib.h>
#include <err.h>

#include "../util.h"
#include "volume.h"

#define ICONsn                          COL1 "  " COL0
#define ICONsm                          COL2 "  " COL0
#define ICONhn                          COL1 "" COL0
#define ICONhm                          COL2 "" COL0


#define DEVNAME                         "default"
#define MIXNAME                         "Master"


#define PAVUCONTROL                     (char *[]){ "pavucontrol-qt", NULL }
#define NORMALIZEVOLUME                 (char *[]){ SCRIPT("pulse_normalize.sh"), NULL }
#define TOGGLEMUTE                      (char *[]){ "amixer", "set", MIXNAME, "toggle", NULL }

size_t
volumeu(char *str, int sigval)
{
        static char *icons[] = { ICONsn, ICONsm, ICONhn, ICONhm };
        static char *devname = DEVNAME, *mixname = MIXNAME;
        char icon[32];
        long min = 0, max = 0, volume = -1;

        int err, enabled = 0;

	snd_mixer_t *mixer = NULL;
	snd_mixer_selem_id_t *mixid = NULL;
	snd_mixer_elem_t *elem = NULL;

        strcpy(icon, icons[1]);

	if((err = snd_mixer_open(&mixer, 0))) {
		warn("snd_mixer_open: %d", err);
		return 1;
	}

	if((err = snd_mixer_attach(mixer, devname))) {
		warn("snd_mixer_attach(mixer, \"%s\"): %d", devname, err);
		goto cleanup;
	}

	if((err = snd_mixer_selem_register(mixer, NULL, NULL))) {
		warn("snd_mixer_selem_register(mixer, NULL, NULL): %d", err);
		goto cleanup;
	}

	if((err = snd_mixer_load((mixer)))) {
		warn("snd_mixer_load(mixer): %d", err);
		goto cleanup;
	}

	snd_mixer_selem_id_alloca(&mixid);
	snd_mixer_selem_id_set_name(mixid, mixname);
	snd_mixer_selem_id_set_index(mixid, 0);

	elem = snd_mixer_find_selem(mixer, mixid);
	if(!elem) {
		warn("snd_mixer_find_selem(mixer, \"%s\") == NULL", mixname);
		goto cleanup;
	}

	if((err = snd_mixer_selem_get_playback_volume_range(elem, &min, &max))) {
		warn("snd_mixer_selem_get_playback_volume_range(): %d", err);
		goto cleanup;
	}

	if((err = snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &volume))) {
		warn("snd_mixer_selem_get_playback_volume(): %d", err);
	}

	// The switch is ON or OFF (1 or 0) if channel is muted or not.
	if((err = snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_MONO, &enabled))) {
		warn("snd_mixer_selem_get_playback_switch(): %d", err);
	}


cleanup:
	snd_mixer_free(mixer);
	snd_mixer_detach(mixer, devname);

        if(enabled) {
                strcpy(icon, icons[0]);
        }

        return SPRINTF(str, "%s%ld", icon, volume == -1 ? -1 : (volume - min)* 100/(max-min));
}
void
volumec(int button)
{
        switch(button) {
                case 1:
                        cspawn(TOGGLEMUTE);
                        break;
                case 2:
                        cspawn(NORMALIZEVOLUME);
                        break;
                case 3:
                        cspawn(PAVUCONTROL);
                        break;
        }
}
