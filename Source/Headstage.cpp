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

using namespace RhythmNode;

Headstage::Headstage(Rhd2000EvalBoard::BoardDataSource stream) :
    dataStream(stream), 
    numStreams(0), 
    channelsPerStream(32), 
    halfChannels(false),
    streamIndex(-1)
{

}

void Headstage::setNumStreams(int num)
{
    numStreams = num;
}

void Headstage::setChannelsPerStream(int nchan, int index)
{
    channelsPerStream = nchan;
    streamIndex = index;
}

int Headstage::getStreamIndex (int index) const
{
    return streamIndex + index;
}

int Headstage::getNumChannels() const
{
    return channelsPerStream * numStreams;
}

int Headstage::getNumStreams() const
{
    return numStreams;
}

void Headstage::setHalfChannels(bool half)
{
    halfChannels = half;
}

int Headstage::getNumActiveChannels() const
{
    return (int)(getNumChannels() / (halfChannels ? 2 : 1));
}

Rhd2000EvalBoard::BoardDataSource Headstage::getDataStream (int index) const
{
    if (index < 0 || index > 1) index = 0;
        return static_cast<Rhd2000EvalBoard::BoardDataSource>(dataStream+MAX_NUM_HEADSTAGES*index);
}

bool Headstage::isConnected() const
{
    return (numStreams > 0);
}
