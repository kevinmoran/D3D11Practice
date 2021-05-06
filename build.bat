@echo off

set EXE_FILENAME=main.exe

set COMMON_COMPILER_FLAGS=/nologo /EHa- /GR- /fp:fast /Oi /W4 /Fm /Fe%EXE_FILENAME%

set DEBUG_FLAGS=/DDEBUG /DDEBUG_BUILD /Od /MTd /Zi
set RELEASE_FLAGS =/DNDEBUG /O2

set COMPILER_FLAGS=%COMMON_COMPILER_FLAGS% %DEBUG_FLAGS%
@REM set COMPILER_FLAGS=%COMMON_COMPILER_FLAGS% %RELEASE_FLAGS%

set LINKER_FLAGS=/INCREMENTAL:NO /opt:ref
set SYSTEM_LIBS=user32.lib gdi32.lib winmm.lib d3d11.lib d3dcompiler.lib

@REM Uncomment one of these to choose between normal or Single Translation Unit build 
@REM set SRC_FILES=../main.cpp ../Collision.cpp ../Player.cpp ../Camera.cpp ../ObjLoading.cpp ../D3D11Helpers.cpp
set SRC_FILES=../jumbo.cpp

set BUILD_DIR=".\build"
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%

cl %COMPILER_FLAGS% %SRC_FILES% /link %LINKER_FLAGS% %SYSTEM_LIBS%

popd

echo Done