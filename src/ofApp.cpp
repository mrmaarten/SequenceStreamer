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
    
    speedSlider = gui->addSlider("Speed", 0.0f, MAX_SPEED, DEFAULT_SPEED);
    speedSlider->onSliderEvent(this, &ofApp::onSpeedSliderEvent);
    
    // Add speed buttons
    vector<float> speeds = {0.2, 0.5, 1.0, 2.0};
    vector<string> labels = {".2x", ".5x", "1x", "2x"};
    for(int i = 0; i < speeds.size(); i++) {
        ofxDatGuiButton* button = gui->addButton(labels[i]);
        button->setWidth(UI_PANEL_WIDTH/4 - 5);
        button->onButtonEvent(this, &ofApp::onSpeedButtonEvent);
        speedButtons.push_back(button);
    }
    
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
        dir.listDir(directoryPath);
        dir.allowExt("jpg");
        dir.allowExt("png");
        
        // If number of files changed, reload the directory
        if (dir.size() != imagePaths.size()) {
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
            currentImageIndex++;
            if (currentImageIndex > rangeEnd || currentImageIndex >= imagePaths.size()) {
                currentImageIndex = rangeStart;
            }
            if (currentImageIndex < imagePaths.size()) {
                currentImage.load(imagePaths[currentImageIndex]);
                updateFrameInfo();
            }
            lastImageTime = currentTime;
        }
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
        case ' ':
            isPlaying = !isPlaying;
            playButton->setLabel(isPlaying ? "Pause" : "Play");
            break;
        case 'b':
            showBlackScreen = !showBlackScreen;
            blackScreenToggle->setChecked(showBlackScreen);
            break;
        case 'o':
            ofFileDialogResult result = ofSystemLoadDialog("Select folder containing images", true);
            if (result.bSuccess) {
                folderSelected(result);
            }
            break;
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

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if(y > ofGetHeight() - 60 && x < UI_PANEL_WIDTH) {
        ofFileDialogResult result = ofSystemLoadDialog("Select folder containing images", true);
        if(result.bSuccess) {
            folderSelected(result);
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

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
    dir.listDir(path);
    dir.allowExt("jpg");
    dir.allowExt("png");
    
    for(int i = 0; i < dir.size(); i++) {
        imagePaths.push_back(dir.getPath(i));
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

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    // Empty implementation - this is a required method in openFrameworks
}
