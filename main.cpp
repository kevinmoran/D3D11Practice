#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#define D3D11_NO_HELPERS
#include <d3d11_1.h>

#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "types.h"
#include "3DMaths.h"
#include "D3D11Helpers.h"
#include "Input.h"
#include "ObjLoading.h"
#include "Camera.h"
#include "Player.h"
#include "Collision.h"

#define WINDOW_TITLE L"D3D11"

// Struct to pass data from WndProc to main loop
struct WndProcData {
    bool windowDidResize;
    KeyState keys[KEY_COUNT];
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    WndProcData* wndProcData = (WndProcData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch(msg)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            KeyState* keys = wndProcData->keys;
            bool isDown = ((lparam & (1 << 31)) == 0);
            // A - Z
            if(wparam >= 'A' && wparam <= 'Z')
                keys[KEY_A + wparam - 'A'].isDown = isDown;
            // 0 - 9
            else if(wparam >= '0' && wparam <= '9')
                keys[KEY_0 + wparam - '0'].isDown = isDown;
            // Numpad 0 - 9
            else if(wparam >= VK_NUMPAD0 && wparam <= VK_NUMPAD9)
                keys[KEY_NUMPAD_0 + wparam - VK_NUMPAD0].isDown = isDown;
            // F keys
            else if(wparam >= VK_F1 && wparam <= VK_F12)
                keys[KEY_F1 + wparam - VK_F1].isDown = isDown;
    
            else if(wparam == VK_BACK)
                keys[KEY_BACKSPACE].isDown = isDown;
            else if(wparam == VK_TAB)
                keys[KEY_TAB].isDown = isDown;
            else if(wparam == VK_RETURN)
                keys[KEY_ENTER].isDown = isDown;
            else if(wparam == VK_SHIFT)
                keys[KEY_SHIFT].isDown = isDown;
            else if(wparam == VK_CONTROL)
                keys[KEY_CTRL].isDown = isDown;
            else if(wparam == VK_MENU)
                keys[KEY_ALT].isDown = isDown;
            // The pause key doesn't work for the simple style of keyboard input,
            // it seems to immediately send a WM_KEYUP message after the WM_KEYDOWN,
            // even if you hold the key, so this flag will just get immediately
            // reset before the game can see it.
            // else if(wparam == VK_PAUSE)
                // keys[KEY_PAUSE].isDown = isDown;
            else if(wparam == VK_CAPITAL)
                keys[KEY_CAPSLOCK].isDown = isDown;
            else if(wparam == VK_ESCAPE)
                keys[KEY_ESC].isDown = isDown;
            else if(wparam == VK_SPACE)
                keys[KEY_SPACE].isDown = isDown;
            else if(wparam == VK_PRIOR)
                keys[KEY_PGUP].isDown = isDown;
            else if(wparam == VK_NEXT)
                keys[KEY_PGDN].isDown = isDown;
            else if(wparam == VK_HOME)
                keys[KEY_HOME].isDown = isDown;
            else if(wparam == VK_END)
                keys[KEY_END].isDown = isDown;
            else if(wparam == VK_LEFT)
                keys[KEY_LEFT].isDown = isDown;
            else if(wparam == VK_UP)
                keys[KEY_UP].isDown = isDown;
            else if(wparam == VK_RIGHT)
                keys[KEY_RIGHT].isDown = isDown;
            else if(wparam == VK_DOWN)
                keys[KEY_DOWN].isDown = isDown;
            // The print screen key seems to only send a WM_KEYUP so this doesn't work. 
            // else if(wparam == VK_SNAPSHOT)
                // keys[KEY_PRINT_SCREEN].isDown = isDown;
            else if(wparam == VK_INSERT)
                keys[KEY_INSERT].isDown = isDown;
            else if(wparam == VK_DELETE)
                keys[KEY_DELETE].isDown = isDown;
            else if(wparam == VK_ADD)
                keys[KEY_NUMPAD_ADD].isDown = isDown;
            else if(wparam == VK_SUBTRACT)
                keys[KEY_NUMPAD_SUBTRACT].isDown = isDown;
            else if(wparam == VK_DECIMAL)
                keys[KEY_NUMPAD_DECIMAL].isDown = isDown;
            else if(wparam == VK_DIVIDE)
                keys[KEY_NUMPAD_DIVIDE].isDown = isDown;
            else if(wparam == VK_OEM_1)
                keys[KEY_SEMICOLON].isDown = isDown;
            else if(wparam == VK_OEM_PLUS)
                keys[KEY_PLUS].isDown = isDown;
            else if(wparam == VK_OEM_COMMA)
                keys[KEY_COMMA].isDown = isDown;
            else if(wparam == VK_OEM_MINUS)
                keys[KEY_MINUS].isDown = isDown;
            else if(wparam == VK_OEM_PERIOD)
                keys[KEY_PERIOD].isDown = isDown;
            else if(wparam == VK_OEM_2)
                keys[KEY_SLASH].isDown = isDown;
            else if(wparam == VK_OEM_3)
                keys[KEY_GRAVE_ACCENT].isDown = isDown;
            else if(wparam == VK_OEM_4)
                keys[KEY_LEFT_BRACKET].isDown = isDown;
            else if(wparam == VK_OEM_5)
                keys[KEY_BACKSLASH].isDown = isDown;
            else if(wparam == VK_OEM_6)
                keys[KEY_RIGHT_BRACKET].isDown = isDown;
            else if(wparam == VK_OEM_7)
                keys[KEY_APOSTROPHE].isDown = isDown;

            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_SIZE:
        {
            if(wndProcData)
                wndProcData->windowDidResize = true;
            break;
        }
        default:
            result = DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return result;
}

HWND openWindow(LPCWSTR title, int windowWidth, int windowHeight)
{
    WNDCLASSEXW winClass = {};
    winClass.cbSize = sizeof(WNDCLASSEXW);
    winClass.style = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc = &WndProc;
    winClass.hInstance = GetModuleHandleW(NULL);
    winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
    winClass.hCursor = LoadCursorW(0, IDC_ARROW);
    winClass.lpszClassName = title;
    winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);

    if(!RegisterClassExW(&winClass)) {
        MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
        return NULL;
    }

    RECT initialRect = { 0, 0, windowWidth, windowHeight };
    AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
    LONG initialWidth = initialRect.right - initialRect.left;
    LONG initialHeight = initialRect.bottom - initialRect.top;

    HWND result = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                            winClass.lpszClassName,
                            title,
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            initialWidth, 
                            initialHeight,
                            0, 0, winClass.hInstance, 0);

    if(!result) {
        MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
    }
    return result;
}

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
    int windowWidth = 1024;
    int windowHeight = 768;
    float windowAspectRatio = (float)windowWidth / windowHeight;
    HWND hWindow = openWindow(WINDOW_TITLE, windowWidth, windowHeight);
    if(!hWindow) return GetLastError();

    WndProcData wndProcData = {};
    SetWindowLongPtrW(hWindow, GWLP_USERDATA, (LONG)&wndProcData);

    LONGLONG startPerfCount = 0;
    LONGLONG perfCounterFrequency = 0;
    { // Initialise timing data
        LARGE_INTEGER perfCount;
        QueryPerformanceCounter(&perfCount);
        startPerfCount = perfCount.QuadPart;
        LARGE_INTEGER perfFreq;
        QueryPerformanceFrequency(&perfFreq);
        perfCounterFrequency = perfFreq.QuadPart;
    }
    double currentTimeInSeconds = 0.0;
    
    D3D11Data d3d11Data;
    d3d11Init(hWindow, &d3d11Data);
    // TODO: WASAPI init
    // TODO? RawInput init?

    // Create Vertex Shader and Input Layout
    ID3D11VertexShader* vertexShader;
    ID3D11InputLayout* inputLayout;
    {
        // TODO: Parse this from vertex shader code!
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        d3d11CreateVertexShaderAndInputLayout(d3d11Data.device, L"shaders.hlsl", "vs_main", &vertexShader, inputElementDesc, ARRAYSIZE(inputElementDesc), &inputLayout);
    }

    // Create Pixel Shader
    ID3D11PixelShader* pixelShader;
    d3d11CreatePixelShader(d3d11Data.device, L"shaders.hlsl", "ps_main", &pixelShader);

    LoadedObj cubeObj = loadObj("cube.obj");

    Mesh cubeMesh = {};
    d3d11CreateMesh(d3d11Data.device, cubeObj, &cubeMesh);
    
    Mesh playerMesh = {};
    d3d11CreateMesh(d3d11Data.device, cubeObj, &playerMesh);

    ColliderData cubeColliderData = createColliderData(cubeObj);
    
    freeLoadedObj(cubeObj);

    Texture cubeTexture = {};
    d3d11CreateTexture(d3d11Data.device, d3d11Data.deviceContext, "test.png", &cubeTexture);

    // Create Sampler State
    ID3D11SamplerState* samplerState;
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MaxLOD         = D3D11_FLOAT32_MAX;

        d3d11Data.device->CreateSamplerState(&samplerDesc, &samplerState);
    }
    
    struct PerObjectVSConstants
    {
        mat4 modelViewProj;
    };
    struct PerObjectPSConstants
    {
        vec4 tintColour;
    };

    ID3D11Buffer* perObjectVSConstantBuffer;
    d3d11CreateConstantBuffer(d3d11Data.device, sizeof(PerObjectVSConstants), & perObjectVSConstantBuffer);
    ID3D11Buffer* perObjectPSConstantBuffer;
    d3d11CreateConstantBuffer(d3d11Data.device, sizeof(PerObjectPSConstants), & perObjectPSConstantBuffer);

    ID3D11RasterizerState* rasterizerState;
    {
        D3D11_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_BACK;
        rasterizerDesc.FrontCounterClockwise = TRUE;

        d3d11Data.device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    }

    ID3D11DepthStencilState* depthStencilState;
    {
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable    = TRUE;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;

        d3d11Data.device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
    }

    // Camera
    Camera camera = cameraInit({0, 1, 3},  {0, 0, 0});

    bool freeCam = false;

    mat4 perspectiveMat = {};
    wndProcData.windowDidResize = true; // To force initial perspectiveMat calculation

    Player player = playerInit({0,0,0}, normalise({0,0,1}));
    ColliderData playerColliderData = cubeColliderData;

    const int NUM_CUBES = 5;
    vec3 cubePositions[NUM_CUBES] = {
        {4,0,0},
        {-1,2,-5},
        {3,1,-8},
        {-0.5,0.2,6},
        {0,-1,0},
    };
    vec3 cubeScales[NUM_CUBES] = {
        {3,3,3},
        {1,2,5},
        {3,1,8},
        {5,0.2,6},
        {20,0.5,20},
    };

    mat4 cubeModelMats[NUM_CUBES];
    ColliderData cubeColliderDatas[NUM_CUBES];
    for(int i=0; i<NUM_CUBES; ++i) {
        cubeModelMats[i] = scaleMat(cubeScales[i]) * translationMat(cubePositions[i]);
        mat3 invModelMat = scaleMat3(1/cubeScales[i]);
        cubeColliderDatas[i] = cubeColliderData;
        cubeColliderDatas[i].modelMatrix = cubeModelMats[i];
        cubeColliderDatas[i].normalMatrix = transpose(invModelMat);
    }
        
    float timeStepMultiplier = 1.f;

    // Main Loop
    bool isRunning = true;
    while(isRunning)
    {
        float dt;
        {
            double previousTimeInSeconds = currentTimeInSeconds;
            LARGE_INTEGER perfCount;
            QueryPerformanceCounter(&perfCount);

            currentTimeInSeconds = (double)(perfCount.QuadPart - startPerfCount) / (double)perfCounterFrequency;
            dt = CLAMP_BELOW((float)(currentTimeInSeconds - previousTimeInSeconds), 1.f/60.f);
        }

        keysUpdateWasDownState(wndProcData.keys, KEY_COUNT);

        { // Process Windows message queue
            MSG msg = {};
            while(PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
            {
                if(msg.message == WM_QUIT)
                    isRunning = false;
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }

        if(wndProcData.keys[KEY_ESC].isDown)
            break;

        if(wndProcData.windowDidResize)
        {
            { // Get window dimensions
                RECT clientRect;
                GetClientRect(hWindow, &clientRect);
                windowWidth = clientRect.right - clientRect.left;
                windowHeight = clientRect.bottom - clientRect.top;
                windowAspectRatio = (float)windowWidth / (float)windowHeight;
            }

            d3d11Data.deviceContext->OMSetRenderTargets(0, 0, 0);
            d3d11Data.mainRenderTarget->Release();
            d3d11Data.mainRenderTargetView->Release();
            d3d11Data.msaaRenderTarget->Release();
            d3d11Data.msaaRenderTargetView->Release();
            d3d11Data.depthStencilView->Release();

            HRESULT res = d3d11Data.swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
            assert(SUCCEEDED(res));
            
            d3d11InitRenderTargetsAndDepthBuffer(&d3d11Data);
            perspectiveMat = makePerspectiveMat(windowAspectRatio, degreesToRadians(100), 0.1f, 1000.f);

            wndProcData.windowDidResize = false;
        }

        if(wndProcData.keys[KEY_TAB].wentDown())
            freeCam = !freeCam;
        if(wndProcData.keys[KEY_R].wentDown()) 
            player = playerInit({}, {0,0,1});
        if(wndProcData.keys[KEY_MINUS].wentDown())
            timeStepMultiplier = CLAMP_ABOVE(timeStepMultiplier*0.5f, 0.25f);
        if(wndProcData.keys[KEY_PLUS].wentDown())
            timeStepMultiplier = CLAMP_BELOW(timeStepMultiplier*2.f, 2.f);

        mat4 playerModelMat;
        if(freeCam) {
            playerModelMat = calculateModelMatrix(player);
        }
        else {
            playerModelMat = playerUpdate(&player, wndProcData.keys, camera.fwd, dt*timeStepMultiplier);
        }
        
        playerColliderData.modelMatrix = playerModelMat;
        playerColliderData.normalMatrix = calculateNormalMatrix(player);

        vec4 cubeTintColours[NUM_CUBES] = {};
        // Collision Detection
        for(u32 i=0; i<NUM_CUBES; ++i)
        {
            SATResult result = checkCollision(playerColliderData, cubeColliderDatas[i]);
            if(result.isColliding){
                cubeTintColours[i] = {0.1f, 0.8f, 0.2f, 1.f};
                player.pos += result.normal * result.minDistance;
            }
            else {
                cubeTintColours[i] = {1,1,1,1};
            }
        }
        playerModelMat = calculateModelMatrix(player);

        mat4 viewMat;
        if(freeCam) {
            viewMat = cameraUpdateFreeCam(&camera, wndProcData.keys, dt*timeStepMultiplier);
        }
        else {
            viewMat = cameraUpdateFollowPlayer(&camera, player.pos);
        }

        mat4 viewPerspectiveMat = viewMat * perspectiveMat;

        FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
        d3d11Data.deviceContext->ClearRenderTargetView(d3d11Data.msaaRenderTargetView, backgroundColor);
        
        d3d11Data.deviceContext->ClearDepthStencilView(d3d11Data.depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

        D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT)windowWidth, (FLOAT)windowHeight, 0.0f, 1.0f };
        d3d11Data.deviceContext->RSSetViewports(1, &viewport);

        d3d11Data.deviceContext->RSSetState(rasterizerState);
        d3d11Data.deviceContext->OMSetDepthStencilState(depthStencilState, 0);

        d3d11Data.deviceContext->OMSetRenderTargets(1, &d3d11Data.msaaRenderTargetView, d3d11Data.depthStencilView);

        d3d11Data.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        d3d11Data.deviceContext->IASetInputLayout(inputLayout);

        d3d11Data.deviceContext->VSSetShader(vertexShader, nullptr, 0);
        d3d11Data.deviceContext->PSSetShader(pixelShader, nullptr, 0);

        d3d11Data.deviceContext->VSSetConstantBuffers(0, 1, &perObjectVSConstantBuffer);
        d3d11Data.deviceContext->PSSetConstantBuffers(0, 1, &perObjectPSConstantBuffer);

        d3d11Data.deviceContext->PSSetShaderResources(0, 1, &cubeTexture.d3dShaderResourceView);
        d3d11Data.deviceContext->PSSetSamplers(0, 1, &samplerState);

        { // Draw player
            d3d11Data.deviceContext->IASetVertexBuffers(0, 1, &playerMesh.vertexBuffer, &playerMesh.stride, &playerMesh.offset);
            d3d11Data.deviceContext->IASetIndexBuffer(playerMesh.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

            PerObjectVSConstants vsConstants = { playerModelMat * viewPerspectiveMat };
            d3d11OverwriteConstantBuffer(d3d11Data.deviceContext, perObjectVSConstantBuffer, &vsConstants, sizeof(PerObjectVSConstants));

            PerObjectPSConstants psConstants = { {0.8f, 0.1f, 0.3f, 1.0f} };
            d3d11OverwriteConstantBuffer(d3d11Data.deviceContext, perObjectPSConstantBuffer, &psConstants, sizeof(PerObjectPSConstants));

            d3d11Data.deviceContext->DrawIndexed(playerMesh.numIndices, 0, 0);
        }

        d3d11Data.deviceContext->IASetVertexBuffers(0, 1, &cubeMesh.vertexBuffer, &cubeMesh.stride, &cubeMesh.offset);
        d3d11Data.deviceContext->IASetIndexBuffer(cubeMesh.indexBuffer, DXGI_FORMAT_R16_UINT, 0);
        
        for(int i=0; i<NUM_CUBES; ++i) {
            PerObjectVSConstants vsConstants = { cubeModelMats[i] * viewPerspectiveMat};
            d3d11OverwriteConstantBuffer(d3d11Data.deviceContext, perObjectVSConstantBuffer, &vsConstants, sizeof(PerObjectVSConstants));
        
            PerObjectPSConstants psConstants = { cubeTintColours[i] };
            d3d11OverwriteConstantBuffer(d3d11Data.deviceContext, perObjectPSConstantBuffer, &psConstants, sizeof(PerObjectPSConstants));

            d3d11Data.deviceContext->DrawIndexed(cubeMesh.numIndices, 0, 0);
        }

        d3d11Data.deviceContext->ResolveSubresource(d3d11Data.mainRenderTarget, 0, d3d11Data.msaaRenderTarget, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

        d3d11Data.swapChain->Present(1, 0);
    }

    depthStencilState->Release();
    rasterizerState->Release();
    perObjectPSConstantBuffer->Release();
    perObjectVSConstantBuffer->Release();
    cubeTexture.d3dShaderResourceView->Release();
    samplerState->Release();
    cubeMesh.indexBuffer->Release();
    cubeMesh.vertexBuffer->Release();
    playerMesh.indexBuffer->Release();
    playerMesh.vertexBuffer->Release();
    pixelShader->Release();
    inputLayout->Release();
    vertexShader->Release();
    d3d11Data.depthStencilView->Release();
    d3d11Data.msaaRenderTarget->Release();
    d3d11Data.msaaRenderTargetView->Release();
    d3d11Data.mainRenderTarget->Release();
    d3d11Data.mainRenderTargetView->Release();
    d3d11Data.swapChain->Release();
    d3d11Data.deviceContext->Release();
    d3d11Data.device->Release();

    return 0;
}
