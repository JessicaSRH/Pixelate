@echo off

set "ESC="
for /f %%A in ('echo prompt $E ^| cmd') do set "ESC=%%A"
@echo off
setlocal EnableDelayedExpansion

set ERRORFLAG=0

for %%f in (*.glsl) do (

	set FILENAME=%%~nf

	set ERROFLAG_FOR_FILE=0

	glslc -DVERTEX_SHADER -fshader-stage=vertex -o spirv_!FILENAME!_vertex.spv %%f
	if !errorlevel! neq 0 (
		echo %ESC%[31mVertex shader compilation failed for %%f.%ESC%[0m
		echo.
		set ERRORFLAG=1
		set ERROFLAG_FOR_FILE=1
	)

	glslc -DFRAGMENT_SHADER -fshader-stage=fragment -o spirv_!FILENAME!_fragment.spv %%f
	if !errorlevel! neq 0 (
		echo %ESC%[31mFragment shader compilation failed for %%f.%ESC%[0m
		echo.
		set ERRORFLAG=1
		set ERROFLAG_FOR_FILE=1
	)

	if !ERROFLAG_FOR_FILE! == 0 (
		echo %ESC%[32m%%f compiled successfully.%ESC%[0m
	)
)

if !ERRORFLAG! == 1 (
	echo %ESC%[31mError compiling shaders.%ESC%[0m
) else (
	echo.
	echo %ESC%[32mAll shaders compiled successfully.%ESC%[0m
)

pause