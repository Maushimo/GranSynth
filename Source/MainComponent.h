#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public AudioAppComponent,
                        public ChangeListener,
                        public Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };
    
    void paint(Graphics& g) override;
    void resized() override;
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
    void changeListenerCallback(ChangeBroadcaster* source) override;
    void timerCallback() override;
    
    void paintIfNoFileIsLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds);
    void paintIfFileIsLoaded(Graphics& g, const Rectangle<int>& thumbnailBounds);
    
    void changeState(TransportState newState);
    void updateLoopState(bool shouldLoop);
    
    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    
    void loopButtonChanged();

private:
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    ToggleButton loopToggle;
    Label currentPositionLabel;
    
    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    TransportState state;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
