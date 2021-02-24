/*
** audio.h
**
** This file is part of mkxp.
**
** Copyright (C) 2013 Jonas Kulla <Nyocurio@gmail.com>
**
** mkxp is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** mkxp is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with mkxp.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AUDIO_H
#define AUDIO_H

#include "util.h"
#include "audiofilter.h"

/* Concerning the 'pos' parameter:
 *   RGSS3 actually doesn't specify a format for this,
 *   it's only implied that it is a numerical value
 *   (must be 0 on invalid cases), and it's not used for
 *   anything outside passing it back into bgm/bgs_play.
 *   We use this freedom to define pos to be a float,
 *   in seconds of playback. (RGSS3 seems to use large
 *   integers that _look_ like sample offsets but I can't
 *   quite make out their meaning yet) */

struct AudioPrivate;
struct RGSSThreadData;

class Audio
{
public:
	void bgmPlay(const char *filename,
	             int volume = 100,
	             int pitch = 100,
	             float pos = -1,
				 bool fadeInOnOffset = true);
	void bgmStop();
	void bgmFade(int time);

	void bgsPlay(const char *filename,
	             int volume = 100,
	             int pitch = 100,
	             float pos = -1,
				 bool fadeInOnOffset = true);
	void bgsStop();
	void bgsFade(int time);

	void mePlay(const char *filename,
	            int volume = 100,
	            int pitch = 100);
	void meStop();
	void meFade(int time);

	void sePlay(const char *filename,
	            int volume = 100,
	            int pitch = 100);
	void seStop();

	void bgmCrossfade(const char *filename,
				 	  float time = 2,
			     	  int volume = 100,
				 	  int pitch = 100,
					  float offset = -1);
	void bgsCrossfade(const char *filename,
				 	  float time = 2,
			     	  int volume = 100,
				 	  int pitch = 100,
					  float offset = -1);
	void meCrossfade(const char *filename,
					 float time = 2,
			    	 int volume = 100,
					 int pitch = 100,
					 float offset = -1);

	float bgmPos();
	float bgsPos();

	bool bgmIsPlaying();
	bool bgsIsPlaying();
	bool meIsPlaying();

#define AUDIO_H_DECL_FILTER_FUNCS(entity) \
	void entity##AddFilter(AudioFilter* filter); \
	void entity##ClearFilters(); \
	void entity##SetALFilter(AL::Filter::ID filter); \
	void entity##ClearALFilter(); \
	void entity##SetALEffect(ALuint effect); \
	void entity##ClearALEffect();

	AUDIO_H_DECL_FILTER_FUNCS(bgm)
	AUDIO_H_DECL_FILTER_FUNCS(bgs)
	AUDIO_H_DECL_FILTER_FUNCS(me)

	void reset();

        /* Non-standard extension */
	DECL_ATTR( BGM_Volume, int)
        DECL_ATTR( SFX_Volume, int)

private:
	Audio(RGSSThreadData &rtData);
	~Audio();

	friend struct SharedStatePrivate;

	AudioPrivate *p;
};

#endif // AUDIO_H
