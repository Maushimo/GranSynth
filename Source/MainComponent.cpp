#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    :   state (Stopped),
        thumbnailCache(5),
        thumbnail(512, formatManager, thumbnailCache)
{
    addAndMakeVisible(&openButton);
    openButton.setButtonText("OPEN");
    openButton.onClick = [this]{ openButtonClicked(); };
    
    addAndMakeVisible(&playButton);
    playButton.setButtonText("PLAY");
    playButton.onClick = [this]{ playButtonClicked(); };
    playButton.setColour(TextButton::buttonColourId, Colours::green);
    playButton.setEnabled(false);
    
    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("STOP");
    stopButton.onClick = [this]{ stopButtonClicked(); };
    stopButton.setColour(TextButton::buttonColourId, Colours::red);
    stopButton.setEnabled(false);
    
    addAndMakeVisible(&loopToggle);
    loopToggle.setButtonText("LOOP");
    loopToggle.onClick = [this]{ loopButtonChanged(); };
    
    addAndMakeVisible(&currentPositionLabel);
    currentPositionLabel.setText("STOPPED", dontSendNotification);
    
    setSize (900, 600);
    
    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);
    thumbnail.addChangeListener(this);
    
    setAudioChannels(0, 2);
    startTimer(20);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    openButton.setBounds(10, 10, getWidth() - 20, 20);
    playButton.setBounds(10, 40, getWidth() - 20, 20);
    stopButton.setBounds(10, 70, getWidth() - 20, 20);
    loopToggle.setBounds(10, 100, getWidth() - 20, 20);
    currentPositionLabel.setBounds(10, 130, getWidth() - 20, 20);
}

void MainComponent::changeState(MainComponent::TransportState newState)
{
    if(state != newState)
    {
        state = newState;
        
        switch (state) {
            case Stopped:
                stopButton.setEnabled(false);
                playButton.setEnabled(true);
                transportSource.setPosition(0.0);
                break;
            case Starting:
                playButton.setEnabled(false);
                transportSource.start();
                break;
            case Playing:
                stopButton.setEnabled(true);
                break;
            case Stopping:
                transportSource.stop();
                break;
        }
    }
}

void MainComponent::paint(Graphics& g)
{
    Rectangle<int> thumbnailBounds(10, 160, getWidth() - 20, getHeight() - 180);
    
    if(thumbnail.getNumChannels() == 0)
    {
        paintIfNoFileIsLoaded(g, thumbnailBounds);
    }
    else
    {
        paintIfFileIsLoaded(g, thumbnailBounds);
    }
}

void MainComponent::paintIfNoFileIsLoaded(Graphics &g, const Rectangle<int> &thumbnailBounds)
{
    g.setColour(Colours::darkgrey);
    g.fillRect(thumbnailBounds);
    g.setColour(Colours::white);
    g.drawFittedText("NO FILE LOADED", thumbnailBounds, Justification::centred, 1.0f);
}

void MainComponent::paintIfFileIsLoaded(Graphics &g, const Rectangle<int> &thumbnailBounds)
{
    g.setColour(Colours::white);
    g.fillRect(thumbnailBounds);
    
    g.setColour(Colours::blueviolet);
    
    auto audioLength(thumbnail.getTotalLength());
    thumbnail.drawChannels(g,
                           thumbnailBounds,
                           0.0,     //start time
                           audioLength,      //end time
                           1.0f);       //vertical zoom
    
    g.setColour(Colours::green);
    auto audioPosition(transportSource.getCurrentPosition());
    auto drawPosition((audioPosition / audioLength) * thumbnailBounds.getWidth() + thumbnailBounds.getX());
    g.drawLine(drawPosition, thumbnailBounds.getY(), drawPosition, thumbnailBounds.getBottom(), 2.0f);
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if(readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
    transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if(source == &transportSource)
    {
        if(transportSource.isPlaying())
        {
            changeState(Playing);
        }
        else
        {
            changeState(Stopped);
        }
    }
    
    if(source == &thumbnail)
    {
        //paint function will be calling during the next screen operation
        repaint();
    }
}

void MainComponent::openButtonClicked()
{
    FileChooser chooser("Select a Wave file to play", {}, "*.wav");
    
    if(chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();
        auto* reader = formatManager.createReaderFor(file);
        
        if(reader != nullptr)
        {
            std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
            transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            playButton.setEnabled(true);
            thumbnail.setSource(new FileInputSource(file));
            readerSource.reset(newSource.release());
        }
    }
}

void MainComponent::playButtonClicked()
{
    updateLoopState(loopToggle.getToggleState());
    changeState(Starting);
}

void MainComponent::stopButtonClicked()
{
    changeState(Stopping);
}

void MainComponent::updateLoopState(bool shouldLoop)
{
    if(readerSource.get() != nullptr)
    {
        readerSource->setLooping(shouldLoop);
    }
}

void MainComponent::loopButtonChanged()
{
    updateLoopState(loopToggle.getToggleState());
}

void MainComponent::timerCallback()
{
    if(transportSource.isPlaying())
    {
        RelativeTime position(transportSource.getCurrentPosition());
        
        auto minutes = ((int)position.inMinutes()) % 60;
        auto seconds = ((int)position.inSeconds()) % 60;
        auto millis = ((int)position.inMilliseconds()) % 1000;
        
        auto positionString = String::formatted("%02d:%02d:%03d", minutes, seconds, millis);
        
        currentPositionLabel.setText(positionString, dontSendNotification);
    }
    else
    {
        currentPositionLabel.setText("STOPPED", dontSendNotification);
    }
    
    repaint();
}
