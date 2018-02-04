@echo off
SET glsl="E:\C Libs\VulkanSDK\1.0.65.1\Bin\glslangValidator.exe"
%glsl% -V shader.vert
%glsl% -V shader.frag
pause