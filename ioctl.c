/*
 * Copyright (c) 2013-2014 Adrian Knoth (Google Inc.)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

/* compile with gcc -DCONFIG_SND_MADIFX_BROKEN -o madifx_testtool ioctl.c -lasound */
#include <stdio.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include <stdint.h>
#include "madifx.h"

static char *texts_madifx_clock_source[] = {
	"Internal",
	"Word Clock",
	"AES In",
	"MADI 1 In",
	"MADI 2 In", 
	"MADI 3 In",
	"Sync-In"
};

static char *rxformat[] = { "64", "56", "32", "28", "16", "14", "No lock" };

static char *synctext[] = { "No Lock", "Lock", "Sync", "N/A" };

int main(int argc, char **argv) {
    int err;
    snd_hwdep_t *hw;
    snd_hwdep_info_t *info;
    snd_hwdep_info_alloca(&info);
    int i;
    char *cardname;
    struct madifx_level_buffer mybuf;
    struct madifx_config myconfig;
    struct madifx_status mystatus;

    if (argc > 1) {
	    cardname = argv[1];
    } else {
	    cardname = "hw:MADIFXtest";
    }

    if ((err = snd_hwdep_open(&hw, cardname, SND_HWDEP_OPEN_DUPLEX)) != 0) {
        perror("Error opening hwdep device");
        return 1; 
    }

    if ((err = snd_hwdep_ioctl(hw, SNDRV_MADIFX_IOCTL_GET_LEVEL, &mybuf)) < 0) {
        perror("Hwdep ioctl error:");
        snd_hwdep_close(hw);
        return 2; 
    }

    if ((err = snd_hwdep_ioctl(hw, SNDRV_MADIFX_IOCTL_GET_CONFIG, &myconfig)) < 0) {
        perror("Hwdep ioctl error:");
        snd_hwdep_close(hw);
        return 3; 
    }

    if ((err = snd_hwdep_ioctl(hw, SNDRV_MADIFX_IOCTL_GET_STATUS, &mystatus)) < 0) {
        perror("Hwdep ioctl error:");
        snd_hwdep_close(hw);
        return 4; 
    }

    for (i=0 ; i < 16; i++) {
	    printf ("%i ", mybuf.rms_out[i]);
    }
    printf ("\n");

    for (i=0; i < 256; i++) {
	    if (mybuf.rms_in[i] != 0) {
		    printf ("%i at %i\n", mybuf.rms_in[i], i);
	    }
    }
    printf ("\n");

    for (i=0; i < 3; i++) {
	    printf ("MADI %i TX64: %i SMUX: %i\n", i+1, myconfig.madi_tx_64[i], myconfig.madi_smux[i]);
    }

    printf ("WCK48: %i\nWCTERM: %i\n", myconfig.wck48, myconfig.wcterm);
    printf ("AES Pro: %i\n", myconfig.aespro);
    printf ("Redundancy mode: %i\nMirror MADI out: %i\n", myconfig.redundancy_mode,
		    myconfig.mirror_madi_out);

    printf ("\n");


    printf ("clock_selection: %s\n", texts_madifx_clock_source[mystatus.clock_selection]);
    printf ("system sample rate: %i\n", mystatus.system_sample_rate);

    for (i = 0; i < 3; i++) {
	    printf ("MADI %i (%s): speed: %i #ch: %s\n",
			    i+1,
			    synctext[mystatus.sync_check[i]],
			    mystatus.external_sample_rates[i],
			    rxformat[mystatus.madi_channelcount[i]]);
    }

    printf ("AES (%s): speed: %i\n",
		    synctext[mystatus.sync_check[syncsource_aes]],
		    mystatus.external_sample_rates[syncsource_aes]);

    printf ("WC (%s): speed: %i\n",
		    synctext[mystatus.sync_check[syncsource_wc]],
		    mystatus.external_sample_rates[syncsource_wc]);

    printf ("Sync-In (%s): speed: %i\n",
		    synctext[mystatus.sync_check[syncsource_syncin]],
		    mystatus.external_sample_rates[syncsource_syncin]);



    snd_hwdep_close(hw);
}

