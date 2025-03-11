#pragma once

#include "ofMain.h"
#include "ofxDatGuiCustom.h"
#include "ofxDatGui.h"
#include "ofxSyphon.h"

class ofApp : public ofBaseApp {
public:
	// Playback direction enum
	enum PlayDirection {
		FORWARD,
		BACKWARD
	};
	
	// Loop mode enum
	enum LoopMode {
		LOOP,
		PING_PONG
	};
	
	void setup();
	void update();
	void draw();
	void exit();
	
	// UI Events
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void mouseScrolled(int x, int y, float scrollX, float scrollY);
	
	void folderSelected(ofFileDialogResult openFileResult);
	
	// Image handling
	void loadImagesFromDirectory(string path);
	void updateImageRange();
	void updateFrameInfo();
	
	// Variables
	string directoryPath;
	vector<string> imagePaths;
	ofImage currentImage;
	
	// Playback control
	int currentImageIndex;
	float playbackSpeed; // in seconds
	float lastImageTime;
	bool isPlaying;
	bool showBlackScreen;
	
	// Range control
	int rangeStart;
	int rangeEnd;
	
	// Syphon
	ofxSyphonServer syphonServer;
	
	// UI Event Handlers
	void onPlayButtonEvent(ofxDatGuiButtonEvent e);
	void onSpeedSliderEvent(ofxDatGuiSliderEvent e);
	void onStartFrameEvent(ofxDatGuiSliderEvent e);
	void onEndFrameEvent(ofxDatGuiSliderEvent e);
	void onBlackScreenToggleEvent(ofxDatGuiToggleEvent e);
	void onSyphonWidthEvent(ofxDatGuiTextInputEvent e);
	void onSyphonHeightEvent(ofxDatGuiTextInputEvent e);
	void onAspectRatioEvent(ofxDatGuiToggleEvent e);
	void onApplySyphonSizeEvent(ofxDatGuiButtonEvent e);
	void onSpeedButtonEvent(ofxDatGuiButtonEvent e);
	void onScrubberEvent(ofxDatGuiSliderEvent e);
	void onLastFramesButtonEvent(ofxDatGuiButtonEvent e);
	void onLastFramesInputEvent(ofxDatGuiTextInputEvent e);
	
	// New event handlers for direction and loop mode
	void onDirectionForwardEvent(ofxDatGuiToggleEvent e);
	void onDirectionBackwardEvent(ofxDatGuiToggleEvent e);
	void onLoopModeEvent(ofxDatGuiToggleEvent e);
	void onPingPongModeEvent(ofxDatGuiToggleEvent e);
	
	// GUI elements
	ofxDatGui* gui;
	ofxDatGuiButton* playButton;
	ofxDatGuiSlider* speedSlider;
	ofxDatGuiSlider* scrubberBar;
	ofxDatGuiSlider* startFrameSlider;
	ofxDatGuiSlider* endFrameSlider;
	ofxDatGuiLabel* currentFrameLabel;
	ofxDatGuiToggle* blackScreenToggle;
	ofxDatGuiTextInput* syphonWidthInput;
	ofxDatGuiTextInput* syphonHeightInput;
	ofxDatGuiToggle* aspectRatioToggle;
	ofxDatGuiButton* applySyphonSizeButton;
	vector<ofxDatGuiButton*> speedButtons;
	
	// New GUI elements for direction and loop mode
	ofxDatGuiToggle* directionForwardButton;
	ofxDatGuiToggle* directionBackwardButton;
	ofxDatGuiToggle* loopModeButton;
	ofxDatGuiToggle* pingPongModeButton;
	
	// Playback variables
	PlayDirection playDirection;
	LoopMode loopMode;
	
	// Directory watching
	float lastCheckTime;
	float checkInterval; // how often to check for new files (in seconds)
	string displayPath; // Shortened version of directoryPath for display
	
	// UI Layout
	const int UI_PANEL_WIDTH = 332; 
	ofRectangle uiPanel;
	ofRectangle previewPanel;
	
	const float BASE_FPS = 12.0f;  // 1.0 on slider = 12fps
	const float MIN_SPEED = 0.2f;
	const float MAX_SPEED = 4.0f;  
	const float DEFAULT_SPEED = 1.0f;
	const float SLIDER_MIDPOINT = 0.667f;  // 2/3 point where speed = 1.0
	
	// Speed conversion helpers
	float convertSliderToSpeed(float sliderValue);
	float convertSpeedToSlider(float speed);
	
	// Syphon output handling
	ofFbo syphonFbo;
	int syphonWidth;
	int syphonHeight;
	bool maintainAspectRatio;
	
	// Playback variables
	bool isScrubbing = false;
	bool prevPlayState = false;
	float prevPlaySpeed = 1.0f;
	
	// Changed from 'dir' to 'imageDir'
	ofDirectory imageDir;
	
	// New GUI elements for last frames functionality
	vector<ofxDatGuiButton*> lastFramesButtons;
	ofxDatGuiTextInput* lastFramesInput;
	
	// Helper function for setting last X frames
	void setLastXFrames(int numFrames);
};
