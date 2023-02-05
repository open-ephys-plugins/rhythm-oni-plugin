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

#include "Headstage.h"

using namespace ONIRhythmNode;

Headstage::Headstage(Rhd2000ONIBoard::BoardDataSource dataSource_, int MAX_H) :
    dataSource(dataSource_),
    MAX_NUM_HEADSTAGES(MAX_H),
    numStreams(0), 
    channelsPerStream(32), 
    halfChannels(false),
    streamIndex(-1),
    firstChannelIndex(0),
    namingScheme(GLOBAL_INDEX)
{
    StringArray stream_prefix = 
    {   "A1", "A2", 
        "B1", "B2",
        "C1", "C2",
        "D1", "D2",
        "E1", "E2",
        "F1", "F2",
        "G1", "G2",
        "H1", "H2"
    };

    prefix = stream_prefix[int(dataSource_)];
}

int Headstage::getNumStreams() const
{
    return numStreams;
}

void Headstage::setNumStreams(int num)
{

    LOGD("Headstage ", prefix, " setting num streams to ", num);


    if (num == 2)
        halfChannels = false;
    
    if (numStreams != num)
    {
        numStreams = num;
        
        generateChannelNames();
    }
    
}

void Headstage::setChannelsPerStream(int nchan)
{

    LOGD("Headstage ", prefix, " setting channels per stream to ", nchan);

    if (channelsPerStream != nchan)
    {
        channelsPerStream = nchan;

        generateChannelNames();
    }
    
}

void Headstage::setFirstStreamIndex(int streamIndex_)
{
    streamIndex = streamIndex_;
}

void Headstage::setFirstChannel(int channelIndex)
{

    LOGD("Headstage ", prefix, " setting first channel to ", channelIndex);

    if (firstChannelIndex != channelIndex)
    {
        firstChannelIndex = channelIndex;

        if (namingScheme == GLOBAL_INDEX)
            generateChannelNames();
    }
}

int Headstage::getStreamIndex (int offset) const
{
    return streamIndex + offset;
}

int Headstage::getNumChannels() const
{
    return channelsPerStream * numStreams;
}

void Headstage::setHalfChannels(bool half)
{

    if (getNumChannels() == 64)
        return;
    
    if (halfChannels != half)
    {
        halfChannels = half;

        generateChannelNames();
    }
    
}

int Headstage::getNumActiveChannels() const
{
    return (int)(getNumChannels() / (halfChannels ? 2 : 1));
}

Rhd2000ONIBoard::BoardDataSource Headstage::getDataStream (int index) const
{
    if (index < 0 || index > 1) index = 0;
        return static_cast<Rhd2000ONIBoard::BoardDataSource>(dataSource + MAX_NUM_HEADSTAGES * index);
}

bool Headstage::isConnected() const
{
    return (numStreams > 0);
}

String Headstage::getChannelName(int ch) const
{

    String name;

    if (ch > -1 && ch < channelNames.size())
        name = channelNames[ch];
    else
        name = " ";

    if (ch == 0)
      LOGD("Headstage ", prefix, " channel ", ch, " name: ", name);

    return name;
}

String Headstage::getStreamPrefix() const
{
    return prefix;
}

void Headstage::setChannelName(String name, int ch)
{
    if (ch > -1 && ch < channelNames.size())
        channelNames.set(ch, name);
}

void Headstage::setNamingScheme(ChannelNamingScheme scheme)
{
    if (namingScheme != scheme)
    {
        namingScheme = scheme;

        generateChannelNames();
    }
    
}

void Headstage::generateChannelNames()
{
    channelNames.clear();

    switch (namingScheme)
    {
    case GLOBAL_INDEX:
        for (int i = 0; i < getNumActiveChannels(); i++)
        {
            channelNames.add("CH" + String(firstChannelIndex + i + 1));
        }
        break;
    case STREAM_INDEX:
        for (int i = 0; i < getNumActiveChannels(); i++)
        {
            channelNames.add(prefix + "_CH" + String(i + 1));
        }
    }
}

void Headstage::setImpedances(Impedances& impedances)
{

    impedanceMagnitudes.clear();
    impedancePhases.clear();

    for (int i = 0; i < impedances.streams.size(); i++)
    {
        if (impedances.streams[i] == streamIndex)
        {
            impedanceMagnitudes.add(impedances.magnitudes[i]);
            impedancePhases.add(impedances.phases[i]);
        }

        if (numStreams == 2 && impedances.streams[i] == streamIndex + 1)
        {
            impedanceMagnitudes.add(impedances.magnitudes[i]);
            impedancePhases.add(impedances.phases[i]);
        }
    }
}

float Headstage::getImpedanceMagnitude(int channel) const
{
    if (channel < impedanceMagnitudes.size())
        return impedanceMagnitudes[channel];

    return 0.0f;
}

float Headstage::getImpedancePhase(int channel) const
{
    if (channel < impedancePhases.size())
        return impedancePhases[channel];

    return 0.0f;
}