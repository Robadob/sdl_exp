#version 120
//https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/lighting.php
varying vec3 N;
varying vec3 v;

void main(void)  
{     
   v = vec3(gl_ModelViewMatrix * gl_Vertex);       
   N = gl_Normal;//normalize(gl_NormalMatrix * );
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;  
   gl_FrontColor = gl_Color;
}