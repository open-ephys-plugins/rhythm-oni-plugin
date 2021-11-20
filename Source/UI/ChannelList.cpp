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

#include "ChannelList.h"

#include "ChannelComponent.h"

#include "../DeviceThread.h"
#include "../DeviceEditor.h"

using namespace RhythmNode;


ChannelList::ChannelList(DeviceThread* board_, DeviceEditor* editor_) :
    board(board_), editor(editor_)
{

    channelComponents.clear();

    numberingSchemeLabel = new Label("Numbering scheme:","Numbering scheme:");
    numberingSchemeLabel->setEditable(false);
    numberingSchemeLabel->setBounds(10,10,150, 25);
    numberingSchemeLabel->setColour(Label::textColourId,juce::Colours::white);
    addAndMakeVisible(numberingSchemeLabel);

    numberingScheme = new ComboBox("numberingScheme");
    numberingScheme->addItem("Continuous",1);
    numberingScheme->addItem("Per Stream",2);
    numberingScheme->setBounds(160,10,100,25);
    numberingScheme->addListener(this);
    numberingScheme->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(numberingScheme);

    impedanceButton = new UtilityButton("Measure Impedance", Font("Default", 13, Font::plain));
    impedanceButton->setRadius(3);
    impedanceButton->setBounds(280,10,140,25);
    impedanceButton->addListener(this);
    addAndMakeVisible(impedanceButton);

    saveImpedanceButton = new ToggleButton("Save impedance measurements");
    saveImpedanceButton->setBounds(430,10,110,25);
    saveImpedanceButton->setToggleState(false, dontSendNotification);
    saveImpedanceButton->addListener(this);
    addAndMakeVisible(saveImpedanceButton);

    autoMeasureButton = new ToggleButton("Measure impedance at acquisition start");
    autoMeasureButton->setBounds(550,10,150,25);
    autoMeasureButton->setToggleState(false,dontSendNotification);
    autoMeasureButton->addListener(this);
    addAndMakeVisible(autoMeasureButton);

    gains.clear();
    gains.add(0.01);
    gains.add(0.1);
    gains.add(1);
    gains.add(2);
    gains.add(5);
    gains.add(10);
    gains.add(20);
    gains.add(50);
    gains.add(100);
    gains.add(500);
    gains.add(1000);

    update();
}


void ChannelList::buttonClicked(Button* btn)
{

    if (btn == impedanceButton)
    {
        editor->measureImpedance();
    }
    else if (btn == saveImpedanceButton)
    {
        //editor->setSaveImpedance(btn->getToggleState());
    }
    else if (btn == autoMeasureButton)
    {
        //editor->setAutoMeasureImpedance(btn->getToggleState());
    }
}

void ChannelList::update()
{
    /*// const int columnWidth = 330;
    const int columnWidth = 250;
    // Query processor for number of channels, types, gains, etc... and update the UI
    channelComponents.clear();
    staticLabels.clear();

    RHD2000Thread* thread = (RHD2000Thread*)proc->getThread();
    DataChannel::DataChannelTypes type;

    // find out which streams are active.
    bool hsActive[MAX_NUM_HEADSTAGES+1];
    int numActiveHeadstages = 0;
    int hsColumn[MAX_NUM_HEADSTAGES + 1];
    int numChannelsPerHeadstage[MAX_NUM_HEADSTAGES + 1];

    chainUpdate = false;

    for (int k = 0; k<MAX_NUM_HEADSTAGES; k++)
    {
        if (thread->isHeadstageEnabled(k))
        {
            numChannelsPerHeadstage[k] = thread->getActiveChannelsInHeadstage(k);
            hsActive[k] = true;
            hsColumn[k] = numActiveHeadstages*columnWidth;
            numActiveHeadstages++;
        }
        else
        {
            numChannelsPerHeadstage[k] = 0;
            hsActive[k] = false;
            hsColumn[k] = 0;
        }
    }

    if (thread->getNumDataOutputs(DataChannel::ADC_CHANNEL,0) > 0)
    {
        numChannelsPerHeadstage[MAX_NUM_HEADSTAGES] = thread->getNumDataOutputs(DataChannel::ADC_CHANNEL, 0);
        hsActive[MAX_NUM_HEADSTAGES] = true;
        hsColumn[MAX_NUM_HEADSTAGES] = numActiveHeadstages*columnWidth;
        numActiveHeadstages++;
    }
    else
    {
        numChannelsPerHeadstage[MAX_NUM_HEADSTAGES] = 0;
        hsActive[MAX_NUM_HEADSTAGES] = false;
        hsColumn[MAX_NUM_HEADSTAGES] = 0;
    }

    StringArray streamNames;
    streamNames.add("Port A1");
    streamNames.add("Port A2");
    streamNames.add("Port B1");
    streamNames.add("Port B2");
    streamNames.add("Port C1");
    streamNames.add("Port C2");
    streamNames.add("Port D1");
    streamNames.add("Port D2");
    streamNames.add("ADC");

    for (int k = 0; k < MAX_NUM_HEADSTAGES + 1; k++)
    {
        if (hsActive[k])
        {
            Label* lbl = new Label(streamNames[k],streamNames[k]);
            lbl->setEditable(false);
            lbl->setBounds(10+hsColumn[k],40,columnWidth, 25);
            lbl->setJustificationType(juce::Justification::centred);
            lbl->setColour(Label::textColourId,juce::Colours::white);
            staticLabels.add(lbl);
            addAndMakeVisible(lbl);

        }

    }

    for (int k = 0; k < MAX_NUM_HEADSTAGES + 1; k++) // +1 is for the ADC "headstage"
    {
        if (hsActive[k])
        {
            int nchans = numChannelsPerHeadstage[k];
            if (k < MAX_NUM_HEADSTAGES && thread->isAuxEnabled())
                nchans += 3;
            for (int ch = 0; ch < nchans; ch++)
            {
                int channelGainIndex = 1;
                int realChan = thread->getChannelFromHeadstage(k, ch);
                float ch_gain = proc->getDataChannel(realChan)->getBitVolts() / proc->getBitVolts(proc->getDataChannel(realChan));
                for (int j = 0; j < gains.size(); j++)
                {
                    if (fabs(gains[j] - ch_gain) < 1e-3)
                    {
                        channelGainIndex = j;
                        break;
                    }
                }
                if (k < MAX_NUM_HEADSTAGES)
                    type = ch < numChannelsPerHeadstage[k] ? DataChannel::HEADSTAGE_CHANNEL : DataChannel::AUX_CHANNEL;
                else
                    type = DataChannel::ADC_CHANNEL;

                FPGAchannelComponent* comp = new FPGAchannelComponent(this, realChan, channelGainIndex + 1, thread->getChannelName(realChan), gains, type);
                comp->setBounds(10 + hsColumn[k], 70 + ch * 22, columnWidth, 22);
                comp->setUserDefinedData(k);
                addAndMakeVisible(comp);
                channelComponents.add(comp);
            }
        }
    }


    StringArray ttlNames;
    proc->getEventChannelNames(ttlNames);
    // add buttons for TTL channels
    for (int k=0; k<ttlNames.size(); k++)
    {
        FPGAchannelComponent* comp = new FPGAchannelComponent(this,k, -1, ttlNames[k],gains,DataChannel::INVALID); //let's treat invalid as an event channel
        comp->setBounds(10+numActiveHeadstages*columnWidth,70+k*22,columnWidth,22);
        comp->setUserDefinedData(k);
        addAndMakeVisible(comp);
        channelComponents.add(comp);
    }

    Label* lbl = new Label("TTL Events", "TTL Events");
    lbl->setEditable(false);
    lbl->setBounds(numActiveHeadstages*columnWidth, 40, columnWidth, 25);
    lbl->setJustificationType(juce::Justification::centred);
    lbl->setColour(Label::textColourId, juce::Colours::white);
    staticLabels.add(lbl);
    addAndMakeVisible(lbl);

    chainUpdate = true;*/
}

void ChannelList::disableAll()
{
    for (auto channelComponent: channelComponents)
    {
        channelComponent->disableEdit();
    }

    impedanceButton->setEnabled(false);
    saveImpedanceButton->setEnabled(false);
    autoMeasureButton->setEnabled(false);
    numberingScheme->setEnabled(false);
}

void ChannelList::enableAll()
{
    for (int k=0; k<channelComponents.size(); k++)
    {
        channelComponents[k]->enableEdit();
    }
    impedanceButton->setEnabled(true);
    saveImpedanceButton->setEnabled(true);
    autoMeasureButton->setEnabled(true);
    numberingScheme->setEnabled(true);
}

void ChannelList::setNewGain(int channel, float gain)
{
    //RHD2000Thread* thread = (RHD2000Thread*)proc->getThread();
    //thread->modifyChannelGain(channel, gain);
    //if (chainUpdate)
    //    proc->requestChainUpdate();
}

void ChannelList::setNewName(int channel, String newName)
{
    //RHD2000Thread* thread = (RHD2000Thread*)proc->getThread();
    //thread->modifyChannelName(channel, newName);
    //if (chainUpdate)
    //    proc->requestChainUpdate();
}

void ChannelList::updateButtons()
{
}

int ChannelList::getNumChannels()
{
    return 0;
}

void ChannelList::comboBoxChanged(ComboBox* b)
{
    if (b == numberingScheme)
    {
       // SourceNode* p = (SourceNode*)proc;
        //RHD2000Thread* thread = (RHD2000Thread*)p->getThread();
        //int scheme = numberingScheme->getSelectedId();
        //thread->setDefaultNamingScheme(scheme);
        //update();
       //p->requestChainUpdate();
    }
}

void ChannelList::updateImpedance(Array<int> streams, Array<int> channels, Array<float> magnitude, Array<float> phase)
{
    int i = 0;
    for (int k = 0; k < streams.size(); k++)
    {
        if (i >= channelComponents.size())
            break; //little safety

        if (channelComponents[i]->type != ContinuousChannel::ELECTRODE)
        {
            k--;
        }
        else
        {
            channelComponents[i]->setImpedanceValues(magnitude[k], phase[k]);
        }
        i++;
    }

}
