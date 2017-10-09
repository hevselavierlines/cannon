#pragma once

#include <vector>

#include "ofMain.h"
#include "ofxImGui.h"

#include "ofxXmlSettings.h"
#include "YAMPE/Particle.h"

class ofApp : public ofBaseApp {
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
		
    // simple 3D world with ground and axes
    const float RANGE = 16;
    ofEasyCam easyCam;
    float cameraHeightRatio = 0.02f;
    ofPoint easyCamTarget = ofPoint(0,5,0);
    void cameraHeightRatioChanged(float & cameraHeightRatio);

    ofPlanePrimitive ground;
    
    ofxImGui::Gui gui;                   // single entery to ImGUI subsystem
    ofRectangle mainWindowRectangle;        //
    ofRectangle loggingWindowRectangle;     //
    void showUserGuide();
    void drawAppMenuBar();
    void drawMainWindow();
    void drawLoggingWindow();
    
    // simimulation (generic)
    void reset();
    void quit();
    float t = 0.0f;
    bool isRunning = true;
    
    ofParameter<bool> isAxisVisible = true;
    ofParameter<bool> isXGridVisible = false;
    ofParameter<bool> isYGridVisible = true;
    ofParameter<bool> isZGridVisible = false;
    ofParameter<bool> isGroundVisible = true;
    ofParameter<bool> isFullScreen;
    ofParameter<std::string> position;

    // simulation (specific stuff)
    // cannon attributes
    float elevation;                        ///< rotation about the y-axis
    float direction;                        ///< rotation about the z-axis
    float muzzleSpeed;                      ///< magnitude of initial velocity

    // game state
    enum GameState {START, PLAY, FIRED, HIT};
    int gameState;
    vector <string> gameStates;
    
    // cannon ball --- TODO we could have an pool of balls instead
    YAMPE::Particle ball;
    vector<YAMPE::Particle::Ref> balls;
    ofVec3f target;         //< target - note y coordinate is zero
    float energyError;
    
    void aim();
    void fire();
    float range(float e);
    float calculateElevation(float targetDistance);
    vector<float> heightLine;
    vector<float> velocityLine;
    vector<float> energyLine;
private:

    // or here

};
