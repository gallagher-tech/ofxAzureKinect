#pragma once

#include "ofMain.h"
#include "ofxAzureKinect.h"

static const std::string k4a_transform_vert_shader =
R"(
#version 330

// -----------------------------------------------------------------------
// Azure Kinect point cloud - vertex shader
// * transform vertices by depth image
// * uses depth -> world texture map
//
// adapted from https://github.com/prisonerjohn/ofxAzureKinect/tree/master/example-shader
// -----------------------------------------------------------------------

// oF input

in vec4  position;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;

// custom input

uniform sampler2DRect uDepthTex;		// depth data - GL_R16 / unsigned short millimeters
uniform sampler2DRect uWorldTex;		// transformation from depth space to kinect world space
uniform ivec2 uFrameSize;				// texture dims

out vec4 vPosition;
out vec2 vTexCoord;
flat out int vValid;

int is_true(float b){
	return int(abs(sign(b)));	// 1 if b != 0
}
int is_false(float b){
	return 1-is_true(b);	// 1 if b == 0
}

void main()
{
	// our texture coordinate in the depth frame
    vTexCoord	= vec2(gl_VertexID % uFrameSize.x, gl_VertexID / uFrameSize.x);

    float depth	= texture(uDepthTex, vTexCoord).r;
    vec4 ray	= texture(uWorldTex, vTexCoord);

	// flag invalid data (0 depth or no transform map)
    vValid = is_true( depth * ray.x * ray.y );			// 1 (valid) or 0

    vec4 posWorld = vec4(1);
    posWorld.z	  = depth * 65535.0;		// Remap 0-1 to float range (millimeters)
	posWorld.xy	  = ray.xy * posWorld.z;	// transform xy (projective scale)
	
	// this sets posWorld.xyz to (ray.xy, 0) if invalid:
	posWorld.xyz  *= is_true( vValid );				// zeroes xyz if invalid
	posWorld.xy	  += ray.xy * is_false( vValid );	// sets xy to ray.xy if invalid

    // Flip X as OpenGL and K4A have different conventions on which direction is positive.
    posWorld.xy *= -1;

	// our position output, in kinect camera space (mm, world space with kinect as origin)
    vPosition =  posWorld;	// modelViewMatrix * posWorld;

	//gl_Position = modelViewProjectionMatrix * posWorld;		// reproject into kinect clip space for rendering
}


)";

class ofApp : public ofBaseApp
{

public:
	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed( int key );
	void keyReleased( int key );
	void mouseMoved( int x, int y );
	void mouseDragged( int x, int y, int button );
	void mousePressed( int x, int y, int button );
	void mouseReleased( int x, int y, int button );
	void mouseEntered( int x, int y );
	void mouseExited( int x, int y );
	void windowResized( int w, int h );
	void dragEvent( ofDragInfo dragInfo );
	void gotMessage( ofMessage msg );

	ofEasyCam cam;

	ofxAzureKinect::Device kinect;
	ofVbo kinectVbo;

	ofShader transformShader;
	ofBufferObject transformBuffer;
	ofVbo transformBaseVbo;
};
