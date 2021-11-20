/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#include "ChannelCanvas.h"

#include "ChannelList.h"

#include "../DeviceThread.h"

using namespace RhythmNode;

/**********************************************/

ChannelCanvas::ChannelCanvas(DeviceThread* board_,
                             DeviceEditor* editor_) :
    board(board_),
    editor(editor_)
{

    channelViewport = std::make_unique<Viewport>();

    channelList = std::make_unique<ChannelList>(board, editor);

    channelViewport->setViewedComponent(channelList.get(), false);
    channelViewport->setScrollBarsShown(true, true);
    addAndMakeVisible(channelViewport.get());

    resized();
    update();
}


void ChannelCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::grey);

}

void ChannelCanvas::refresh()
{
    repaint();
}

void ChannelCanvas::refreshState()
{
    resized();
}

void ChannelCanvas::update()
{
    // create channel buttons (name, gain, recording, impedance, ... ?)
    channelList->update();

    //if (static_cast<DeviceThread*>(processor->getThread())->isAcquisitionActive())
    //{
    //   channelList->disableAll();
    //}
}

void ChannelCanvas::beginAnimation()
{
    channelList->disableAll();
}

void ChannelCanvas::endAnimation()
{
    channelList->enableAll();
}

void ChannelCanvas::resized()
{

    int scrollBarThickness = channelViewport->getScrollBarThickness();
    int numChannels = 35; // max channels per stream? (32+3)*2

    channelViewport->setBounds(0,0,getWidth(), getHeight());
    channelList->setBounds(0, 0, getWidth()-scrollBarThickness, 200+22*numChannels);
}


void ChannelCanvas::updateImpedance(Array<int> streams, Array<int> channels, Array<float> magnitude, Array<float> phase)
{
    channelList->updateImpedance(streams, channels,  magnitude, phase);
}
