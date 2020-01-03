#pragma once

#include <k4a/k4atypes.h>

#ifdef OFXAZUREKINECT_BODYSDK
#include <k4abttypes.h>
#endif

#include "ofVectorMath.h"

inline const glm::vec2 & toGlm(const k4a_float2_t & v) 
{
	return *reinterpret_cast<const glm::vec2*>(&v);
}

inline const glm::vec3 & toGlm(const k4a_float3_t & v)
{
	return *reinterpret_cast<const glm::vec3*>(&v);
}

#ifdef OFXAZUREKINECT_BODYSDK
inline const glm::quat toGlm(const k4a_quaternion_t & q)
{
	return glm::quat(q.v[0], q.v[1], q.v[2], q.v[3]);
}
#endif

namespace ofxAzureKinect
{
	typedef k4a_depth_mode_t DepthMode;
	typedef k4a_color_resolution_t ColorResolution;
	typedef k4a_image_format_t ImageFormat;
	typedef k4a_fps_t FramesPerSecond;

#ifdef OFXAZUREKINECT_BODYSDK
	typedef k4abt_sensor_orientation_t SensorOrientation;
	typedef k4abt_tracker_processing_mode_t ProcessingMode;
#endif

	static const std::string pointcloud_vert_shader = R"(
		#version 150

		// -----------------------------------------------------------------------
		// Azure Kinect point cloud - vertex shader
		// * transform vertices from depth texture -> world map -> world space
		// adapted by @tyhenry from https://github.com/prisonerjohn/ofxAzureKinect/tree/master/example-shader
		// -----------------------------------------------------------------------

		in vec4  position;						// oF
		uniform mat4 modelViewMatrix;			// oF
		uniform mat4 modelViewProjectionMatrix;	// oF

		uniform sampler2DRect uDepthTex;		// depth data - GL_R16 / unsigned short millimeters
		uniform sampler2DRect uWorldTex;		// transformation from depth space to kinect world space
		uniform ivec2 uFrameSize;				// texture dims

		out vec3 vPosition;						// world space position
		out vec2 vTexCoord;						// color UV map coords
		flat out int vValid;					// flag valid / invalid point

		int is_true(float b)	{ return int(abs(sign(b))); }	// b != 0 ? 1 : 0
		int is_false(float b)	{ return 1-is_true(b); }		// b == 0 ? 1 : 0

		void main()
		{
			vTexCoord	= vec2(gl_VertexID % uFrameSize.x, gl_VertexID / uFrameSize.x);
			float depth	= texture( uDepthTex, vTexCoord ).r * 65535.0;	// remap 0-1 to short range ( millimeters )
			vec4 ray	= texture( uWorldTex, vTexCoord );				// depth to world projection map
			vValid		= is_true( depth * ray.x * ray.y );				// flag valid (1) / invalid (0)

			vec4 pos	= vec4( ray.xy * depth, depth, 1.);
			pos.xyz		*= is_true( vValid );				// zeroes xyz if invalid
			pos.xy		+= ray.xy * is_false( vValid );		// sets xy to ray.xy if invalid

			pos.xy		*= -1;	// flip xy to account for OpenGL <--> K4A axes
			vPosition	=  pos;
			gl_Position = modelViewProjectionMatrix * pos;	// reproject into kinect clip space for rendering
		}
	)";

}
