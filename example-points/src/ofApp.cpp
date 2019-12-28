#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{

	ofBackground( 0 );
	ofSetVerticalSync( true );

	//ofSetLogLevel(OF_LOG_VERBOSE);

	ofLogNotice( __FUNCTION__ ) << "Found " << ofxAzureKinect::Device::getInstalledCount() << " installed devices.";

	// Open Kinect.
	auto kinectSettings            = ofxAzureKinect::DeviceSettings();
	kinectSettings.colorResolution = K4A_COLOR_RESOLUTION_1080P;  // default = _2160P
	kinectSettings.updateIr        = false;                       // default = true
	kinectSettings.updateVbo       = false;                       // default = true
	kinectSettings.updateColor     = true;

	// more defaults:
	//kinectSettings.depthMode       = K4A_DEPTH_MODE_WFOV_2X2BINNED;
	//kinectSettings.colorFormat     = K4A_IMAGE_FORMAT_COLOR_BGRA32;
	//kinectSettings.cameraFps       = K4A_FRAMES_PER_SECOND_30;
	//kinectSettings.updateColor     = true;
	//kinectSettings.updateWorld     = true;  // ? what do i do ?
	//kinectSettings.synchronized    = true;

	if ( kinect.open( kinectSettings ) ) {
		kinect.startCameras();
	} else {
		ofLogError( __FUNCTION__ ) << "Error opening kinect device!";
	}

	// Load shader.
	ofShader::TransformFeedbackSettings settings;
	settings.shaderSources[GL_VERTEX_SHADER] = k4a_transform_vert_shader;
	settings.bindDefaults                    = false;
	settings.varyingsToCapture               = {"vPosition", "vTexCoord"};  // vec4, vec2
	transformShader.setup( settings );

	// allocate transform output buffer
	glm::ivec2 depthDims = {kinect.getDepthToWorldPix().getWidth(), kinect.getDepthToWorldPix().getHeight()};
	size_t nVerts        = depthDims.x * depthDims.y;
	size_t stride        = sizeof( glm::vec4 ) + sizeof( glm::vec2 );  // vPosition, vTexCoord
	size_t bufSz         = nVerts * stride;
	transformBuffer.allocate( bufSz, GL_STATIC_DRAW );

	// log buffer data
	std::stringstream ss;
	ss << "Allocated transform output buffer of size " << bufSz << "b" << std::endl
	   << "    xy dims: " << depthDims << "    (" << nVerts << " verts)" << std::endl
	   << "    capture out: ";
	for ( auto& v : settings.varyingsToCapture ) ss << v << " ";
	ss << "    (stride: " << stride << ")";
	ofLogNotice( __FUNCTION__ ) << ss.str();

	// base vbo
	std::vector<glm::vec3> verts( nVerts );
	transformBaseVbo.setVertexData( verts.data(), nVerts, GL_DYNAMIC_DRAW );
}

//--------------------------------------------------------------
void ofApp::update()
{

	if ( kinect.isStreaming() ) {

		// update the vbo using transform feedback shader

		transformShader.beginTransformFeedback( GL_POINTS, transformBuffer );
		this->transformShader.setUniformTexture( "uDepthTex", kinect.getDepthTex(), 1 );
		this->transformShader.setUniformTexture( "uWorldTex", kinect.getDepthToWorldTex(), 2 );
		this->transformShader.setUniform2i( "uFrameSize", kinect.getDepthTex().getWidth(), kinect.getDepthTex().getHeight() );

		transformBaseVbo.draw( GL_POINTS, 0, transformBaseVbo.getNumVertices() );

		transformShader.endTransformFeedback( transformBuffer );

		size_t stride = sizeof( glm::vec4 ) + sizeof( glm::vec2 );
		kinectVbo.setVertexBuffer( transformBuffer, 4, stride, 0 );
		kinectVbo.setTexCoordBuffer( transformBuffer, stride, sizeof( glm::vec4 ) );
	}

	ofSetWindowTitle( ofToString( ofGetFrameRate(), 2 ) + " FPS" );
}

//--------------------------------------------------------------
void ofApp::draw()
{

	cam.begin();
	ofEnableDepthTest();

	ofPushMatrix();
	{
		ofRotateYDeg( 180 );
		ofDrawAxis( 100.0f );

		kinect.getColorInDepthTex().bind();

		kinectVbo.draw( GL_POINTS, 0, kinectVbo.getNumVertices() );

		kinect.getColorInDepthTex().unbind();
	}
	ofPopMatrix();

	ofDisableDepthTest();
	cam.end();
}

void ofApp::exit()
{
	kinect.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed( int key )
{
}

//--------------------------------------------------------------
void ofApp::keyReleased( int key )
{
}

//--------------------------------------------------------------
void ofApp::mouseMoved( int x, int y )
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged( int x, int y, int button )
{
}

//--------------------------------------------------------------
void ofApp::mousePressed( int x, int y, int button )
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased( int x, int y, int button )
{
}

//--------------------------------------------------------------
void ofApp::mouseEntered( int x, int y )
{
}

//--------------------------------------------------------------
void ofApp::mouseExited( int x, int y )
{
}

//--------------------------------------------------------------
void ofApp::windowResized( int w, int h )
{
}

//--------------------------------------------------------------
void ofApp::gotMessage( ofMessage msg )
{
}

//--------------------------------------------------------------
void ofApp::dragEvent( ofDragInfo dragInfo )
{
}
