#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // Initialize variables
    currentImageIndex = 0;
    playbackSpeed = 1.0;
    lastImageTime = 0;
    isPlaying = false;
    showBlackScreen = false;
    rangeStart = 0;
    rangeEnd = 0;
    lastCheckTime = 0;
    checkInterval = 1.0; // Check every second
    playDirection = FORWARD;
    loopMode = LOOP;
    
    // Initialize Syphon parameters
    syphonWidth = 1920;
    syphonHeight = 1080;
    maintainAspectRatio = true;
    
    // Setup Syphon and FBO
    syphonFbo.allocate(syphonWidth, syphonHeight, GL_RGBA);
    syphonServer.setName("Frame Player Output");
    
    // Setup UI layout with fixed width
    uiPanel = ofRectangle(0, 0, UI_PANEL_WIDTH, ofGetHeight());
    previewPanel = ofRectangle(UI_PANEL_WIDTH, 0, ofGetWidth() - UI_PANEL_WIDTH, ofGetHeight());
    
    // Setup DatGui
    gui = new ofxDatGui(10, 10);
    gui->setWidth(UI_PANEL_WIDTH - 20);
    
    // Add controls
    playButton = gui->addButton("Play");
    playButton->onButtonEvent(this, &ofApp::onPlayButtonEvent);
    playButton->setBackgroundColor(ofColor(0, 200, 0)); // Initial green color for paused
    
    speedSlider = gui->addSlider("Speed", 0.0f, MAX_SPEED, DEFAULT_SPEED);
    speedSlider->onSliderEvent(this, &ofApp::onSpeedSliderEvent);
    
    // Add speed buttons
    vector<float> speeds = {0.2, 0.5, 1.0, 2.0};
    vector<string> labels = {"0.2x", "0.5x", "1x", "2x"};
    for(int i = 0; i < speeds.size(); i++) {
        ofxDatGuiButton* button = gui->addButton(labels[i]);
        button->setWidth(UI_PANEL_WIDTH/4 - 5);
        button->onButtonEvent(this, &ofApp::onSpeedButtonEvent);
        speedButtons.push_back(button);
    }
    
    // Add playback direction controls
    gui->addLabel("Playback Direction");
    directionForwardButton = gui->addToggle("Forward");
    directionForwardButton->setChecked(playDirection == FORWARD);
    directionForwardButton->onToggleEvent(this, &ofApp::onDirectionForwardEvent);
    
    directionBackwardButton = gui->addToggle("Backward");
    directionBackwardButton->setChecked(playDirection == BACKWARD);
    directionBackwardButton->onToggleEvent(this, &ofApp::onDirectionBackwardEvent);
    
    // Add loop mode controls
    gui->addLabel("Loop Mode");
    loopModeButton = gui->addToggle("Loop");
    loopModeButton->setChecked(loopMode == LOOP);
    loopModeButton->onToggleEvent(this, &ofApp::onLoopModeEvent);
    
    pingPongModeButton = gui->addToggle("Ping Pong");
    pingPongModeButton->setChecked(loopMode == PING_PONG);
    pingPongModeButton->onToggleEvent(this, &ofApp::onPingPongModeEvent);
    
    // Add scrubber bar instead of empty line
    scrubberBar = gui->addSlider("", 0, 1, 0);
    scrubberBar->setLabel("Scrub");
    scrubberBar->onSliderEvent(this, &ofApp::onScrubberEvent);
    
    startFrameSlider = gui->addSlider("Start Frame", 1, 1, 1);
    startFrameSlider->onSliderEvent(this, &ofApp::onStartFrameEvent);
    startFrameSlider->setPrecision(0);
    
    endFrameSlider = gui->addSlider("End Frame", 1, 1, 1);
    endFrameSlider->onSliderEvent(this, &ofApp::onEndFrameEvent);
    endFrameSlider->setPrecision(0);
    
    currentFrameLabel = gui->addLabel("Frame: 0/0");
    
    blackScreenToggle = gui->addToggle("Black Screen");
    blackScreenToggle->onToggleEvent(this, &ofApp::onBlackScreenToggleEvent);
    
    // Add Syphon controls
    gui->addLabel("Syphon Settings");
    syphonWidthInput = gui->addTextInput("Width", "1920");
    syphonWidthInput->setInputType(ofxDatGuiInputType::NUMERIC);
    syphonWidthInput->onTextInputEvent(this, &ofApp::onSyphonWidthEvent);
    
    syphonHeightInput = gui->addTextInput("Height", "1080");
    syphonHeightInput->setInputType(ofxDatGuiInputType::NUMERIC);
    syphonHeightInput->onTextInputEvent(this, &ofApp::onSyphonHeightEvent);
    
    aspectRatioToggle = gui->addToggle("Maintain Aspect Ratio");
    aspectRatioToggle->setChecked(maintainAspectRatio);
    aspectRatioToggle->onToggleEvent(this, &ofApp::onAspectRatioEvent);
    
    applySyphonSizeButton = gui->addButton("Apply Size");
    applySyphonSizeButton->onButtonEvent(this, &ofApp::onApplySyphonSizeEvent);
    
    // Set initial display path
    displayPath = "Drop folder here or click Open";
}

//--------------------------------------------------------------
void ofApp::update(){
    // Add directory watching
    float currentTime = ofGetElapsedTimef();
    if (!directoryPath.empty() && currentTime - lastCheckTime >= checkInterval) {
        imageDir.listDir(directoryPath);
        imageDir.allowExt("jpg");
        imageDir.allowExt("png");
        imageDir.allowExt("tif");
        imageDir.allowExt("tiff");
        
        // If number of files changed, reload the directory
        if (imageDir.size() != imagePaths.size()) {
            loadImagesFromDirectory(directoryPath);
        }
        lastCheckTime = currentTime;
    }

    // Update GUI
    gui->update();

    if (isPlaying && !showBlackScreen && !imagePaths.empty() && speedSlider->getValue() > 0.0f) {
        float frameTime = 1.0f / (BASE_FPS * convertSliderToSpeed(speedSlider->getValue()));
        currentTime = ofGetElapsedTimef();
        
        if (currentTime - lastImageTime >= frameTime) {
            // Update frame index based on playback direction
            if (playDirection == FORWARD) {
                currentImageIndex++;
                
                // Handle reaching the end based on loop mode
                if (currentImageIndex > rangeEnd) {
                    if (loopMode == LOOP) {
                        currentImageIndex = rangeStart;
                    } else if (loopMode == PING_PONG) {
                        currentImageIndex = rangeEnd - 1;
                        if (currentImageIndex < rangeStart) currentImageIndex = rangeStart;
                        playDirection = BACKWARD;
                        directionForwardButton->setChecked(false);
                        directionBackwardButton->setChecked(true);
                    }
                }
            } else { // BACKWARD
                currentImageIndex--;
                
                // Handle reaching the start based on loop mode
                if (currentImageIndex < rangeStart) {
                    if (loopMode == LOOP) {
                        currentImageIndex = rangeEnd;
                    } else if (loopMode == PING_PONG) {
                        currentImageIndex = rangeStart + 1;
                        if (currentImageIndex > rangeEnd) currentImageIndex = rangeEnd;
                        playDirection = FORWARD;
                        directionForwardButton->setChecked(true);
                        directionBackwardButton->setChecked(false);
                    }
                }
            }
            
            if (currentImageIndex >= 0 && currentImageIndex < imagePaths.size()) {
                currentImage.load(imagePaths[currentImageIndex]);
                updateFrameInfo();
            }
            lastImageTime = currentTime;
        }
    }

    // Update scrubber position when playing
    if (isPlaying && !showBlackScreen && !imagePaths.empty()) {
        // Update scrubber to reflect current position in the range
        float scrubberPos = 0;
        if (rangeEnd > rangeStart) {
            scrubberPos = (float)(currentImageIndex - rangeStart) / (rangeEnd - rangeStart);
        }
        scrubberBar->setValue(scrubberPos);
    }
}

float ofApp::convertSliderToSpeed(float sliderValue) {
    if(sliderValue <= 0) return 0;
    
    // Convert slider value to speed using two different scales
    float normalizedSlider = sliderValue / MAX_SPEED;  // 0-1 range
    
    if(normalizedSlider <= SLIDER_MIDPOINT) {
        // First 2/3 of slider: 0.0 - 1.0 speed
        return (normalizedSlider / SLIDER_MIDPOINT) * 1.0f;
    } else {
        // Last 1/3 of slider: 1.0 - 4.0 speed
        float remaining = (normalizedSlider - SLIDER_MIDPOINT) / (1.0f - SLIDER_MIDPOINT);
        return 1.0f + (remaining * 3.0f);  // Map to 1.0 - 4.0
    }
}

float ofApp::convertSpeedToSlider(float speed) {
    if(speed <= 0) return 0;
    
    if(speed <= 1.0f) {
        // Convert speeds 0.0 - 1.0 to first 2/3 of slider
        return (speed * SLIDER_MIDPOINT) * MAX_SPEED;
    } else {
        // Convert speeds 1.0 - 4.0 to last 1/3 of slider
        float normalized = (speed - 1.0f) / 3.0f;  // Convert to 0-1 range
        return (SLIDER_MIDPOINT + (normalized * (1.0f - SLIDER_MIDPOINT))) * MAX_SPEED;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    // Draw UI Panel background
    ofPushStyle();
    ofSetColor(40);
    ofDrawRectangle(uiPanel);
    ofPopStyle();
    
    // First render to FBO for Syphon output
    if (!showBlackScreen && currentImage.isAllocated()) {
        syphonFbo.begin();
        ofClear(0, 0, 0, 255);
        
        if (maintainAspectRatio) {
            // Calculate scaling to maintain aspect ratio
            float scale = min(syphonWidth / (float)currentImage.getWidth(),
                            syphonHeight / (float)currentImage.getHeight());
            
            float newWidth = currentImage.getWidth() * scale;
            float newHeight = currentImage.getHeight() * scale;
            
            // Center the image in the FBO
            float x = (syphonWidth - newWidth) / 2;
            float y = (syphonHeight - newHeight) / 2;
            
            currentImage.draw(x, y, newWidth, newHeight);
        } else {
            // Stretch to fill entire FBO
            currentImage.draw(0, 0, syphonWidth, syphonHeight);
        }
        syphonFbo.end();
        
        // Send FBO to Syphon
        syphonServer.publishTexture(&syphonFbo.getTexture());
    }
    
    // Draw preview in window
    if (!showBlackScreen && currentImage.isAllocated()) {
        float scale = min(previewPanel.width / (float)currentImage.getWidth(),
                         previewPanel.height / (float)currentImage.getHeight());
        
        float newWidth = currentImage.getWidth() * scale;
        float newHeight = currentImage.getHeight() * scale;
        
        float x = previewPanel.x + (previewPanel.width - newWidth) / 2;
        float y = previewPanel.y + (previewPanel.height - newHeight) / 2;
        
        currentImage.draw(x, y, newWidth, newHeight);
    }
    
    // Draw GUI
    gui->draw();
    
    // Draw drop zone
    ofPushStyle();
    ofSetColor(40);
    ofDrawRectRounded(ofRectangle(10, ofGetHeight() - 60, UI_PANEL_WIDTH - 20, 50), 5);
    
    // Draw path
    ofSetColor(180);
    string pathToShow = displayPath;
    // Truncate path if too long
    int maxChars = (UI_PANEL_WIDTH - 80) / 8; // Approximate characters that fit
    if(pathToShow.length() > maxChars) {
        pathToShow = "..." + pathToShow.substr(pathToShow.length() - (maxChars - 3));
    }
    ofDrawBitmapString(pathToShow, 20, ofGetHeight() - 30);
    
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::exit(){
    // Remove the syphonServer.close() call since it's not needed
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case ' ': {
            isPlaying = !isPlaying;
            playButton->setLabel(isPlaying ? "Pause" : "Play");
            // Update button color based on play state
            if(isPlaying) {
                playButton->setBackgroundColor(ofColor(255, 128, 0)); // Orange for playing
            } else {
                playButton->setBackgroundColor(ofColor(0, 200, 0)); // Green for paused
            }
            break;
        }
        case 'b': {
            showBlackScreen = !showBlackScreen;
            blackScreenToggle->setChecked(showBlackScreen);
            break;
        }
        case 'o': {
            ofFileDialogResult result = ofSystemLoadDialog("Select folder containing images", true);
            if (result.bSuccess) {
                folderSelected(result);
            }
            break;
        }
        case 'f': {
            // Toggle forward direction
            playDirection = FORWARD;
            directionForwardButton->setChecked(true);
            directionBackwardButton->setChecked(false);
            break;
        }
        case 'r': {
            // Toggle reverse direction
            playDirection = BACKWARD;
            directionForwardButton->setChecked(false);
            directionBackwardButton->setChecked(true);
            break;
        }
        case 'l': {
            // Toggle loop mode
            loopMode = LOOP;
            loopModeButton->setChecked(true);
            pingPongModeButton->setChecked(false);
            break;
        }
        case 'p': {
            // Toggle ping pong mode
            loopMode = PING_PONG;
            loopModeButton->setChecked(false);
            pingPongModeButton->setChecked(true);
            break;
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    // Check if we're scrubbing
    if (isScrubbing) {
        // Calculate position within the scrubber bar
        float barX = scrubberBar->getX();
        float barY = scrubberBar->getY();
        float barWidth = scrubberBar->getWidth();
        float barHeight = scrubberBar->getHeight();
        
        if (x >= barX && x <= barX + barWidth) {
            float percentage = (x - barX) / barWidth;
            percentage = ofClamp(percentage, 0.0f, 1.0f);
            
            // Calculate frame index based on percentage
            int frameIndex = rangeStart + round(percentage * (rangeEnd - rangeStart));
            frameIndex = ofClamp(frameIndex, rangeStart, rangeEnd);
            
            // Update current frame
            if (frameIndex != currentImageIndex && frameIndex < imagePaths.size()) {
                currentImageIndex = frameIndex;
                currentImage.load(imagePaths[currentImageIndex]);
                updateFrameInfo();
            }
            
            // Update scrubber position
            scrubberBar->setValue(percentage);
        }
    } else if(y > ofGetHeight() - 60 && x < UI_PANEL_WIDTH) {
        ofFileDialogResult result = ofSystemLoadDialog("Select folder containing images", true);
        if(result.bSuccess) {
            folderSelected(result);
        }
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    // Check if click is within scrubber bar
    float barX = scrubberBar->getX();
    float barY = scrubberBar->getY();
    float barWidth = scrubberBar->getWidth();
    float barHeight = scrubberBar->getHeight();
    
    if (x >= barX && x <= barX + barWidth &&
        y >= barY && y <= barY + barHeight) {
        
        // Store current playback state
        prevPlayState = isPlaying;
        prevPlaySpeed = speedSlider->getValue();
        
        // Pause playback during scrubbing
        isPlaying = false;
        playButton->setLabel("Play");
        playButton->setBackgroundColor(ofColor(0, 200, 0));
        
        // Set scrubbing flag
        isScrubbing = true;
        
        // Initial scrub position
        float percentage = (x - barX) / barWidth;
        percentage = ofClamp(percentage, 0.0f, 1.0f);
        
        // Calculate frame index based on percentage
        int frameIndex = rangeStart + round(percentage * (rangeEnd - rangeStart));
        frameIndex = ofClamp(frameIndex, rangeStart, rangeEnd);
        
        // Update current frame
        if (frameIndex != currentImageIndex && frameIndex < imagePaths.size()) {
            currentImageIndex = frameIndex;
            currentImage.load(imagePaths[currentImageIndex]);
            updateFrameInfo();
        }
        
        // Update scrubber position
        scrubberBar->setValue(percentage);
    } else if(y > ofGetHeight() - 60 && x < UI_PANEL_WIDTH) {
        ofFileDialogResult result = ofSystemLoadDialog("Select folder containing images", true);
        if(result.bSuccess) {
            folderSelected(result);
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    // If we were scrubbing, restore previous playback state
    if (isScrubbing) {
        isScrubbing = false;
        
        // Restore previous playback state
        isPlaying = prevPlayState;
        speedSlider->setValue(prevPlaySpeed);
        
        // Update play button appearance
        playButton->setLabel(isPlaying ? "Pause" : "Play");
        if(isPlaying) {
            playButton->setBackgroundColor(ofColor(255, 128, 0)); // Orange for playing
        } else {
            playButton->setBackgroundColor(ofColor(0, 200, 0)); // Green for paused
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    // Update panel layouts - keep UI width fixed
    uiPanel.height = h;
    previewPanel.set(UI_PANEL_WIDTH, 0, w - UI_PANEL_WIDTH, h);
    
    // Update GUI position
    gui->setPosition(10, 10);
}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    if(dragInfo.files.size() > 0) {
        // Check if it's a directory
        ofDirectory dir(dragInfo.files[0]);
        if(dir.isDirectory()) {
            directoryPath = dragInfo.files[0];
            displayPath = directoryPath;
            loadImagesFromDirectory(directoryPath);
        }
    }
}

void ofApp::folderSelected(ofFileDialogResult result) {
    directoryPath = result.getPath();
    displayPath = directoryPath;
    loadImagesFromDirectory(directoryPath);
}

void ofApp::loadImagesFromDirectory(string path) {
    imagePaths.clear();
    imageDir.listDir(path);
    imageDir.allowExt("jpg");
    imageDir.allowExt("png");
    imageDir.allowExt("tif");
    imageDir.allowExt("tiff");
    
    for(int i = 0; i < imageDir.size(); i++) {
        imagePaths.push_back(imageDir.getPath(i));
    }
    
    if (!imagePaths.empty()) {
        // Set the maximum range for the sliders (1-based for display)
        int lastFrame = imagePaths.size();  // 1-based for display
        
        // Update slider ranges and values
        startFrameSlider->setMax(lastFrame);
        endFrameSlider->setMax(lastFrame);
        
        startFrameSlider->setValue(1);
        endFrameSlider->setValue(lastFrame);
        
        // Update internal range variables (0-based for program)
        rangeStart = 0;  // 0-based index for first frame
        rangeEnd = lastFrame - 1;  // 0-based index for last frame
        
        // Set current frame within the range
        currentImageIndex = ofClamp(currentImageIndex, rangeStart, rangeEnd);
        currentImage.load(imagePaths[currentImageIndex]);
        updateFrameInfo();
    }
}

void ofApp::updateImageRange() {
    // Convert from 1-based display to 0-based program indices
    rangeStart = startFrameSlider->getValue() - 1;
    rangeEnd = endFrameSlider->getValue() - 1;
    
    // Make sure we're within bounds (using 0-based indices)
    rangeStart = ofClamp(rangeStart, 0, imagePaths.size() - 1);
    rangeEnd = ofClamp(rangeEnd, rangeStart, imagePaths.size() - 1);
    
    // Update current index if it's out of range
    if (currentImageIndex < rangeStart || currentImageIndex > rangeEnd) {
        currentImageIndex = rangeStart;
        if (!imagePaths.empty()) {
            currentImage.load(imagePaths[currentImageIndex]);
        }
    }
}

void ofApp::updateFrameInfo() {
    // Display frame numbers as 1-based
    string info = ofToString(currentImageIndex + 1) + "/" + ofToString(imagePaths.size());
    currentFrameLabel->setLabel(info);
}

void ofApp::onPlayButtonEvent(ofxDatGuiButtonEvent e) {
    isPlaying = !isPlaying;
    playButton->setLabel(isPlaying ? "Pause" : "Play");
    
    // Set button color based on play state
    if(isPlaying) {
        playButton->setBackgroundColor(ofColor(255, 128, 0)); // Orange for playing
    } else {
        playButton->setBackgroundColor(ofColor(0, 200, 0)); // Green for paused
    }
    
    if(isPlaying && speedSlider->getValue() <= 0.0f) {
        speedSlider->setValue(1.0f);
    }
}

void ofApp::onSpeedSliderEvent(ofxDatGuiSliderEvent e) {
    float actualSpeed = convertSliderToSpeed(e.value);
    playbackSpeed = (actualSpeed > 0.0f) ? (1.0f / (BASE_FPS * actualSpeed)) : 0.0f;
    lastImageTime = ofGetElapsedTimef();
}

void ofApp::onSpeedButtonEvent(ofxDatGuiButtonEvent e) {
    // Find the button's speed value based on its label
    string label = e.target->getLabel();
    float value = 1.0f; // default
    if(label == ".2x") value = 0.2f;
    else if(label == ".5x") value = 0.5f;
    else if(label == "1x") value = 1.0f;
    else if(label == "2x") value = 2.0f;
    
    speedSlider->setValue(value);
}

void ofApp::onStartFrameEvent(ofxDatGuiSliderEvent e) {
    updateImageRange();
}

void ofApp::onEndFrameEvent(ofxDatGuiSliderEvent e) {
    updateImageRange();
}

void ofApp::onBlackScreenToggleEvent(ofxDatGuiToggleEvent e) {
    showBlackScreen = e.checked;
}

void ofApp::onSyphonWidthEvent(ofxDatGuiTextInputEvent e) {
    syphonWidth = ofToInt(e.text);
}

void ofApp::onSyphonHeightEvent(ofxDatGuiTextInputEvent e) {
    syphonHeight = ofToInt(e.text);
}

void ofApp::onAspectRatioEvent(ofxDatGuiToggleEvent e) {
    maintainAspectRatio = e.checked;
}

void ofApp::onApplySyphonSizeEvent(ofxDatGuiButtonEvent e) {
    syphonFbo.allocate(syphonWidth, syphonHeight, GL_RGBA);
}

void ofApp::onScrubberEvent(ofxDatGuiSliderEvent e) {
    if (!imagePaths.empty() && rangeEnd > rangeStart) {
        // Calculate frame index based on percentage
        int frameIndex = rangeStart + round(e.value * (rangeEnd - rangeStart));
        frameIndex = ofClamp(frameIndex, rangeStart, rangeEnd);
        
        // Update current frame
        if (frameIndex != currentImageIndex && frameIndex < imagePaths.size()) {
            currentImageIndex = frameIndex;
            currentImage.load(imagePaths[currentImageIndex]);
            updateFrameInfo();
        }
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    // Empty implementation - this is a required method in openFrameworks
}

// Add new event handlers for direction and loop mode controls
void ofApp::onDirectionForwardEvent(ofxDatGuiToggleEvent e) {
    if (e.checked) {
        playDirection = FORWARD;
        directionBackwardButton->setChecked(false);
    } else if (!directionBackwardButton->getChecked()) {
        // Don't allow both to be unchecked
        directionForwardButton->setChecked(true);
    }
}

void ofApp::onDirectionBackwardEvent(ofxDatGuiToggleEvent e) {
    if (e.checked) {
        playDirection = BACKWARD;
        directionForwardButton->setChecked(false);
    } else if (!directionForwardButton->getChecked()) {
        // Don't allow both to be unchecked
        directionBackwardButton->setChecked(true);
    }
}

void ofApp::onLoopModeEvent(ofxDatGuiToggleEvent e) {
    if (e.checked) {
        loopMode = LOOP;
        pingPongModeButton->setChecked(false);
    } else if (!pingPongModeButton->getChecked()) {
        // Don't allow both to be unchecked
        loopModeButton->setChecked(true);
    }
}

void ofApp::onPingPongModeEvent(ofxDatGuiToggleEvent e) {
    if (e.checked) {
        loopMode = PING_PONG;
        loopModeButton->setChecked(false);
    } else if (!loopModeButton->getChecked()) {
        // Don't allow both to be unchecked
        pingPongModeButton->setChecked(true);
    }
}
