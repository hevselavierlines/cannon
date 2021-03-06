#include <math.h>
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    // repatable randomness
    ofSeedRandom();

    gui.setup();
    ImGui::GetIO().MouseDrawCursor = false;

    // load parameters from file
    // loadFromFile("settings.xml");
    
    // instantiate the ground
    ground.set(RANGE, RANGE);
    ground.rotate(90, 1, 0, 0);
    
    // lift camera to 'eye' level
    easyCam.setDistance(RANGE);
    float d = easyCam.getDistance();
    easyCam.setPosition(ofVec3f(0, cameraHeightRatio*d, d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio))+easyCamTarget);
    easyCam.setTarget(easyCamTarget);
    
    string gameStateLabels[] = {"START", "PLAY", "FIRED", "HIT"};
    gameStates.assign(gameStateLabels, gameStateLabels+4);
    gameState = START;
    
    muzzleSpeed = 4.0f;
    ball.setBodyColor(ofColor(0x666666));
    reset();
    balls.reserve(128);
    for(int i = 0; i < 128; i++) {
        YAMPE::Particle::Ref partBall(new YAMPE::Particle());
        partBall->setBodyColor(ofColor(255, i * 2, 0));
        float num = (128 - i) * 0.00078125;
        partBall->setRadius(num);
        balls.push_back(partBall);
    }
    int lineLength = 1000;
    heightLine.reserve(lineLength);
    velocityLine.reserve(lineLength);
    energyLine.reserve(lineLength);
    for(int i = 0; i < lineLength; i++) {
        heightLine.push_back(0);
        velocityLine.push_back(0);
        energyLine.push_back(0);
    }
    
    energyError = 0;
}

void ofApp::reset() {

    t = 0.0f;
    
    target.set(ofRandom(1) * 15.0f - 7.5f, 0, ofRandom(1) * 15.0f - 7.5f);
    ball.force = ofVec3f();
    ball.acceleration = ofVec3f();
    ball.velocity = ofVec3f();
    ball.position = ofVec3f();
}

void ofApp::update() {

    float dt = ofClamp(ofGetLastFrameTime(), 0.0, 0.02);
    if (!isRunning) return;
    t += dt;

    if(dt > 0) {
        if(ball.position.y > 0) {
            //Position over zero means that the ball is still in the air.
            ball.acceleration = ofVec3f(0, -0.981f, 0);
        } else {
            //Position below or equal zero means that the ball has hit the floor.
            if(ball.position.y < 0) {
                gameState = HIT;
            }
            ball.velocity = ofVec3f(0, 0, 0);
            ball.position.y = 0;
        }
        // update the track "balls"
        for(int i = balls.size() - 2; i >= 0; i--) {
            balls[i + 1]->position.x = balls[i]->position.x;
            balls[i + 1]->position.y = balls[i]->position.y;
            balls[i + 1]->position.z = balls[i]->position.z;
        }
        // set the first item of the "track" balls to the current position.
        balls[0]->position.x = ball.position.x;
        balls[0]->position.y = ball.position.y;
        balls[0]->position.z = ball.position.z;
        ball.integrate(dt);
        
        ball.potentialEnergy = 9.81f * ball.position.y * ball.mass();
        ball.kineticEnergy = 0.5f * ball.velocity.lengthSquared() * ball.mass();
        ball.errorEnergy = abs(ball.potentialEnergy - ball.kineticEnergy);
        
        /**
         * Move all the historic line points one position to the left.
         */
        velocityLine[0] = 0.0f;
        energyLine[0] = 0.0f;
        heightLine[0] = 0.0f;
        for(int i = 0; i < heightLine.size() - 1; i++) {
            heightLine[i] = heightLine[i + 1];
            velocityLine[i] = velocityLine[i + 1];
            energyLine[i] = energyLine[i + 1];
        }
        heightLine[heightLine.size() - 1] = ball.position.y;
        velocityLine[velocityLine.size() - 1] = ball.position.x;
        energyLine[energyLine.size() - 1] = ball.errorEnergy;
    }
}

void ofApp::draw() {
    ofEnableDepthTest();
    ofBackgroundGradient(ofColor(128), ofColor(0), OF_GRADIENT_BAR);
    
    ofPushStyle();
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
    ofRotateY(rightDirection);
    float rightElevation = 90.0f - elevation;
    ofRotateX(rightElevation);
    ofTranslate(0, -0.5, 0);
    ofSetColor(255, 128, 0);
    ofDrawCylinder(0, 1.0, 0, 0.15, 1);
    ofPopMatrix();
    //reset color.
    ofSetColor(0, 0, 0);
    ofDrawBox(target.x, 0, target.z, 1, 0.1, 1);
    //this draws the track of the balls
    for(int i = 0; i < balls.size(); i++) {
        balls[i]->draw();
    }
    
    //this draws the current ball
    ball.draw();
    
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
            ImGui::Text("Ball Position: {%5.2f, %5.2f, %5.2f}", ball.position.x, ball.position.y, ball.position.z);
            ImGui::Text("Ball Velocity: {%5.2f, %5.2f, %5.2f}", ball.velocity.x, ball.velocity.y, ball.velocity.z);
            ImGui::Text("Ball Energy:\n"
                        "Potential: %5.2f J\n"
                        "Kinetic: %5.2f J\n "
                        "Total: %5.2f J", ball.potentialEnergy, ball.kineticEnergy, ball.potentialEnergy + ball.kineticEnergy);
            ImGui::Text("Distance to target: %5.2f", ball.position.distance(target));
        }
        
        if (ImGui::CollapsingHeader("Graphical Output")) {
            ImGui::PlotHistogram("Height (y)", &heightLine[0], heightLine.size());
            ImGui::PlotHistogram("Horizontal (x)", &velocityLine[0], velocityLine.size());
            ImGui::PlotHistogram("Energy Error", &energyLine[0], energyLine.size());
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
        
        case 'q':                               // toggle axis unit vectors
            isAxisVisible = !isAxisVisible;
            break;
        case '1':                               // toggle grids (X)
           isXGridVisible = !isXGridVisible;
            break;
        case '2':                               // toggle grids (Y)
            isYGridVisible = !isYGridVisible;
            break;
        case '3':                               // toggle grids (Z)
            isZGridVisible = !isZGridVisible;
            break;
        case 'g':                               // toggle ground
            isGroundVisible = !isGroundVisible;
            break;
        case 'u':                               // set the up vecetor to be up (ground to be level)
            easyCam.setTarget(ofVec3f::zero());
            break;

 /*       case 'S' :                              // save gui parameters to file
            gui.saveToFile("settings.xml");

            break;
        case 'L' :                              // load gui parameters
            gui.loadFromFile("settings.xml");
            break;
*/
        case 'a':
            aim();
            break;
        case 's':
            fire();
            break;
        case 'r':
            reset();
            break;
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
    
    ofVec3f direct = target.getNormalized();
    //with the atan2 function the angle can be gathered by the vector data.
    float angle = ofRadToDeg(atan2(direct.x,direct.z)) - 90.0f;
    //correct angles below zero.
    while (angle < 0) {
        angle += 360;
    }
    direction = angle;
    float distance = target.lengthSquared();
    int calcAngle = calculateElevation(distance);
    elevation = calcAngle;
}

/**
 * The range function estimates the distance of the ball with the given angle.
 * @param e this is the given angle in degrees.
 */
float ofApp::range(float e) {
    float ux = muzzleSpeed * cos(ofDegToRad(e));
    float uy = muzzleSpeed * sin(ofDegToRad(e));
    float h = 0.5;
    float g = 0.981;
    
    float calcDistance = ( ux / g ) * ( uy + sqrt ( uy*uy + 2*g*h ) ) ;
    calcDistance *= calcDistance;
    return calcDistance;
}
/**
 * This function tries to get the right angle with trying by trying the 
 * {@code range(e)} function.
 */
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

/**
 * The fire function fires the cannon and sets the game state accordingly.
 */
void ofApp::fire() {
    ball.position = ofVec3f(0, 0.5, 0);
    ball.velocity = ofVec3f(0, 0, 0);
    ball.acceleration = ofVec3f(0, 0, 0);
    
    float rightDirection = direction + 90.0f;
    float rightElevation = 90.0f - elevation;
    
    float cosElevation = cos(ofDegToRad(rightElevation));
    float sinElevation = sin(ofDegToRad(rightElevation));
    float sinDirection = sin(ofDegToRad(rightDirection));
    float cosDirection = cos(ofDegToRad(rightDirection));
    
    float dirY = cosElevation * muzzleSpeed;
    float dirX = sinDirection * sinElevation * muzzleSpeed;
    float dirZ = cosDirection * sinElevation * muzzleSpeed;
    
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
