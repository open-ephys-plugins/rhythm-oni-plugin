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

#include "AcqBoardOutputEditor.h"
#include "AcqBoardOutput.h"
#include <stdio.h>


namespace AcqBoardOutputNamespace {

    AcqBoardOutputEditor::AcqBoardOutputEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors = true)
        : GenericEditor(parentNode, useDefaultParameterEditors)

    {

        accumulator = 0;

        desiredWidth = 200;

        board = (AcqBoardOutput*)parentNode;

        inputChannelSelector = new ComboBox();
        inputChannelSelector->setBounds(10, 30, 55, 20);
        inputChannelSelector->addListener(this);
        inputChannelSelector->addItem("Trig", 1);

        for (int i = 0; i < 16; i++)
            inputChannelSelector->addItem(String(i + 1), i + 2); // start numbering at one for user-visible channels

        inputChannelSelector->setSelectedId(1, dontSendNotification);
        addAndMakeVisible(inputChannelSelector);

        outputChannelSelector = new ComboBox();
        outputChannelSelector->setBounds(10, 80, 80, 20);
        outputChannelSelector->addListener(this);
        outputChannelSelector->addItem("Output CH", 1);

        for (int i = 0; i < 8; i++)
            outputChannelSelector->addItem(String(i + 1), i + 2);

        outputChannelSelector->setSelectedId(1, dontSendNotification);
        addAndMakeVisible(outputChannelSelector);

        gateChannelSelector = new ComboBox();
        gateChannelSelector->setBounds(10, 55, 55, 20);
        gateChannelSelector->addListener(this);
        gateChannelSelector->addItem("Gate", 1);

        for (int i = 0; i < 16; i++)
            gateChannelSelector->addItem(String(i + 1), i + 2); // start numbering at one for user-visible channels

        gateChannelSelector->setSelectedId(1, dontSendNotification);
        addAndMakeVisible(gateChannelSelector);

        durations.add(100.0f);
        durations.add(500.0f);
        durations.add(1000.0f);

        durationSelector = new ComboBox();
        durationSelector->setBounds(10, 105, 85, 20);
        durationSelector->addListener(this);
        durationSelector->addItem("Duration", 1);

        for (int i = 0; i < durations.size(); i++)
        {
            durationSelector->addItem(String(durations[i]) + " ms", i + 2);
        }

        durationSelector->setSelectedId(1, dontSendNotification);
        addAndMakeVisible(durationSelector);

        triggerButton = new UtilityButton("Trigger", Font("Small Text", 13, Font::plain));
        triggerButton->setBounds(80, 35, 80, 30);
        triggerButton->addListener(this);
        addAndMakeVisible(triggerButton);

    }

    AcqBoardOutputEditor::~AcqBoardOutputEditor()
    {
    }

    void AcqBoardOutputEditor::receivedEvent()
    {


    }

    void AcqBoardOutputEditor::buttonEvent(Button* button)
    {

        if (button == triggerButton)
        {
            AcqBoardOutput* processor = (AcqBoardOutput*)getProcessor();
            processor->triggerOutput();
        }
    }

    void AcqBoardOutputEditor::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
    {
        if (comboBoxThatHasChanged == outputChannelSelector)
        {
            board->setOutputChannel(outputChannelSelector->getSelectedId() - 2);
        }
        else if (comboBoxThatHasChanged == inputChannelSelector)
        {
            board->setInputChannel(inputChannelSelector->getSelectedId() - 1);
        }
        else if (comboBoxThatHasChanged == gateChannelSelector)
        {
            board->setGateChannel(gateChannelSelector->getSelectedId() - 1);
        }
        else if (comboBoxThatHasChanged == durationSelector)
        {
            board->setDurationMs(durations[durationSelector->getSelectedId() - 2]);
        }
    }

    void AcqBoardOutputEditor::timerCallback()
    {

        repaint();

        accumulator++;

        if (isFading)
        {

            if (accumulator > 15.0)
            {
                stopTimer();
                isFading = false;
            }

        }
        else
        {

            if (accumulator < 10.0)
            {
                //icon->setOpacity(0.8f-(0.05*float(accumulator)));
                accumulator++;
            }
            else
            {
                //icon->setOpacity(0.3f);
                stopTimer();
                accumulator = 0;
            }
        }
    }

}