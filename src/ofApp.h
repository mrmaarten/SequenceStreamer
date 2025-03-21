#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxSyphon.h"

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();
	
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
	
	// Helper methods
	void folderSelected(ofFileDialogResult result);
	void loadImagesFromDirectory(string path);
	void updateImageRange();
	void updateFrameInfo();
	void setLastXFrames(int numFrames);
	float convertSliderToSpeed(float sliderValue);
	float convertSpeedToSlider(float speed);
	
	// Event handlers for ofxGui
	void onPlayButtonEvent();
	void onSpeedSliderEvent(float & value);
	void onSpeed02xEvent();
	void onSpeed05xEvent();
	void onSpeed1xEvent();
	void onSpeed2xEvent();
	void onDirectionForwardEvent(bool & value);
	void onDirectionBackwardEvent(bool & value);
	void onLoopModeEvent(bool & value);
	void onPingPongModeEvent(bool & value);
	void onScrubberEvent(float & value);
	void onStartFrameEvent(int & value);
	void onEndFrameEvent(int & value);
	void onBlackScreenToggleEvent(bool & value);
	void onSyphonWidthEvent(int & value);
	void onSyphonHeightEvent(int & value);
	void onAspectRatioEvent(bool & value);
	void onApplySyphonSizeEvent();
	void onLast5FramesEvent();
	void onLast10FramesEvent();
	void onLast100FramesEvent();
	void onCustomLastFramesEvent(int & value);
	void onOpenFolderEvent();
	void onSyphon1080pEvent();
	void onSyphon720pEvent();
	void onSyphonImageResEvent();
	void onSyphonHalfResEvent();
	void onScrubbingQualityEvent(int & value);
	void onUltraLowQualityEvent(bool & value);
	
	// Constants
	static const float BASE_FPS;
	static const float MAX_SPEED;
	static const float SLIDER_MIDPOINT;
	static const int UI_PANEL_WIDTH = 300;
	
	// Playback direction enum
	enum Direction {
		FORWARD,
		BACKWARD
	};
	
	// Loop mode enum
	enum LoopMode {
		LOOP,
		PING_PONG
	};
	
	// UI layout
	ofRectangle uiPanel;
	ofRectangle previewPanel;
	
	// ofxGui elements
	ofxPanel gui;
	ofxButton playButtonGui;
	ofxFloatSlider speedSliderGui;
	ofxButton openFolderButtonGui;
	
	// Speed presets
	ofxPanel speedPresetsGui;
	ofxButton speed02xGui;
	ofxButton speed05xGui;
	ofxButton speed1xGui;
	ofxButton speed2xGui;
	
	// Direction controls
	ofxPanel directionGroupGui;
	ofxToggle directionForwardGui;
	ofxToggle directionBackwardGui;
	
	// Loop mode controls
	ofxPanel loopGroupGui;
	ofxToggle loopModeToggleGui;
	ofxToggle pingPongModeToggleGui;
	
	// Scrubber
	ofxFloatSlider scrubberSliderGui;
	
	// Frame range controls
	ofxIntSlider startFrameSliderGui;
	ofxIntSlider endFrameSliderGui;
	
	// Last frames controls
	ofxPanel lastFramesGroupGui;
	ofxButton last5FramesGui;
	ofxButton last10FramesGui;
	ofxButton last100FramesGui;
	ofxIntSlider customLastFramesGui;
	
	// Display and toggles
	ofxLabel currentFrameLabelGui;
	ofxToggle blackScreenToggleGui;
	
	// Syphon controls
	ofxPanel syphonGroupGui;
	ofxIntSlider syphonWidthSliderGui;
	ofxIntSlider syphonHeightSliderGui;
	ofxToggle aspectRatioToggleGui;
	ofxButton applySyphonSizeButtonGui;
	ofxLabel syphonDeviderGui;
	ofxButton syphon1080pGui;
	ofxButton syphon720pGui;
	ofxButton syphonImageResGui;
	ofxButton syphonHalfResGui;
	
	// Scrubbing quality control
	ofxPanel scrubbingGroupGui;
	ofxIntSlider scrubbingQualitySliderGui;
	ofxToggle ultraLowQualityToggleGui;
	
	// Image and playback variables
	ofImage currentImage;
	vector<string> imagePaths;
	ofDirectory imageDir;
	string directoryPath;
	string displayPath;
	int currentImageIndex;
	float playbackSpeed;
	float lastImageTime;
	bool isPlaying = false;
	bool showBlackScreen;
	int rangeStart;
	int rangeEnd;
	bool rangeSetByUser = false;
	int lastFrame = 0;
	float lastCheckTime;
	float checkInterval;
	Direction playDirection;
	LoopMode loopMode;
	
	// Scrubbing variables
	bool isScrubbing = false;
	float scrubEndTime = 0;

	bool prevPlayState;
	float prevPlaySpeed;
	int previousDirSize;
	
	// Syphon variables
	ofxSyphonServer syphonServer;
	ofFbo syphonFbo;
	int syphonWidth;
	int syphonHeight;
	bool maintainAspectRatio;

	// Scrubbing quality control
	int scrubbingQuality = 320;  // Width in pixels for scrubbing preview (lower = faster)
	bool ultraLowQualityScrubbing = false;  // For very large image sets

	// Add this to your class declaration
	void checkDirectoryForChanges();
	void checkScrubEnd(ofEventArgs &args);

private:
	ofDirectory getImageDirectory(const string& path);
};