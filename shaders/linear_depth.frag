#version 430
//This shader outputs a normalised (0-1) linear depth to a single component color attachment
//We use a color attachment as imageLoad() & imageStore() do not support depth/stencil texture formats
in vec3 vertex;

out float fragColor;

uniform mat4 _projectionMat;

void main()
{
    //Depth in the normalised range 0 to +1
    float depthNorm = gl_FragCoord.z;
    //Depth in device range -1 to +1
    float depthDevice = (depthNorm * 2.0) - 1.0;
    //Linear depth in range znear - zfar
    //http://stackoverflow.com/a/16597492/1646387
    vec4 unprojected = inverse(_projectionMat) * vec4(0, 0, depthDevice, 1.0);
    float depthView = -(unprojected.z / unprojected.w);//Negate why? Is it because using ortho matrix
    //Get projection near/far planes from column major matrices
    //http://stackoverflow.com/a/12926655/1646387
    int isOrtho = 1;//int(_projectionMat[3][3]);//This coord is 1 for ortho, 0 for proj
    int isProj = 0;//int(-_projectionMat[2][3]);//This coord is -1 for proj, 0 for ortho
    float zNear = (isOrtho*((1.0f + _projectionMat[3][2]) / _projectionMat[2][2]))+(isProj*(_projectionMat[3][2] / (_projectionMat[2][2] - 1.0f)));
    float zFar = (isOrtho*(-(1.0f - _projectionMat[3][2]) / _projectionMat[2][2]))+(isProj*(_projectionMat[3][2] / (_projectionMat[2][2] + 1.0f)));
    //Linear depth in range 0-1
	fragColor = (depthView-zNear)/(zFar-zNear);
}