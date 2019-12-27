#pragma once

// uncomment to include Body Tracking SDK (must be installed on system):
// #define OFXAZUREKINECT_BODYSDK

#include "ofxAzureKinect/Device.h"
#include "ofxAzureKinect/Types.h"

// include the static body tracking library if we are using it
#ifdef OFXAZUREKINECT_BODYSDK
#pragma comment(lib, "k4abt.lib")
#endif

