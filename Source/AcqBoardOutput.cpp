/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#include "AcqBoardOutput.h"
#include "AcqBoardOutputEditor.h"

#include <stdio.h>


namespace AcqBoardOutputNamespace {

    AcqBoardOutput::AcqBoardOutput()
        : GenericProcessor("Acq Board Output")
        , gateIsOpen(true)
    {

        addIntParameter(Parameter::STREAM_SCOPE, "output_bit", "The digital output to trigger", 1, 1, 8);
        addIntParameter(Parameter::STREAM_SCOPE, "input_bit", "The TTL bit for triggering output", 1, 1, 16);
        addIntParameter(Parameter::STREAM_SCOPE, "gate_bit", "The TTL bit for gating the output", 1, 1, 16);
        
        addFloatParameter(
            Parameter::STREAM_SCOPE, 
            "event_duration", 
            "The amount of time (in ms) the output stays high", 
            500, 100, 2000, 1.0);
    }


    AudioProcessorEditor* AcqBoardOutput::createEditor()
    {
        editor = std::make_unique<AcqBoardOutputEditor>(this);
        return editor.get();
    }

    void AcqBoardOutput::handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int sampleNum)
    {
        if (Event::getEventType(event) == EventChannel::TTL)
        {
            TTLEventPtr ttl = TTLEvent::deserialize(event, eventInfo);

            const int eventBit = ttl->getBit() + 1;
            DataStream* stream = getDataStream(ttl->getStreamId());

            if (eventBit == int((*stream)["gate_bit"]))
            {
                if (ttl->getState())
                    gateIsOpen = true;
                else
                    gateIsOpen = false;
            }

            if (gateIsOpen)
            {
                if (eventBit == int((*stream)["input_bit"]))
                {

                    if (ttl->getState())
                    {
                        broadcastMessage("ACQBOARD TRIGGER "
                            + (*stream)["output_bit"].toString()
                            + " "
                            + (*stream)["event_duration"].toString());
                    }
                }
            }
        }
    }

    void AcqBoardOutput::process(AudioBuffer<float>& buffer)
    {
        checkForEvents();
    }

}