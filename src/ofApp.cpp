#include <iostream>
#include <math.h>
#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    // repatable randomness
    ofSeedRandom();
    
    // simulation specific stuff goes here

    gui.setup();
    ImGui::GetIO().MouseDrawCursor = false;

    // load parameters from file
    // loadFromFile("settings.xml");
    
    // instantiate the ground
    ground.set(RANGE, RANGE);
    ground.rotate(90, 1,0,0);
    
    // lift camera to 'eye' level
    easyCam.setDistance(RANGE);
    float d = easyCam.getDistance();
    easyCam.setPosition(ofVec3f(0, cameraHeightRatio*d, d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio))+easyCamTarget);
    easyCam.setTarget(easyCamTarget);

    // simulation specific stuff goes here
    
    string gameStateLabels[] = {"START", "PLAY", "FIRED", "HIT"};
    gameStates.assign(gameStateLabels, gameStateLabels+4);
    gameState = START;
    
    target.set(ofRandom(1) * 10.0f - 5.0f, 0, ofRandom(1) * 10.0f - 5.0f);
    muzzleSpeed = 4.0f;
    ball.setBodyColor(ofColor(0x666666));
    reset();
    balls.reserve(100);
    for(int i = 0; i < 100; i++) {
        YAMPE::Particle partBall;
        partBall.setBodyColor(ofColor(0x444444));
        float num = (100 - i) * 0.001;
        partBall.setRadius(num);
        balls.push_back(partBall);
    }
}

void ofApp::reset() {

    t = 0.0f;
    
    target.set(ofRandom(1) * 10.0f - 5.0f, 0, ofRandom(1) * 10.0f - 5.0f);
    ball.force = ofVec3f();
    ball.acceleration = ofVec3f();
    ball.velocity = ofVec3f();
    ball.position = ofVec3f();
}

void ofApp::update() {

    float dt = ofClamp(ofGetLastFrameTime(), 0.0, 0.02);
    if (!isRunning) return;
    t += dt;

    // simulation specific stuff goes here
    if(dt > 0) {
        if(ball.position.y > 0) {
            ball.applyForce(ofVec3f(0, -0.981f, 0));
        } else {
            if(ball.position.y < 0) {
                gameState = HIT;
            }
            ball.velocity = ofVec3f(0, 0, 0);
            ball.position.y = 0;
        }
        for(int i = balls.size() - 1; i >= 0; i--) {
            balls[i + 1].position.x = balls[i].position.x;
            balls[i + 1].position.y = balls[i].position.y;
            balls[i + 1].position.z = balls[i].position.z;
        }
        balls[0].position.x = ball.position.x;
        balls[0].position.y = ball.position.y;
        balls[0].position.z = ball.position.z;
        ball.integrate(dt);
    } else {
    }
}

void ofApp::draw() {
    
 
    ofEnableDepthTest();
    ofBackgroundGradient(ofColor(128), ofColor(0), OF_GRADIENT_BAR);
    
    ofPushStyle();
    //cout <<"Camera" <<easyCamTarget <<endl;
    easyCam.begin();
    
    ofDrawGrid(RANGE/(2*8), 8, false, isXGridVisible, isYGridVisible, isZGridVisible);
    
    if (isAxisVisible) {
        ofDrawAxis(1);
    }
    
    if (isGroundVisible) {
        ofPushStyle();
        ofSetHexColor(0xB87333);
        ground.draw();
        ofPopStyle();
    }
    
    // simulation specific stuff goes here
    ofSetColor(255, 0, 0);
    ofDrawBox(0, 0.2, 0, 0.8, 0.4, 0.8);
    ofDrawSphere(0, 0.4, 0, 0.3);
    
    ofPushMatrix();
    ofTranslate(0, 0.5, 0);
    float rightDirection = direction + 90.0f;
    ofRotate(rightDirection, 0, 1.0, 0);
    float rightElevation = 90.0f - elevation;
    ofRotate(rightElevation, 1.0, 0, 0);
    ofTranslate(0, -0.5, 0);
    ofSetColor(255, 128, 0);
    ofDrawCylinder(0, 1.0, 0, 0.15, 1);
    ofPopMatrix();

    ofSetColor(0, 0, 0);
    ofDrawBox(target.x, 0, target.z, 1, 0.1, 1);
    
    ball.draw();
    for(int i = 0; i < balls.size(); i++) {
        balls[i].draw();
    }
    
    ofPopStyle();

    
    easyCam.end();
    ofPopStyle();

    // draw gui elements
    gui.begin();
    drawAppMenuBar();
    drawMainWindow();
    drawLoggingWindow();
    
    
    gui.end();
    
    
}


void ofApp::drawAppMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "")) quit();
            ImGui::EndMenu();
        }
        
        float d = easyCam.getDistance();
        
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem(isAxisVisible?"Hide Unit Axis":"Show Unit Axis", "")) {isAxisVisible = !isAxisVisible;}
            if (ImGui::MenuItem(isGroundVisible?"Hide Ground":"Show Ground", "")) {isGroundVisible = !isGroundVisible;}
            if (ImGui::MenuItem(isXGridVisible?"Hide Grid (X)":"Show Grid (X)", "")) {isXGridVisible = !isXGridVisible;}
            if (ImGui::MenuItem(isYGridVisible?"Hide Grid (Y)":"Show Grid (Y)", "")) {isYGridVisible = !isYGridVisible;}
            if (ImGui::MenuItem(isZGridVisible?"Hide Grid (Z)":"Show Grid (Z)", "")) {isZGridVisible = !isZGridVisible;}
            ImGui::Separator();
            if (ImGui::MenuItem("Align camera above X axis ", "")) {
                easyCam.setPosition(ofVec3f(d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio), cameraHeightRatio*d, 0)+easyCamTarget);
                easyCam.setTarget(easyCamTarget);
            }
            if (ImGui::MenuItem("Align camera above Z axis ", "")) {
                easyCam.setPosition(ofVec3f(0, cameraHeightRatio*d, d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio))+easyCamTarget);
                easyCam.setTarget(easyCamTarget);
                cout <<"here";
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Align camera along X axis ", "")) {
                easyCam.setPosition(ofVec3f(d, 0, 0)+easyCamTarget);
                easyCam.setTarget(easyCamTarget);
            }
            if (ImGui::MenuItem("Align camera along Y axis ", "")) {
                easyCam.setPosition(ofVec3f(0.001, d, 0)+easyCamTarget);
                easyCam.setTarget(easyCamTarget);
            }
            if (ImGui::MenuItem("Align camera along Z axis ", "")) {
                easyCam.setPosition(ofVec3f(0, 0, d)+easyCamTarget);
                easyCam.setTarget(easyCamTarget);
            }
            
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


void ofApp::drawMainWindow() {
    

    ImGui::SetNextWindowSize(ImVec2(400,400), ImGuiSetCond_FirstUseEver);
    if (ImGui::Begin("Main")) {

        if (ImGui::CollapsingHeader("3D")) {
            if(ImGui::Button("Reset##CameraTarget")) {
                easyCamTarget = ofVec3f(0,5,0);
                easyCam.setTarget(easyCamTarget);
            }

            ImGui::SameLine();
            if (ImGui::InputFloat3("Camera Target", &easyCamTarget.x)) {
                easyCam.setTarget(easyCamTarget);
            }
            if (ImGui::SliderFloat("Camera Height Ratio", &cameraHeightRatio, 0.0f, 1.0f))
                cameraHeightRatioChanged(cameraHeightRatio);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::SliderFloat("MuzzleSpeed", &muzzleSpeed, 3.0f, 5.0f, "%2.2f (m/s)");
            ImGui::SliderFloat("Elevation", &elevation, 0.0f, 90.0f, "%3.0f (deg)", 1);
            ImGui::SliderFloat("Direction", &direction, 0.0f, 360.0f, "%3.0f (deg)", 1);
            if(ImGui::Button("Aim")) aim();
            ImGui::SameLine();
            if(ImGui::Button("fire")) fire();
        }
        

        if(ImGui::Button("Reset")) {reset();}
        ImGui::SameLine();
        if(ImGui::Button(isRunning?"Stop":" Go ")) {isRunning = !isRunning;}
        ImGui::SameLine();
        ImGui::Text("   Time = %8.1f", t);
        if(ImGui::Button("Quit")) {quit();}
        
        
        if (ImGui::CollapsingHeader("Numerical Output")) {
            // Display some useful info
            ImGui::Text("Elevation:     % 5.2f", elevation);
            ImGui::Text("Direction:     % 5.2f", direction);
            ImGui::Text("Game State:    %5s", gameStates[gameState].c_str());
        }
        
        
        if (ImGui::CollapsingHeader("Graphical Output")) {
        }
    }
    // store window size so that camera can ignore mouse clicks
    mainWindowRectangle.setPosition(ImGui::GetWindowPos().x,ImGui::GetWindowPos().y);
    mainWindowRectangle.setSize(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
    ImGui::End();

}



void ofApp::drawLoggingWindow() {
    ImGui::SetNextWindowSize(ImVec2(200,300), ImGuiSetCond_FirstUseEver);
    if (ImGui::Begin("Logging")) {
    }
    // store window size so that camera can ignore mouse clicks
    loggingWindowRectangle.setPosition(ImGui::GetWindowPos().x,ImGui::GetWindowPos().y);
    loggingWindowRectangle.setSize(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
    ImGui::End();
}

//--------------------------------------------------------------
// GUI events and listeners
//--------------------------------------------------------------

void ofApp::keyPressed(int key) {
    
    float d = easyCam.getDistance();
    
    switch (key) {
        
//        case 'h':                               // toggle GUI/HUD
//           isGuiVisible = !isGuiVisible;
//            break;
//        case 'b':                               // toggle debug
//            isDebugVisible = !isDebugVisible;
//            break;
//        case 'a':                               // toggle axis unit vectors
//            isAxisVisible = !isAxisVisible;
//            break;
//        case '1':                               // toggle grids (X)
//           isXGridVisible = !isXGridVisible;
//            break;
//        case '2':                               // toggle grids (Y)
//            isYGridVisible = !isYGridVisible;
//            break;
//        case '3':                               // toggle grids (Z)
//            isZGridVisible = !isZGridVisible;
//            break;
//        case 'g':                               // toggle ground
//            isGroundVisible = !isGroundVisible;
//            break;
//        case 'u':                               // set the up vecetor to be up (ground to be level)
//            easyCam.setTarget(ofVec3f::zero());
//            break;
//
//        case 'S' :                              // save gui parameters to file
//            gui.saveToFile("settings.xml");
//
//            break;
//        case 'L' :                              // load gui parameters
//            gui.loadFromFile("settings.xml");
//            break;
//
        case 'z':
            easyCam.setPosition(ofVec3f(0, cameraHeightRatio*d, d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio))+easyCamTarget);
            easyCam.setTarget(easyCamTarget);
            break;
        case 'Z':
            easyCam.setPosition(0, 0, d);
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'x':
            easyCam.setPosition(d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio), cameraHeightRatio*d, 0);
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'X':
            easyCam.setPosition(ofVec3f(d, 0, 0)+easyCamTarget);
            easyCam.setTarget(easyCamTarget);
            break;
        case 'Y':
            easyCam.setPosition(ofVec3f(0.001, d, 0)+easyCamTarget);
            easyCam.setTarget(easyCamTarget);
            break;
            
        case 'f':                               // toggle fullscreen
            // BUG: window size is not preserved
            isFullScreen = !isFullScreen;
            if (isFullScreen) {
                ofSetFullscreen(false);
            } else {
                ofSetFullscreen(true);
            }
            break;

        // simulation specific stuff goes here

    }
}


void ofApp::cameraHeightRatioChanged(float & cameraHeightRatio) {
    
    float d = easyCam.getDistance();
    easyCam.setPosition(0, cameraHeightRatio*d, d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio));
    easyCam.setTarget(easyCamTarget);
}


void ofApp::quit() {
    ofExit();
}

void ofApp::aim() {
    gameState = PLAY;
    ofVec3f direct;
    direct.x = target.x;
    direct.y = target.y;
    direct.z = target.z;
    direct.normalize();
    
    float angle = atan2(direct.x,direct.z) / 0.0174533f - 90.0f;
    if (angle < 0) {
        angle += 360;
    }
    direction = angle;
    float r = range(elevation);
    float distance = target.length();
    int calcAngle = calculateElevation(distance);
    elevation = calcAngle;
}

float ofApp::range(float e) {
    float ux = muzzleSpeed * cos(e * 0.0174533f);
    float uy = muzzleSpeed * sin(e * 0.0174533f);
    float h = 0.7;
    float g = 0.981;
    
    float calcDistance = ( ux / g ) *( uy + sqrt ( uy*uy + 2*g*h ) ) ;
    return calcDistance;
}

float ofApp::calculateElevation(float targetDistance) {
    float xMin(0.0f), fxMin(targetDistance - range(xMin));
    float xMax(90.0f), fxMax(targetDistance - range(xMax));
    
    while(xMax - xMin > 0.1e-3) {
        float x = 0.5 * (xMin + xMax);
        float fx = targetDistance - range(x);
        
        if(fxMin * fx < 0) {
            xMax = x;
            fxMax = fx;
        } else {
            xMin = x;
            fxMin = fx;
        }
    }
    return 0.5 * (xMin + xMax);
}
void ofApp::fire() {
    ball.position = ofVec3f(0, 0.7, 0);
    ball.velocity = ofVec3f(0, 0, 0);
    ball.acceleration = ofVec3f(0, 0, 0);
    
    float rightDirection = direction + 90.0f;
    float rightElevation = 90.0f - elevation;
    
    float dirY = cos(rightElevation * 0.0174533f) * muzzleSpeed;
    float dirX = sin(rightDirection * 0.0174533f) * sin(rightElevation * 0.0174533) * muzzleSpeed;
    float dirZ = cos(rightDirection * 0.0174533f) * sin(rightElevation * 0.0174533) * muzzleSpeed;
    
    ball.setVelocity(ofVec3f(dirX, dirY, dirZ));
    
    gameState = FIRED;
}
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y ) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {
    // easy camera should ignore GUI mouse clicks
    if (mainWindowRectangle.inside(x,y)||loggingWindowRectangle.inside(x,y))
        easyCam.disableMouseInput();
    else
        easyCam.enableMouseInput();
}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
