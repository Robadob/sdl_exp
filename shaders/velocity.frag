#version 430
//Based on: https://github.com/NVIDIAGameWorks/GraphicsSamples/blob/master/samples/es3aep-kepler/MotionBlurAdvanced/assets/shaders/scenevelocity.frag
//Which may use the formula from: http://graphics.cs.williams.edu/papers/MotionBlurI3D12/
//A non generalised version of this could also calculate fragment color during this pass
in vec4 vertex;
in vec4 prevVertex;

// OUTPUT
out vec3 velocity;

// UNIFORMS

// This value represents the (1/2 * exposure * framerate) part of the qX
// equation.
// NOTE ABOUT EXPOSURE: It represents the fraction of time that the exposure
//                      is open in the camera during a frame render. Larger
//                      values create more motion blur, smaller values create
//                      less blur. Arbitrarily defined by paper authors' in
//                      their source code as 75% and controlled by a slider
//                      in their demo.
uniform float u_fHalfExposureXFramerate;
uniform float u_fK;

// Constants
#define EPSILON 0.001
const vec2 VHALF =  vec2(0.5, 0.5);
const vec2 VONE  =  vec2(1.0, 1.0);
const vec2 VTWO  =  vec2(2.0, 2.0);

// Bias/scale helper functions
vec2 readBiasScale(vec2 v)
{
    return v * VTWO - VONE;
}

vec2 writeBiasScale(vec2 v)
{
    return (v + VONE) * VHALF;
}

void main()
{
    vec2 vQX = ((vertex.xy / vertex.w)
              - (prevVertex.xy/ prevVertex.w))
              * u_fHalfExposureXFramerate;
    float fLenQX = length(vQX);

    float fWeight = max(0.5, min(fLenQX, u_fK));
    fWeight /= ( fLenQX + EPSILON );
    
    vQX *= fWeight;
    
    velocity = vec3(writeBiasScale(vQX), 0.5);
}