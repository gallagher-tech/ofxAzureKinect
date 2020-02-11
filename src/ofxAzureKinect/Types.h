#pragma once

#include <k4a/k4atypes.h>

#ifdef OFXAZUREKINECT_BODYSDK
#include <k4abttypes.h>
#endif

#include "ofVectorMath.h"

inline const glm::vec2& toGlm( const k4a_float2_t& v )
{
	return *reinterpret_cast<const glm::vec2*>( &v );
}

inline const glm::vec3& toGlm( const k4a_float3_t& v )
{
	return *reinterpret_cast<const glm::vec3*>( &v );
}

#ifdef OFXAZUREKINECT_BODYSDK
inline const glm::quat toGlm( const k4a_quaternion_t& q )
{
	return glm::quat( q.v[0], q.v[1], q.v[2], q.v[3] );
}
#endif

namespace helpers {

// Helper - RAII class to auto cleanup / do something on early return
struct scope_guard
{
	scope_guard( const std::function<void()>& f )
	    : _f( f ) {}
	~scope_guard()
	{
		if ( _f ) _f();
	}

protected:
	std::function<void()> _f;
};

}  // namespace helpers

namespace ofxAzureKinect {
typedef k4a_depth_mode_t DepthMode;
typedef k4a_color_resolution_t ColorResolution;
typedef k4a_image_format_t ImageFormat;
typedef k4a_fps_t FramesPerSecond;

template <typename T>
inline const std::map<T, std::string>& type_to_str_map()
{
	return {};
}

template <>
inline const std::map<k4a_depth_mode_t, std::string>& type_to_str_map()
{
	static const std::map<k4a_depth_mode_t, std::string> names = {
	    { K4A_DEPTH_MODE_NFOV_2X2BINNED, "K4A_DEPTH_MODE_NFOV_2X2BINNED" }, /**< Depth captured at 320x288. Passive IR is also captured at 320x288. */
	    { K4A_DEPTH_MODE_NFOV_UNBINNED, "K4A_DEPTH_MODE_NFOV_UNBINNED" },   /**< Depth captured at 640x576. Passive IR is also captured at 640x576. */
	    { K4A_DEPTH_MODE_WFOV_2X2BINNED, "K4A_DEPTH_MODE_WFOV_2X2BINNED" }, /**< Depth captured at 512x512. Passive IR is also captured at 512x512. */
	    { K4A_DEPTH_MODE_WFOV_UNBINNED, "K4A_DEPTH_MODE_WFOV_UNBINNED" },   /**< Depth captured at 1024x1024. Passive IR is also captured at 1024x1024. */
	    { K4A_DEPTH_MODE_PASSIVE_IR, "K4A_DEPTH_MODE_PASSIVE_IR" },         /**< Passive IR only: return {};captured at 1024x1024. */
	    { K4A_DEPTH_MODE_OFF, "K4A_DEPTH_MODE_OFF" } };                     /**< Depth sensor will be turned off with this setting. */
	return names;
};
template <>
inline const std::map<k4a_image_format_t, std::string>& type_to_str_map()
{
	static const std::map<k4a_image_format_t, std::string> names = {
	    { K4A_IMAGE_FORMAT_COLOR_BGRA32, "K4A_IMAGE_FORMAT_COLOR_BGRA32" },
	    { K4A_IMAGE_FORMAT_DEPTH16, "K4A_IMAGE_FORMAT_DEPTH16" },
	    { K4A_IMAGE_FORMAT_IR16, "K4A_IMAGE_FORMAT_IR16" } };
	return names;
};
template <>
inline const std::map<k4a_color_resolution_t, std::string>& type_to_str_map()
{
	static const std::map<k4a_color_resolution_t, std::string> names = {
	    { K4A_COLOR_RESOLUTION_OFF, "K4A_COLOR_RESOLUTION_OFF" },       /**< Color camera will be turned off with this setting */
	    { K4A_COLOR_RESOLUTION_720P, "K4A_COLOR_RESOLUTION_720P" },     /**< 1280 * 720  16:9 */
	    { K4A_COLOR_RESOLUTION_1080P, "K4A_COLOR_RESOLUTION_1080P" },   /**< 1920 * 1080 16:9 */
	    { K4A_COLOR_RESOLUTION_1440P, "K4A_COLOR_RESOLUTION_1440P" },   /**< 2560 * 1440 16:9 */
	    { K4A_COLOR_RESOLUTION_1536P, "K4A_COLOR_RESOLUTION_1536P" },   /**< 2048 * 1536 4:3  */
	    { K4A_COLOR_RESOLUTION_2160P, "K4A_COLOR_RESOLUTION_2160P" },   /**< 3840 * 2160 16:9 */
	    { K4A_COLOR_RESOLUTION_3072P, "K4A_COLOR_RESOLUTION_3072P" } }; /**< 4096 * 3072 4:3  */
	return names;
};
template <>
inline const std::map<k4a_fps_t, std::string>& type_to_str_map()
{
	static const std::map<k4a_fps_t, std::string> names = {
	    { K4A_FRAMES_PER_SECOND_5, "K4A_FRAMES_PER_SECOND_5" },     /**< 5 FPS */
	    { K4A_FRAMES_PER_SECOND_15, "K4A_FRAMES_PER_SECOND_15" },   /**< 15 FPS */
	    { K4A_FRAMES_PER_SECOND_30, "K4A_FRAMES_PER_SECOND_30" } }; /**< 30 FPS */
	return names;
};

template <typename T>
inline std::string to_string(T type) {
	try {
		return type_to_str_map<T>().at(type);
	}
	catch (...) {}
	return "";
}
template <typename T>
inline T to_type(std::string str, const T& default_value) {
	for (auto& type : type_to_str_map<T>()) {
		if (type.second == str) {
			return type.first;
		}
		return default_value;
	}
}

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
			vPosition	=  pos.xyz;
			gl_Position = modelViewProjectionMatrix * pos;	// reproject into kinect clip space for rendering
		}
	)";

}  // namespace ofxAzureKinect
