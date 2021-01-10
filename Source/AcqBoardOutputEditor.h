/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2020 Open Ephys

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

#ifndef __ACQBOARDOUTPUTEDITOR_H_28EB4CC9__
#define __ACQBOARDOUTPUTEDITOR_H_28EB4CC9__


#include <EditorHeaders.h>

namespace AcqBoardOutputNamespace {

    /**

      User interface for the AcqBoardOutput processor.

      @see AcqBoardOutput

    */

    class AcqBoardOutput;

    class AcqBoardOutputEditor : public GenericEditor,
        public ComboBox::Listener

    {
    public:
        AcqBoardOutputEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
        virtual ~AcqBoardOutputEditor();

        void receivedEvent();

        //ImageIcon* icon;

        void comboBoxChanged(ComboBox* comboBoxThatHasChanged);

        AcqBoardOutput* board;

        void buttonEvent(Button* button);

        Array<float> durations;


    private:

        // ScopedPointer<UtilityButton> triggerButton;
        ScopedPointer<ComboBox> inputChannelSelector;
        ScopedPointer<ComboBox> outputChannelSelector;
        ScopedPointer<ComboBox> gateChannelSelector;
        ScopedPointer<ComboBox> durationSelector;

        ScopedPointer<UtilityButton> triggerButton;

        void timerCallback();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AcqBoardOutputEditor);

    };


}

#endif  // __AcqBoardOutputEDITOR_H_28EB4CC9__
