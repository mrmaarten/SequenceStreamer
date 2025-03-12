#include "ofApp.h"
// Remove ofxDatGui include, keep only ofxGui
#include "ofxGui.h"

// Constants
const float ofApp::SLIDER_MIDPOINT = 0.5f;
const float ofApp::BASE_FPS = 30.0f;
const float ofApp::MAX_SPEED = 4.0f;

//--------------------------------------------------------------
void ofApp::setup(){
    // Add detailed logging for resource loading
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofLogNotice("ofApp") << "Starting application setup";
    
    // Get the executable path to help with debugging
    string executablePath = ofFilePath::getCurrentExeDir();
    ofLogNotice("ofApp") << "Executable path: " << executablePath;
    
    // Remove all ofxDatGui asset loading code
    // Remove lines from "DIRECT APPROACH" through the theme setup
    
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
    
    // Setup ofxGui
    gui.setup("Frame Player");
    gui.setPosition(10, 10);
    gui.setSize(UI_PANEL_WIDTH - 20, 30);
    
    // Add open folder button at the top
    openFolderButtonGui.setup("Open Folder");
    openFolderButtonGui.addListener(this, &ofApp::onOpenFolderEvent);
    gui.add(&openFolderButtonGui);
    
    // Add controls
    playButtonGui.setup("Play");
    playButtonGui.addListener(this, &ofApp::onPlayButtonEvent);
    gui.add(&playButtonGui);
    
    speedSliderGui.setup("Speed", 1.0f, 0.0f, MAX_SPEED);
    speedSliderGui.addListener(this, &ofApp::onSpeedSliderEvent);
    gui.add(&speedSliderGui);
    
    // Add speed preset buttons
    speedPresetsGui.setup("Speed Presets");
    speed02xGui.setup("0.2x");
    speed02xGui.addListener(this, &ofApp::onSpeed02xEvent);
    speedPresetsGui.add(&speed02xGui);
    
    speed05xGui.setup("0.5x");
    speed05xGui.addListener(this, &ofApp::onSpeed05xEvent);
    speedPresetsGui.add(&speed05xGui);
    
    speed1xGui.setup("1x");
    speed1xGui.addListener(this, &ofApp::onSpeed1xEvent);
    speedPresetsGui.add(&speed1xGui);
    
    speed2xGui.setup("2x");
    speed2xGui.addListener(this, &ofApp::onSpeed2xEvent);
    speedPresetsGui.add(&speed2xGui);
    
    gui.add(&speedPresetsGui);
    
    // Add playback direction controls
    directionGroupGui.setup("Playback Direction");
    directionForwardGui.setup("Forward", true);
    directionForwardGui.addListener(this, &ofApp::onDirectionForwardEvent);
    directionGroupGui.add(&directionForwardGui);
    
    directionBackwardGui.setup("Backward", false);
    directionBackwardGui.addListener(this, &ofApp::onDirectionBackwardEvent);
    directionGroupGui.add(&directionBackwardGui);
    
    gui.add(&directionGroupGui);
    
    // Add loop mode controls
    loopGroupGui.setup("Loop Mode");
    loopModeToggleGui.setup("Loop", true);
    loopModeToggleGui.addListener(this, &ofApp::onLoopModeEvent);
    loopGroupGui.add(&loopModeToggleGui);
    
    pingPongModeToggleGui.setup("Ping Pong", false);
    pingPongModeToggleGui.addListener(this, &ofApp::onPingPongModeEvent);
    loopGroupGui.add(&pingPongModeToggleGui);
    
    gui.add(&loopGroupGui);
    
    // Add scrubber bar
    scrubberSliderGui.setup("Scrub", 0, 0, 1);
    scrubberSliderGui.addListener(this, &ofApp::onScrubberEvent);
    gui.add(&scrubberSliderGui);
    
    // Add frame range controls
    startFrameSliderGui.setup("Start Frame", 1, 1, 1);
    startFrameSliderGui.addListener(this, &ofApp::onStartFrameEvent);
    gui.add(&startFrameSliderGui);
    
    endFrameSliderGui.setup("End Frame", 1, 1, 1);
    endFrameSliderGui.addListener(this, &ofApp::onEndFrameEvent);
    gui.add(&endFrameSliderGui);
    
    // Add "Play Last X Frames" controls
    lastFramesGroupGui.setup("Play Last X Frames");
    
    last5FramesGui.setup("Last 5");
    last5FramesGui.addListener(this, &ofApp::onLast5FramesEvent);
    lastFramesGroupGui.add(&last5FramesGui);
    
    last10FramesGui.setup("Last 10");
    last10FramesGui.addListener(this, &ofApp::onLast10FramesEvent);
    lastFramesGroupGui.add(&last10FramesGui);
    
    last100FramesGui.setup("Last 100");
    last100FramesGui.addListener(this, &ofApp::onLast100FramesEvent);
    lastFramesGroupGui.add(&last100FramesGui);
    
    gui.add(&lastFramesGroupGui);
    
    // Custom input for last frames (ofxGui doesn't have text input, using slider instead)
    customLastFramesGui.setup("Custom Last Frames", 10, 1, 1000);
    customLastFramesGui.addListener(this, &ofApp::onCustomLastFramesEvent);
    gui.add(&customLastFramesGui);
    
    // Current frame display (using a label)
    currentFrameLabelGui.setup("Frame", "0/0");
    gui.add(&currentFrameLabelGui);
    
    // Black screen toggle
    blackScreenToggleGui.setup("Black Screen", false);
    blackScreenToggleGui.addListener(this, &ofApp::onBlackScreenToggleEvent);
    gui.add(&blackScreenToggleGui);
    
    // Add Syphon controls
    syphonGroupGui.setup("Syphon Settings");
    
    // ofxGui doesn't have text input, using sliders instead
    syphonWidthSliderGui.setup("Width", 1920, 320, 3840);
    syphonWidthSliderGui.addListener(this, &ofApp::onSyphonWidthEvent);
    syphonGroupGui.add(&syphonWidthSliderGui);
    
    syphonHeightSliderGui.setup("Height", 1080, 240, 2160);
    syphonHeightSliderGui.addListener(this, &ofApp::onSyphonHeightEvent);
    syphonGroupGui.add(&syphonHeightSliderGui);
    
    aspectRatioToggleGui.setup("Maintain Aspect Ratio", true);
    aspectRatioToggleGui.addListener(this, &ofApp::onAspectRatioEvent);
    syphonGroupGui.add(&aspectRatioToggleGui);
    
    applySyphonSizeButtonGui.setup("Apply Size");
    applySyphonSizeButtonGui.addListener(this, &ofApp::onApplySyphonSizeEvent);
    syphonGroupGui.add(&applySyphonSizeButtonGui);
    
    gui.add(&syphonGroupGui);
    
    // Set initial display path
    displayPath = "";
    
    // Log the app bundle structure
    ofLogNotice("ofApp") << "Listing app bundle contents:";
    ofDirectory appDir;
    appDir.listDir(ofToDataPath("../Resources/"));
    for (int i = 0; i < appDir.size(); i++) {
        ofLogNotice("ofApp") << appDir.getPath(i);
    }
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

    if (isPlaying && !showBlackScreen && !imagePaths.empty() && speedSliderGui > 0.0f) {
        float frameTime = 1.0f / (BASE_FPS * convertSliderToSpeed(speedSliderGui));
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
                        directionForwardGui = false;
                        directionBackwardGui = true;
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
                        directionForwardGui = true;
                        directionBackwardGui = false;
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
        scrubberSliderGui = scrubberPos;
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
    syphonFbo.begin();
    ofClear(0, 0, 0, 255);
    
    if (!showBlackScreen && currentImage.isAllocated()) {
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
    }
    // We don't need to draw anything else when showBlackScreen is true
    // as we already cleared the FBO to black
    
    syphonFbo.end();
    
    // Send FBO to Syphon (always, even for black screen)
    syphonServer.publishTexture(&syphonFbo.getTexture());
    
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
    gui.draw();
    
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
    
    // Add a visual cue for the drop zone
    ofSetColor(120);
    ofDrawBitmapString("Drop folder here open", 20, ofGetHeight() - 45);
    
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
            playButtonGui.setName(isPlaying ? "Pause" : "Play");
            break;
        }
        case 'b': {
            showBlackScreen = !showBlackScreen;
            blackScreenToggleGui = showBlackScreen;
            break;
        }
        case 'o': {
            onOpenFolderEvent();
            break;
        }
        case 'f': {
            // Toggle forward direction
            playDirection = FORWARD;
            directionForwardGui = true;
            directionBackwardGui = false;
            break;
        }
        case 'r': {
            // Toggle reverse direction
            playDirection = BACKWARD;
            directionForwardGui = false;
            directionBackwardGui = true;
            break;
        }
        case 'l': {
            // Toggle loop mode
            loopMode = LOOP;
            loopModeToggleGui = true;
            pingPongModeToggleGui = false;
            break;
        }
        case 'p': {
            // Toggle ping pong mode
            loopMode = PING_PONG;
            loopModeToggleGui = false;
            pingPongModeToggleGui = true;
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
        // ofxGui doesn't have getX() and getY() methods, so we need to use a different approach
        ofRectangle scrubberRect = scrubberSliderGui.getShape();
        float barX = scrubberRect.x;
        float barWidth = scrubberRect.width;
        
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
            scrubberSliderGui = percentage;
        }
    } else if(y >= ofGetHeight() - 60 && y <= ofGetHeight() - 10 && x >= 10 && x <= UI_PANEL_WIDTH - 10) {
        // More precise check for the drop zone rectangle
        ofFileDialogResult result = ofSystemLoadDialog("Select folder containing images", true);
        if(result.bSuccess) {
            folderSelected(result);
        }
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    // Check if click is within scrubber bar
    ofRectangle scrubberRect = scrubberSliderGui.getShape();
    float barX = scrubberRect.x;
    float barWidth = scrubberRect.width;
    
    if (x >= barX && x <= barX + barWidth) {
        
        // Store current playback state
        prevPlayState = isPlaying;
        prevPlaySpeed = speedSliderGui;
        
        // Pause playback during scrubbing
        isPlaying = false;
        playButtonGui.setName("Play");
        
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
        scrubberSliderGui = percentage;
    } else if(y >= ofGetHeight() - 60 && y <= ofGetHeight() - 10 && x >= 10 && x <= UI_PANEL_WIDTH - 10) {
        // More precise check for the drop zone rectangle
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
        speedSliderGui = prevPlaySpeed;
        
        // Update play button appearance
        playButtonGui.setName(isPlaying ? "Pause" : "Play");
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
    gui.setPosition(10, 10);
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
        startFrameSliderGui.setMax(lastFrame);
        endFrameSliderGui.setMax(lastFrame);
        
        startFrameSliderGui = 1;
        endFrameSliderGui = lastFrame;
        
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
    rangeStart = startFrameSliderGui - 1;
    rangeEnd = endFrameSliderGui - 1;
    
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
    currentFrameLabelGui = info;
}

// New event handlers for ofxGui
void ofApp::onPlayButtonEvent(){
    isPlaying = !isPlaying;
    playButtonGui.setName(isPlaying ? "Pause" : "Play");
}

void ofApp::onSpeedSliderEvent(float & value){
    float actualSpeed = convertSliderToSpeed(value);
    playbackSpeed = (actualSpeed > 0.0f) ? (1.0f / (BASE_FPS * actualSpeed)) : 0.0f;
    lastImageTime = ofGetElapsedTimef();
}

void ofApp::onSpeed02xEvent(){
    speedSliderGui = 0.2f;
}

void ofApp::onSpeed05xEvent(){
    speedSliderGui = 0.5f;
}

void ofApp::onSpeed1xEvent(){
    speedSliderGui = 1.0f;
}

void ofApp::onSpeed2xEvent(){
    speedSliderGui = 2.0f;
}

void ofApp::onDirectionForwardEvent(bool & value){
    if (value) {
        playDirection = FORWARD;
        directionBackwardGui = false;
    } else if (!directionBackwardGui) {
        // Don't allow both to be unchecked
        directionForwardGui = true;
    }
}

void ofApp::onDirectionBackwardEvent(bool & value){
    if (value) {
        playDirection = BACKWARD;
        directionForwardGui = false;
    } else if (!directionForwardGui) {
        // Don't allow both to be unchecked
        directionBackwardGui = true;
    }
}

void ofApp::onLoopModeEvent(bool & value){
    if (value) {
        loopMode = LOOP;
        pingPongModeToggleGui = false;
    } else if (!pingPongModeToggleGui) {
        // Don't allow both to be unchecked
        loopModeToggleGui = true;
    }
}

void ofApp::onPingPongModeEvent(bool & value){
    if (value) {
        loopMode = PING_PONG;
        loopModeToggleGui = false;
    } else if (!loopModeToggleGui) {
        // Don't allow both to be unchecked
        pingPongModeToggleGui = true;
    }
}

void ofApp::onScrubberEvent(float & value){
    if (!imagePaths.empty() && rangeEnd > rangeStart) {
        // Calculate frame index based on percentage
        int frameIndex = rangeStart + round(value * (rangeEnd - rangeStart));
        frameIndex = ofClamp(frameIndex, rangeStart, rangeEnd);
        
        // Update current frame
        if (frameIndex != currentImageIndex && frameIndex < imagePaths.size()) {
            currentImageIndex = frameIndex;
            currentImage.load(imagePaths[currentImageIndex]);
            updateFrameInfo();
        }
    }
}

void ofApp::onStartFrameEvent(int & value){
    updateImageRange();
}

void ofApp::onEndFrameEvent(int & value){
    updateImageRange();
}

void ofApp::onBlackScreenToggleEvent(bool & value){
    showBlackScreen = value;
}

void ofApp::onSyphonWidthEvent(int & value){
    syphonWidth = value;
}

void ofApp::onSyphonHeightEvent(int & value){
    syphonHeight = value;
}

void ofApp::onAspectRatioEvent(bool & value){
    maintainAspectRatio = value;
}

void ofApp::onApplySyphonSizeEvent(){
    syphonFbo.allocate(syphonWidth, syphonHeight, GL_RGBA);
}

void ofApp::onLast5FramesEvent(){
    setLastXFrames(5);
    customLastFramesGui = 5;
}

void ofApp::onLast10FramesEvent(){
    setLastXFrames(10);
    customLastFramesGui = 10;
}

void ofApp::onLast100FramesEvent(){
    setLastXFrames(100);
    customLastFramesGui = 100;
}

void ofApp::onCustomLastFramesEvent(int & value){
    setLastXFrames(value);
}

void ofApp::setLastXFrames(int numFrames){
    if (!imagePaths.empty()) {
        int totalFrames = imagePaths.size();
        
        // Calculate new range
        int newStart = std::max(0, totalFrames - numFrames);
        int newEnd = totalFrames - 1;
        
        // Update sliders
        startFrameSliderGui = newStart + 1; // Convert to 1-based for display
        endFrameSliderGui = newEnd + 1;     // Convert to 1-based for display
        
        // Update internal range variables
        rangeStart = newStart;
        rangeEnd = newEnd;
        
        // Set current frame to start of range
        currentImageIndex = rangeStart;
        if (currentImageIndex < imagePaths.size()) {
            currentImage.load(imagePaths[currentImageIndex]);
            updateFrameInfo();
        }
    }
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    // Empty implementation - this is a required method in openFrameworks
}

// Add the event handler for the open folder button
void ofApp::onOpenFolderEvent(){
    ofFileDialogResult result = ofSystemLoadDialog("Select folder containing images", true);
    if(result.bSuccess) {
        folderSelected(result);
    }
}
