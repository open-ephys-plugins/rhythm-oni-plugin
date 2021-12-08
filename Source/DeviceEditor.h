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

#ifndef __DEVICEEDITOR_H_2AD3C591__
#define __DEVICEEDITOR_H_2AD3C591__

#include <VisualizerEditorHeaders.h>

namespace RhythmNode
{

	class HeadstageOptionsInterface;
	class SampleRateInterface;
	class BandwidthInterface;
	class DSPInterface;
	class AudioInterface;
	class ClockDivideInterface;
	class DeviceThread;
	class ChannelCanvas;

	struct ImpedanceData;

	class DeviceEditor : public VisualizerEditor, 
						 public ComboBox::Listener, 
						 public Button::Listener,
					     public PopupChannelSelector::Listener

	{
	public:

		/** Constructor */
		DeviceEditor(GenericProcessor* parentNode, DeviceThread* thread);

		/** Destructor*/
		~DeviceEditor() { }

		/** Respond to combo box changes (e.g. sample rate)*/
		void comboBoxChanged(ComboBox* comboBox);

		/** Respond to button clicks*/
		void buttonClicked(Button* button);

		/** Disable UI during acquisition*/
		void startAcquisition();

		/** Enable UI after acquisition is finished*/
		void stopAcquisition();

		/** Runs impedance test*/
		void measureImpedance();

		/** Saves impedance data to a file*/
		void saveImpedance(File& file);

		/** Updates channel canvas*/
		void updateSettings();

		/** Saves custom parameters */
		void saveVisualizerEditorParameters(XmlElement* xml) override;

		/** Loads custom parameters*/
		void loadVisualizerEditorParameters(XmlElement* xml) override;

		/** Creates an interface with additional channel settings*/
		Visualizer* createNewCanvas(void);

		/** Called by PopupChannelSelector */
		void channelStateChanged(Array<int> newChannels) override;

	private:

		OwnedArray<HeadstageOptionsInterface> headstageOptionsInterfaces;
		OwnedArray<ElectrodeButton> electrodeButtons;

		ScopedPointer<SampleRateInterface> sampleRateInterface;
		ScopedPointer<BandwidthInterface> bandwidthInterface;
		ScopedPointer<DSPInterface> dspInterface;

		ScopedPointer<AudioInterface> audioInterface;
		ScopedPointer<ClockDivideInterface> clockInterface;

		ScopedPointer<UtilityButton> rescanButton, dacTTLButton;
		ScopedPointer<UtilityButton> auxButton;
		ScopedPointer<UtilityButton> adcButton;
		ScopedPointer<UtilityButton> ledButton;

		ScopedPointer<UtilityButton> dspoffsetButton;
		ScopedPointer<ComboBox> ttlSettleCombo, dacHPFcombo;

		ScopedPointer<Label> audioLabel, ttlSettleLabel, dacHPFlabel;

		bool saveImpedances, measureWhenRecording;

		DeviceThread* board;
		ChannelCanvas* canvas;

		enum AudioChannel {
			LEFT = 0,
			RIGHT = 1
		};

		AudioChannel activeAudioChannel;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceEditor);

	};


	class HeadstageOptionsInterface : public Component,
									  public Button::Listener
	{
	public:

		/** Constructor*/
		HeadstageOptionsInterface(DeviceThread*, DeviceEditor*, int hsNum);
		~HeadstageOptionsInterface();

		void paint(Graphics& g);

		void buttonClicked(Button* button);

		void checkEnabledState();

	private:

		int hsNumber1, hsNumber2;
		int channelsOnHs1, channelsOnHs2;
		String name;

		bool isEnabled;

		DeviceThread* board;
		DeviceEditor* editor;

		ScopedPointer<UtilityButton> hsButton1;
		ScopedPointer<UtilityButton> hsButton2;

	};


	class BandwidthInterface : public Component,
		public Label::Listener
	{
	public:
		BandwidthInterface(DeviceThread*, DeviceEditor*);
		~BandwidthInterface();

		void paint(Graphics& g);
		void labelTextChanged(Label* te);

		void setLowerBandwidth(double value);
		void setUpperBandwidth(double value);
		double getLowerBandwidth();
		double getUpperBandwidth();

	private:

		String name;

		String lastLowCutString, lastHighCutString;

		DeviceThread* board;
		DeviceEditor* editor;

		ScopedPointer<Label> upperBandwidthSelection;
		ScopedPointer<Label> lowerBandwidthSelection;

		double actualUpperBandwidth;
		double actualLowerBandwidth;

	};

	class DSPInterface : public Component,
		public Label::Listener
	{
	public:
		DSPInterface(DeviceThread*, DeviceEditor*);
		~DSPInterface();

		void paint(Graphics& g);
		void labelTextChanged(Label* te);

		void setDspCutoffFreq(double value);
		double getDspCutoffFreq();

	private:

		String name;

		DeviceThread* board;
		DeviceEditor* editor;

		ScopedPointer<Label> dspOffsetSelection;

		double actualDspCutoffFreq;

	};



	class SampleRateInterface : public Component,
		public ComboBox::Listener
	{
	public:
		SampleRateInterface(DeviceThread*, DeviceEditor*);
		~SampleRateInterface();

		int getSelectedId();
		void setSelectedId(int);

		String getText();

		void paint(Graphics& g);
		void comboBoxChanged(ComboBox* cb);

	private:

		int sampleRate;
		String name;

		DeviceThread* board;
		DeviceEditor* editor;

		ScopedPointer<ComboBox> rateSelection;
		StringArray sampleRateOptions;

	};

	class AudioInterface : public Component,
		public Label::Listener
	{
	public:
		AudioInterface(DeviceThread*, DeviceEditor*);
		~AudioInterface();

		void paint(Graphics& g);
		void labelTextChanged(Label* te);

		void setNoiseSlicerLevel(int value);
		int getNoiseSlicerLevel();

	private:

		String name;

		String lastNoiseSlicerString;
		String lastGainString;

		DeviceThread* board;
		DeviceEditor* editor;

		ScopedPointer<Label> noiseSlicerLevelSelection;

		int actualNoiseSlicerLevel;

	};

	class ClockDivideInterface : public Component,
		public Label::Listener
	{
	public:
		ClockDivideInterface(DeviceThread*, DeviceEditor*);

		void paint(Graphics& g);
		void labelTextChanged(Label* te);

		void setClockDivideRatio(int value);
		int getClockDivideRatio() const { return actualDivideRatio; };

	private:

		String name;
		String lastDivideRatioString;

		DeviceThread * board;
		DeviceEditor * editor;

		ScopedPointer<Label> divideRatioSelection;
		int actualDivideRatio;

	};

}
#endif  // __DEVICEEDITOR_H_2AD3C591__
