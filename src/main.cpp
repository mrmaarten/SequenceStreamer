#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    //Use ofGLFWWindowSettings for more options like multi-monitor fullscreen
    ofGLFWWindowSettings settings;
    settings.setSize(1612, 768);
    settings.setGLVersion(3, 2); // Set OpenGL version
    settings.windowMode = OF_WINDOW; //can also be OF_FULLSCREEN

    auto window = ofCreateWindow(settings);

    ofRunApp(window, make_shared<ofApp>());
    ofRunMainLoop();
} 
