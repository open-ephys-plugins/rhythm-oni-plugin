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

#include "ImpedanceMeter.h"

using namespace ONIRhythmNode;

#define PI  3.14159265359
#define TWO_PI  6.28318530718
#define DEGREES_TO_RADIANS  0.0174532925199
#define RADIANS_TO_DEGREES  57.2957795132

// Allocates memory for a 3-D array of doubles.
void allocateDoubleArray3D(std::vector<std::vector<std::vector<double> > >& array3D,
    int xSize, int ySize, int zSize)
{
    int i, j;

    if (xSize == 0) return;
    array3D.resize(xSize);
    for (i = 0; i < xSize; ++i)
    {
        array3D[i].resize(ySize);
        for (j = 0; j < ySize; ++j)
        {
            array3D[i][j].resize(zSize);
        }
    }
}

ImpedanceMeter::ImpedanceMeter(DeviceThread* board_) : 
    ThreadWithProgressWindow(
        "RHD2000 Impedance Measurement",
        true,
        true),
    board(board_)
{
    // to perform electrode impedance measurements at very low frequencies.
    const int maxNumBlocks = 120;
    int numStreams = 8;

    allocateDoubleArray3D(amplifierPreFilter, numStreams, 32, MAX_SAMPLES_PER_DATA_BLOCK * maxNumBlocks);
}

ImpedanceMeter::~ImpedanceMeter()
{
    stopThreadSafely();
}

void ImpedanceMeter::stopThreadSafely()
{
    if (isThreadRunning())
    {
        CoreServices::sendStatusMessage("Impedance measurement in progress. Stopping it.");

        if (!stopThread(3000)) //wait three seconds max for it to exit gracefully
        {
            std::cerr << "ERROR: Impedance measurement did not exit." << std::endl;
            {
                const ScopedLock lock(board->oniLock);
                board->evalBoard->stop();
            }
        }
    }
}

void ImpedanceMeter::waitSafely()
{
    if (!waitForThreadToExit(120000)) //two minutes should be enough for completing a scan
    {
        CoreServices::sendStatusMessage("Impedance measurement took too much time. Aborting.");

        if (!stopThread(3000)) //wait three seconds max for it to exit gracefully
        {
            std::cerr << "ERROR: Impedance measurement thread did not exit." << std::endl;
            {
                const ScopedLock lock(board->oniLock);
                board->evalBoard->stop();
            }
        }
    }
}

float ImpedanceMeter::updateImpedanceFrequency(float desiredImpedanceFreq, bool& impedanceFreqValid)
{
    int impedancePeriod;
    double lowerBandwidthLimit, upperBandwidthLimit;
    float actualImpedanceFreq;

    upperBandwidthLimit = board->settings.dsp.upperBandwidth / 1.5;
    lowerBandwidthLimit = board->settings.dsp.lowerBandwidth * 1.5;

    if (board->settings.dsp.enabled)
    {
        if (board->settings.dsp.cutoffFreq > board->settings.dsp.lowerBandwidth)
        {
            lowerBandwidthLimit = board->settings.dsp.cutoffFreq * 1.5;
        }
    }

    if (desiredImpedanceFreq > 0.0)
    {
        impedancePeriod = (board->settings.boardSampleRate / desiredImpedanceFreq);
        if (impedancePeriod >= 4 && impedancePeriod <= 1024 &&
            desiredImpedanceFreq >= lowerBandwidthLimit &&
            desiredImpedanceFreq <= upperBandwidthLimit)
        {
            actualImpedanceFreq = board->settings.boardSampleRate / impedancePeriod;
            impedanceFreqValid = true;
        }
        else
        {
            actualImpedanceFreq = 0.0;
            impedanceFreqValid = false;
        }
    }
    else
    {
        actualImpedanceFreq = 0.0;
        impedanceFreqValid = false;
    }

    return actualImpedanceFreq;
}


int ImpedanceMeter::loadAmplifierData(std::queue<Rhd2000DataBlock>& dataQueue,
    int numBlocks, int numDataStreams)
{

    int block, t, channel, stream;
    int indexAmp = 0;

    for (block = 0; block < numBlocks; ++block)
    {

        // Load and scale RHD2000 amplifier waveforms
        // (sampled at amplifier sampling rate)
        for (t = 0; t < SAMPLES_PER_DATA_BLOCK(board->evalBoard->isUSB3()); ++t)
        {
            for (channel = 0; channel < 32; ++channel)
            {
                for (stream = 0; stream < numDataStreams; ++stream)
                {
                    // Amplifier waveform units = microvolts
                    amplifierPreFilter[stream][channel][indexAmp] = 0.195 *
                        (dataQueue.front().amplifierData[stream][channel][t] - 32768);
                }
            }
            ++indexAmp;
        }
        // We are done with this Rhd2000DataBlock object; remove it from dataQueue
        dataQueue.pop();
    }

    return 0;
}


void ImpedanceMeter::measureComplexAmplitude(
    std::vector<std::vector<std::vector<double>>>& measuredMagnitude,
    std::vector<std::vector<std::vector<double>>>& measuredPhase,
    int capIndex, 
    int stream, 
    int chipChannel, 
    int numBlocks,
    double sampleRate, 
    double frequency, 
    int numPeriods)
{
    int period = (sampleRate / frequency);
    int startIndex = 0;
    int endIndex = startIndex + numPeriods * period - 1;

    // Move the measurement window to the end of the waveform to ignore start-up transient.
    while (endIndex < SAMPLES_PER_DATA_BLOCK(board->evalBoard->isUSB3()) * numBlocks - period)
    {
        startIndex += period;
        endIndex += period;
    }

    double iComponent, qComponent;

    // Measure real (iComponent) and imaginary (qComponent) amplitude of frequency component.
    amplitudeOfFreqComponent(iComponent, qComponent, amplifierPreFilter[stream][chipChannel],
        startIndex, endIndex, sampleRate, frequency);
    // Calculate magnitude and phase from real (I) and imaginary (Q) components.
    measuredMagnitude[stream][chipChannel][capIndex] =
        sqrt(iComponent * iComponent + qComponent * qComponent);
    measuredPhase[stream][chipChannel][capIndex] =
        RADIANS_TO_DEGREES *atan2(qComponent, iComponent);
}


void ImpedanceMeter::amplitudeOfFreqComponent(
    double& realComponent, 
    double& imagComponent,
    const std::vector<double>& data, 
    int startIndex,
    int endIndex, 
    double sampleRate, 
    double frequency)
{
    int length = endIndex - startIndex + 1;
    const double k = TWO_PI * frequency / sampleRate;  // precalculate for speed

    // Perform correlation with sine and cosine waveforms.
    double meanI = 0.0;
    double meanQ = 0.0;
    for (int t = startIndex; t <= endIndex; ++t)
    {
        meanI += data.at(t) * cos(k * t);
        meanQ += data.at(t) * -1.0 * sin(k * t);
    }
    meanI /= (double)length;
    meanQ /= (double)length;

    realComponent = 2.0 * meanI;
    imagComponent = 2.0 * meanQ;
}


void ImpedanceMeter::factorOutParallelCapacitance(double& impedanceMagnitude, double& impedancePhase,
    double frequency, double parasiticCapacitance)
{
    // First, convert from polar coordinates to rectangular coordinates.
    double measuredR = impedanceMagnitude * cos(DEGREES_TO_RADIANS * impedancePhase);
    double measuredX = impedanceMagnitude * sin(DEGREES_TO_RADIANS * impedancePhase);

    double capTerm = TWO_PI * frequency * parasiticCapacitance;
    double xTerm = capTerm * (measuredR * measuredR + measuredX * measuredX);
    double denominator = capTerm * xTerm + 2 * capTerm * measuredX + 1;
    double trueR = measuredR / denominator;
    double trueX = (measuredX + xTerm) / denominator;

    // Now, convert from rectangular coordinates back to polar coordinates.
    impedanceMagnitude = sqrt(trueR * trueR + trueX * trueX);
    impedancePhase = RADIANS_TO_DEGREES * atan2(trueX, trueR);
}

void ImpedanceMeter::empiricalResistanceCorrection(double& impedanceMagnitude, double& impedancePhase,
    double boardSampleRate)
{
    // First, convert from polar coordinates to rectangular coordinates.
    double impedanceR = impedanceMagnitude * cos(DEGREES_TO_RADIANS * impedancePhase);
    double impedanceX = impedanceMagnitude * sin(DEGREES_TO_RADIANS * impedancePhase);

    // Emprically derived correction factor (i.e., no physical basis for this equation).
    impedanceR /= 10.0 * exp(-boardSampleRate / 2500.0) * cos(TWO_PI * boardSampleRate / 15000.0) + 1.0;

    // Now, convert from rectangular coordinates back to polar coordinates.
    impedanceMagnitude = sqrt(impedanceR * impedanceR + impedanceX * impedanceX);
    impedancePhase = RADIANS_TO_DEGREES * atan2(impedanceX, impedanceR);
}


void ImpedanceMeter::run()
{
    LOGD("Running imedance measurement");
    runImpedanceMeasurement(board->impedances);
    
    LOGD("Restoring board settings");
    restoreBoardSettings();

    LOGD("Impedance measurement finished");
    board->impedanceMeasurementFinished();

    setProgress(1.0f);

}

#define CHECK_EXIT if (threadShouldExit()) return

void ImpedanceMeter::runImpedanceMeasurement(Impedances& impedances)
{
    int commandSequenceLength, stream, channel, capRange;
    double cSeries;
    std::vector<int> commandList;

    setProgress(0.0f);

    int numdataStreams = board->evalBoard->getNumEnabledDataStreams();
	LOGD("ImpedanceMeter: Num enabled streams = ", numdataStreams);

    bool rhd2164ChipPresent = false;
    int chOffset;

    Array<int> enabledStreams;

    for (stream = 0; stream < board->MAX_NUM_DATA_STREAMS; ++stream)
    {
        CHECK_EXIT;

        if (board->evalBoard->isStreamEnabled(stream))
        {
            enabledStreams.add(stream);
        }

        if (board->chipId[stream] == CHIP_ID_RHD2164_B)
        {
            rhd2164ChipPresent = true;
        }
    }

    bool validImpedanceFreq;
    LOGD("ImpedanceMeter: Updating impedance frequency to 1000");
    float actualImpedanceFreq = updateImpedanceFrequency(1000.0, validImpedanceFreq);

    if (!validImpedanceFreq)
    {
        LOGD("Invalid frqeuency");
        return;
    }
    
    // Create a command list for the AuxCmd1 slot.
    commandSequenceLength = board->chipRegisters.createCommandListZcheckDac(commandList, actualImpedanceFreq, 128.0);
    CHECK_EXIT;
    LOGD("ImpedanceMeter: Updating command list");
    board->evalBoard->uploadCommandList(commandList, Rhd2000ONIBoard::AuxCmd1, 1);
    board->evalBoard->selectAuxCommandLength(Rhd2000ONIBoard::AuxCmd1,
        0, commandSequenceLength - 1);

    if (board->settings.fastTTLSettleEnabled)
    {
        board->evalBoard->enableExternalFastSettle(false);
    }

    CHECK_EXIT;
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortA,
        Rhd2000ONIBoard::AuxCmd1, 1);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortB,
        Rhd2000ONIBoard::AuxCmd1, 1);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortC,
        Rhd2000ONIBoard::AuxCmd1, 1);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortD,
        Rhd2000ONIBoard::AuxCmd1, 1);

    // Select number of periods to measure impedance over
    int numPeriods = (0.020 * actualImpedanceFreq); // Test each channel for at least 20 msec...
    if (numPeriods < 5) numPeriods = 5; // ...but always measure across no fewer than 5 complete periods
    double period = board->settings.boardSampleRate / actualImpedanceFreq;
    int numBlocks = ceil((numPeriods + 2.0) * period / 60.0);  // + 2 periods to give time to settle initially
    if (numBlocks < 2) numBlocks = 2;   // need first block for command to switch channels to take effect.

    CHECK_EXIT;
    board->settings.dsp.cutoffFreq = board->chipRegisters.setDspCutoffFreq(board->settings.dsp.cutoffFreq);
    board->settings.dsp.lowerBandwidth = board->chipRegisters.setLowerBandwidth(board->settings.dsp.lowerBandwidth);
    board->settings.dsp.upperBandwidth = board->chipRegisters.setUpperBandwidth(board->settings.dsp.upperBandwidth);
    board->chipRegisters.enableDsp(board->settings.dsp.enabled);
    board->chipRegisters.enableZcheck(true);
    LOGD("ImpedanceMeter: Z check enabled");
    
    commandSequenceLength = board->chipRegisters.createCommandListRegisterConfig(commandList, false);
    CHECK_EXIT;
    // Upload version with no ADC calibration to AuxCmd3 RAM Bank 1.
    board->evalBoard->uploadCommandList(commandList, Rhd2000ONIBoard::AuxCmd3, 3);
    board->evalBoard->selectAuxCommandLength(Rhd2000ONIBoard::AuxCmd3, 0, commandSequenceLength - 1);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortA, Rhd2000ONIBoard::AuxCmd3, 3);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortB, Rhd2000ONIBoard::AuxCmd3, 3);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortC, Rhd2000ONIBoard::AuxCmd3, 3);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortD, Rhd2000ONIBoard::AuxCmd3, 3);

    CHECK_EXIT;
    board->evalBoard->setContinuousRunMode(false);

    board->evalBoard->setMaxTimeStep(128*SAMPLES_PER_DATA_BLOCK(board->evalBoard->isUSB3()) * numBlocks);

    // Create matrices of doubles of size (numStreams x 32 x 3) to store complex amplitudes
    // of all amplifier channels (32 on each data stream) at three different Cseries values.
    std::vector<std::vector<std::vector<double>>>  measuredMagnitude;
    std::vector<std::vector<std::vector<double>>>  measuredPhase;

    measuredMagnitude.resize(board->evalBoard->getNumEnabledDataStreams());
    measuredPhase.resize(board->evalBoard->getNumEnabledDataStreams());

    for (int i = 0; i < board->evalBoard->getNumEnabledDataStreams(); ++i)
    {
        measuredMagnitude[i].resize(32);
        measuredPhase[i].resize(32);

        for (int j = 0; j < 32; ++j)
        {
            measuredMagnitude[i][j].resize(3);
            measuredPhase[i][j].resize(3);
        }
    }

    double distance, minDistance, current, Cseries;
    double impedanceMagnitude, impedancePhase;

    const double bestAmplitude = 250.0;  // we favor voltage readings that are closest to 250 uV: not too large,
    // and not too small.
    const double dacVoltageAmplitude = 128 * (1.225 / 256);  // this assumes the DAC amplitude was set to 128
    const double parasiticCapacitance = 14.0e-12;  // 14 pF: an estimate of on-chip parasitic capacitance,
    // including 10 pF of amplifier input capacitance.
    double relativeFreq = actualImpedanceFreq / board->settings.boardSampleRate;

    int bestAmplitudeIndex;

    // We execute three complete electrode impedance measurements: one each with
    // Cseries set to 0.1 pF, 1 pF, and 10 pF.  Then we select the best measurement
    // for each channel so that we achieve a wide impedance measurement range.
    for (capRange = 0; capRange < 3; ++capRange)
    {
        //LOGD("ImpedanceMeter: capRange = ", capRange);
        
        switch (capRange)
        {
        case 0:
            board->chipRegisters.setZcheckScale(Rhd2000Registers::ZcheckCs100fF);
            cSeries = 0.1e-12;
            break;
        case 1:
            board->chipRegisters.setZcheckScale(Rhd2000Registers::ZcheckCs1pF);
            cSeries = 1.0e-12;
            break;
        case 2:
            board->chipRegisters.setZcheckScale(Rhd2000Registers::ZcheckCs10pF);
            cSeries = 10.0e-12;
            break;
        }

        // Check all 32 channels across all active data streams.
        for (channel = 0; channel < 32; ++channel)
        {

            CHECK_EXIT;

            //LOGD("ImpedanceMeter: channel = ", channel);
   
            board->chipRegisters.setZcheckChannel(channel);
            commandSequenceLength =
                board->chipRegisters.createCommandListRegisterConfig(commandList, false);
            // Upload version with no ADC calibration to AuxCmd3 RAM Bank 1.
            board->evalBoard->uploadCommandList(commandList, Rhd2000ONIBoard::AuxCmd3, 3);

            //LOGD("ImpedanceMeter: uploaded command list");
            
            board->evalBoard->run();

            std::queue<Rhd2000DataBlock> dataQueue;
            board->evalBoard->readDataBlocks(numBlocks, dataQueue);
            {
                const ScopedLock lock(board->oniLock);
                board->evalBoard->stop();
            }

            loadAmplifierData(dataQueue, numBlocks, numdataStreams);
            //LOGD("ImpedanceMeter: loaded amplifier data");
            
            for (stream = 0; stream < numdataStreams; ++stream)
            {
                //LOGD("ImpedanceMeter: stream = ", stream);
                setProgress(float(capRange) / 3.0f 
                            + (float(channel) / 32.0f / 3.0f) 
                            + (float(stream) / float(numdataStreams) / 32.0f / 3.0f));

                if (board->chipId[stream] != CHIP_ID_RHD2164_B)
                {
                    measureComplexAmplitude(measuredMagnitude, measuredPhase,
                        capRange, stream, channel, numBlocks, board->settings.boardSampleRate,
                        actualImpedanceFreq, numPeriods);
                }
            }

            // If an RHD2164 chip is plugged in, we have to set the Zcheck select register to channels 32-63
            // and repeat the previous steps.
            if (rhd2164ChipPresent)
            {
                CHECK_EXIT;
                board->chipRegisters.setZcheckChannel(channel + 32); // address channels 32-63
                commandSequenceLength =
                    board->chipRegisters.createCommandListRegisterConfig(commandList, false);
                // Upload version with no ADC calibration to AuxCmd3 RAM Bank 1.
                board->evalBoard->uploadCommandList(commandList, Rhd2000ONIBoard::AuxCmd3, 3);

                board->evalBoard->run();
              /*  while (board->evalBoard->isRunning())
                {

                }*/
                board->evalBoard->readDataBlocks(numBlocks, dataQueue);
                {
                    const ScopedLock lock(board->oniLock);
                    board->evalBoard->stop();
                }
                loadAmplifierData(dataQueue, numBlocks, numdataStreams);

                for (stream = 0; stream < board->evalBoard->getNumEnabledDataStreams(); ++stream)
                {
                    if (board->chipId[stream] == CHIP_ID_RHD2164_B)
                    {
                        measureComplexAmplitude(measuredMagnitude, measuredPhase,
                            capRange, stream, channel, numBlocks, board->settings.boardSampleRate,
                            actualImpedanceFreq, numPeriods);
                    }
                }
            }
        }
    }

    impedances.streams.clear();
    impedances.channels.clear();
    impedances.magnitudes.clear();
    impedances.phases.clear();

    for (stream = 0; stream < board->evalBoard->getNumEnabledDataStreams(); ++stream)
    {
        if ((board->chipId[stream] == CHIP_ID_RHD2132) && (board->numChannelsPerDataStream[stream] == 16))
            chOffset = RHD2132_16CH_OFFSET;
        else
            chOffset = 0;

        for (channel = 0; channel < board->numChannelsPerDataStream[stream]; ++channel)
        {
            if (1)
            {
                minDistance = 9.9e99;  // ridiculously large number
                for (capRange = 0; capRange < 3; ++capRange)
                {
                    // Find the measured amplitude that is closest to bestAmplitude on a logarithmic scale
                    distance = abs(log(measuredMagnitude[stream][channel+chOffset][capRange] / bestAmplitude));
                    if (distance < minDistance)
                    {
                        bestAmplitudeIndex = capRange;
                        minDistance = distance;
                    }
                }
                switch (bestAmplitudeIndex)
                {
                case 0:
                    Cseries = 0.1e-12;
                    break;
                case 1:
                    Cseries = 1.0e-12;
                    break;
                case 2:
                    Cseries = 10.0e-12;
                    break;
                }

                // Calculate current amplitude produced by on-chip voltage DAC
                current = TWO_PI * actualImpedanceFreq * dacVoltageAmplitude * Cseries;

                // Calculate impedance magnitude from calculated current and measured voltage.
                impedanceMagnitude = 1.0e-6 * (measuredMagnitude[stream][channel + chOffset][bestAmplitudeIndex] / current) *
                    (18.0 * relativeFreq * relativeFreq + 1.0);

                // Calculate impedance phase, with small correction factor accounting for the
                // 3-command SPI pipeline delay.
                impedancePhase = measuredPhase[stream][channel + chOffset][bestAmplitudeIndex] + (360.0 * (3.0 / period));

                // Factor out on-chip parasitic capacitance from impedance measurement.
                factorOutParallelCapacitance(impedanceMagnitude, impedancePhase, actualImpedanceFreq,
                    parasiticCapacitance);

                // Perform empirical resistance correction to improve accuarcy at sample rates below 15 kS/s.
                empiricalResistanceCorrection(impedanceMagnitude, impedancePhase,
                    board->settings.boardSampleRate);

                impedances.streams.add(enabledStreams[stream]);
                impedances.channels.add(channel + chOffset);
                impedances.magnitudes.add(impedanceMagnitude);
                impedances.phases.add(impedancePhase);

                //if (impedanceMagnitude > 1000000)
                //    cout << "stream " << stream << " channel " << 1 + channel << " magnitude: " << String(impedanceMagnitude / 1e6, 2) << " MOhm , phase : " << impedancePhase << endl;
                //else
                //    cout << "stream " << stream << " channel " << 1 + channel << " magnitude: " << String(impedanceMagnitude / 1e3, 2) << " kOhm , phase : " << impedancePhase << endl;

            }
        }
    }
    
    impedances.valid = true;

}

void ImpedanceMeter::restoreBoardSettings()
{
    board->evalBoard->setContinuousRunMode(false);
    board->evalBoard->setMaxTimeStep(0);

    // Switch back to flatline
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortA, Rhd2000ONIBoard::AuxCmd1, 0);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortB, Rhd2000ONIBoard::AuxCmd1, 0);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortC, Rhd2000ONIBoard::AuxCmd1, 0);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortD, Rhd2000ONIBoard::AuxCmd1, 0);

    
    board->evalBoard->selectAuxCommandLength(Rhd2000ONIBoard::AuxCmd1, 0, 1);

    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortA, Rhd2000ONIBoard::AuxCmd3,
        board->settings.fastSettleEnabled ? 2 : 1);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortB, Rhd2000ONIBoard::AuxCmd3,
        board->settings.fastSettleEnabled ? 2 : 1);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortC, Rhd2000ONIBoard::AuxCmd3,
        board->settings.fastSettleEnabled ? 2 : 1);
    board->evalBoard->selectAuxCommandBank(Rhd2000ONIBoard::PortD, Rhd2000ONIBoard::AuxCmd3,
        board->settings.fastSettleEnabled ? 2 : 1);


    if (board->settings.fastTTLSettleEnabled)
    {
        board->evalBoard->enableExternalFastSettle(true);
    }
}

