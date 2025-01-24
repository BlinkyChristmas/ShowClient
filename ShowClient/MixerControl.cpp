// Copyright © 2025 Charles Kerr. All rights reserved.

#include "MixerControl.hpp"
#ifdef BEAGLE
#include <alsa/asoundlib.h>
#include <iostream>
auto setVolume(long volume) -> long {
    snd_mixer_t *handle;
    snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;
    const char * card = "Default"
    int err ;
    if ( (err = snd_mixer_open( &handle,0)) < 0 ) {
        std::cerr << "Error opening mixer: " << snd_strerror(err) << std::endl;
        return 0 ;
    }
    if ( ( err = snd_mixer_attach(handle,card)) < 0 ) {
        std::cerr << "Error attaching to card: " << snd_strerror(err) << std::endl;
        snd_mixer_close(handle);
        return 0 ;
    }
    if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
        std::cerr << "Error registering mixer elements: " << snd_strerror(err) << std::endl;
        snd_mixer_close(handle);
        return 0;
    }
    if ((err = snd_mixer_load(handle)) < 0) {
        std::cerr << "Error loading mixer elements: " << snd_strerror(err) << std::endl;
        snd_mixer_close(handle);
        return 0;
    }
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, control);

    if ((elem = snd_mixer_find_selem(handle, sid)) == NULL) {
        std::cerr << "Error finding mixer control: " << snd_strerror(err) << std::endl;
        snd_mixer_close(handle);
        return 0;
    }

    if ((err = snd_mixer_selem_set_playback_volume_all(elem, volume)) < 0) {
        std::cerr << "Error setting volume: " << snd_strerror(err) << std::endl;
        snd_mixer_close(handle);
        return 0;
    }

    snd_mixer_close(handle);

    return volume;
}


#else

auto setVolume(int card, long volume) -> long {
    return volume;
}
#endif
