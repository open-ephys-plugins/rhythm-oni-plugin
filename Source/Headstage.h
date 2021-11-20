/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2021 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __HEADSTAGE_H_2C4CBD67__
#define __HEADSTAGE_H_2C4CBD67__

#include <DataThreadHeaders.h>

#include <stdio.h>
#include <string.h>
#include <array>
#include <atomic>

#include "rhythm-api/rhd2000evalboard.h"
#include "rhythm-api/rhd2000registers.h"
#include "rhythm-api/rhd2000datablock.h"
#include "rhythm-api/okFrontPanelDLL.h"

#include "DeviceThread.h"

namespace RhythmNode
{

	class Headstage
	{
	public:

		/** Constructor */
		Headstage(Rhd2000EvalBoard::BoardDataSource stream);

		/** Destructor*/
		~Headstage() { }

		/** Returns the index of this headstage's data stream */
		int getStreamIndex(int index)  const;

		/** Returns the number of channels this headstage sens*/
		int getNumChannels()            const;

		/** Returns the number of streams this headstage sends*/
		int getNumStreams()             const;

		/** Returns the number of actively acquired channels*/
		int getNumActiveChannels()      const;

		/** Returns true if the headstage is connected*/
		bool isConnected() const;

		/** Returns the BoardDataSource object for a given index*/
		Rhd2000EvalBoard::BoardDataSource getDataStream(int index) const;

		/** Sets the number of streams for this headstage*/
		void setNumStreams(int num);

		/** Sets the number of channels per stream*/
		void setChannelsPerStream(int nchan, int index);

		/** Sets the number of half-channels; mainly used for the 16-ch RHD2132 board */
		void setHalfChannels(bool half); 

	private:
		Rhd2000EvalBoard::BoardDataSource dataStream;

		int streamIndex;
		int numStreams;
		int channelsPerStream;
		bool halfChannels;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Headstage);
	};


}
#endif  // __HEADSTAGE_H_2C4CBD67__
