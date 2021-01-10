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
        , outputChannel(0)
        , inputChannel(-1)
        , gateChannel(-1)
        , state(true)
        , acquisitionIsActive(false)
        , durationMs(100)
    {
        setProcessorType(PROCESSOR_TYPE_SINK);

        if (socket.connect("127.0.0.1", 55555, 500))
        {
            CoreServices::sendStatusMessage("Successful connection.");
        }
        else {
            CoreServices::sendStatusMessage("Unsuccessful connection.");
        }
    }


    AcqBoardOutput::~AcqBoardOutput()
    {

    }


    AudioProcessorEditor* AcqBoardOutput::createEditor()
    {
        editor = new AcqBoardOutputEditor(this, true);
        return editor;
    }


    void AcqBoardOutput::handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int sampleNum)
    {
        if (Event::getEventType(event) == EventChannel::TTL)
        {
            TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, eventInfo);

            //int eventNodeId = *(dataptr+1);
            const int eventId = ttl->getState() ? 1 : 0;
            const int eventChannel = ttl->getChannel();

            // std::cout << "Received event from " << eventNodeId
            //           << " on channel " << eventChannel
            //           << " with value " << eventId << std::endl;

            if (eventChannel == gateChannel)
            {
                if (eventId == 1)
                    state = true;
                else
                    state = false;
            }

            if (state)
            {
                if (inputChannel == -1 || eventChannel == inputChannel)
                {

                    triggerOutput();

                }
            }
        }
    }

    void AcqBoardOutput::triggerOutput()
    {
        setState(outputChannel, HIGH);
        startTimer(durationMs);
    }

    void AcqBoardOutput::timerCallback()
    {
        setState(outputChannel, LOW);
        stopTimer();
    }

    void AcqBoardOutput::setState(int channel, State state)
    {

        if (socket.isConnected())
        {
            unsigned char buffer[3];

            buffer[0] = 127;
            buffer[1] = channel;

            if (state == HIGH)
                buffer[2] = 1;
            else
                buffer[2] = 0;

            socket.write(buffer, 3);
        }
    }

    void AcqBoardOutput::setParameter(int parameterIndex, float newValue)
    {
        // make sure current output channel is off:
        //arduino.sendDigital(outputChannel, ARD_LOW);

        if (parameterIndex == 0)
        {
            outputChannel = (int)newValue;
        }
        else if (parameterIndex == 1)
        {
            inputChannel = (int)newValue;
        }
        else if (parameterIndex == 2)
        {
            gateChannel = (int)newValue;

            if (gateChannel == -1)
                state = true;
            else
                state = false;
        }
        else if (parameterIndex == 3)
        {
            durationMs = newValue;
        }
    }

    void AcqBoardOutput::setDurationMs(float ms)
    {
        setParameter(3, ms);
    }


    void AcqBoardOutput::setOutputChannel(int chan)
    {
        setParameter(0, chan);
    }


    void AcqBoardOutput::setInputChannel(int chan)
    {
        setParameter(1, chan - 1);
    }


    void AcqBoardOutput::setGateChannel(int chan)
    {
        setParameter(2, chan - 1);
    }


    bool AcqBoardOutput::enable()
    {
        acquisitionIsActive = true;

        return true;
    }


    bool AcqBoardOutput::disable()
    {

        acquisitionIsActive = false;

        return true;
    }


    void AcqBoardOutput::process(AudioSampleBuffer& buffer)
    {
        checkForEvents();
    }

}