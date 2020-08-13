#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>
#define D3D11_NO_HELPERS
#include <d3d11_1.h>

#include <assert.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "D3D11Helpers.h"
#include "3DMaths.h"
#include "Input.h"
#include "Camera.h"

#define WINDOW_TITLE L"D3D11"

// Struct to pass data from WndProc to main loop
struct WndProcData {
    bool windowDidResize;
    bool keyIsDown[GameActionCount];
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    WndProcData* wndProcData = (WndProcData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch(msg)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            bool isDown = (msg == WM_KEYDOWN);
            if(wparam == VK_ESCAPE)
                DestroyWindow(hwnd);
            else if(wparam == 'W')
                wndProcData->keyIsDown[GameActionMoveCamFwd] = isDown;
            else if(wparam == 'A')
                wndProcData->keyIsDown[GameActionMoveCamLeft] = isDown;
            else if(wparam == 'S')
                wndProcData->keyIsDown[GameActionMoveCamBack] = isDown;
            else if(wparam == 'D')
                wndProcData->keyIsDown[GameActionMoveCamRight] = isDown;
            else if(wparam == 'E')
                wndProcData->keyIsDown[GameActionRaiseCam] = isDown;
            else if(wparam == 'Q')
                wndProcData->keyIsDown[GameActionLowerCam] = isDown;
            else if(wparam == VK_UP)
                wndProcData->keyIsDown[GameActionLookUp] = isDown;
            else if(wparam == VK_LEFT)
                wndProcData->keyIsDown[GameActionTurnCamLeft] = isDown;
            else if(wparam == VK_DOWN)
                wndProcData->keyIsDown[GameActionLookDown] = isDown;
            else if(wparam == VK_RIGHT)
                wndProcData->keyIsDown[GameActionTurnCamRight] = isDown;
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

HWND openWindow(LPCWSTR title)
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

    RECT initialRect = { 0, 0, 1024, 768 };
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
    HWND hWindow = openWindow(WINDOW_TITLE);
    if(!hWindow) return GetLastError();

    WndProcData wndProcData = {};
    SetWindowLongPtrW(hWindow, GWLP_USERDATA, (LONG)&wndProcData);
    
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

    Mesh cubeMesh = {};
    d3d11CreateMesh(d3d11Data.device, "cube.obj", &cubeMesh);

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
    
    // Create Constant Buffer
    struct Constants
    {
        mat4 modelViewProj;
    };

    ID3D11Buffer* constantBuffer;
    d3d11CreateConstantBuffer(d3d11Data.device, sizeof(Constants), & constantBuffer);

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

    // Player
    vec3 playerPos = {0,0,0};
    vec3 playerFwd = {0,0,-1};    

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
            dt = (float)(currentTimeInSeconds - previousTimeInSeconds);
            if(dt > (1.f / 60.f))
                dt = (1.f / 60.f);
        }

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

        int windowWidth, windowHeight;
        float windowAspectRatio;
        { // Get window dimensions
            RECT clientRect;
            GetClientRect(hWindow, &clientRect);
            windowWidth = clientRect.right - clientRect.left;
            windowHeight = clientRect.bottom - clientRect.top;
            windowAspectRatio = (float)windowWidth / (float)windowHeight;
        }

        if(wndProcData.windowDidResize)
        {
            d3d11Data.deviceContext->OMSetRenderTargets(0, 0, 0);
            d3d11Data.mainRenderTarget->Release();
            d3d11Data.mainRenderTargetView->Release();
            d3d11Data.msaaRenderTarget->Release();
            d3d11Data.msaaRenderTargetView->Release();
            d3d11Data.depthStencilView->Release();

            HRESULT res = d3d11Data.swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
            assert(SUCCEEDED(res));
            
            d3d11InitRenderTargetsAndDepthBuffer(&d3d11Data);
            perspectiveMat = makePerspectiveMat(windowAspectRatio, degreesToRadians(84), 0.1f, 1000.f);

            wndProcData.windowDidResize = false;
        }

        mat4 playerModelMat;
        { // Update player position
            vec3 playerRight = cross(playerFwd, {0, 1, 0});

            const float PLAYER_MOVE_SPEED = 5.f;
            const float PLAYER_MOVE_AMOUNT = PLAYER_MOVE_SPEED * dt;
            if(wndProcData.keyIsDown[GameActionMoveCamFwd])
                playerPos += playerFwd * PLAYER_MOVE_AMOUNT;
            if(wndProcData.keyIsDown[GameActionMoveCamBack])
                playerPos -= playerFwd * PLAYER_MOVE_AMOUNT;
            if(wndProcData.keyIsDown[GameActionMoveCamLeft])
                playerPos -= playerRight * PLAYER_MOVE_AMOUNT;
            if(wndProcData.keyIsDown[GameActionMoveCamRight])
                playerPos += playerRight * PLAYER_MOVE_AMOUNT;

            playerModelMat = scaleMat({1,1,1}) * translationMat(playerPos);
        }

        mat4 viewMat;
        if(freeCam)
            viewMat = cameraUpdateFreeCam(&camera, wndProcData.keyIsDown, dt);
        else 
            viewMat = cameraUpdateFollowPlayer(&camera, playerPos);
        mat4 viewPerspectiveMat = viewMat * perspectiveMat;

        // Spin the cube
        const int NUM_CUBES = 5;
        vec3 cubePositions[NUM_CUBES] = {
            {4,0,-6},
            {-1,2,-5},
            {3,1,-8},
            {-0.5,0.2, 6},
            {0,-1,0},
        };
        vec3 cubeScales[NUM_CUBES] = {
            {3,3,3},
            {1,2,5},
            {3,1,8},
            {5,0.2, 6},
            {20,0.5,20},
        };

        mat4 cubeModelMats[NUM_CUBES];
        for(int i=0; i<NUM_CUBES; ++i)
            cubeModelMats[i] = scaleMat(cubeScales[i]) * translationMat(cubePositions[i]);

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

        d3d11Data.deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

        d3d11Data.deviceContext->PSSetShaderResources(0, 1, &cubeTexture.d3dShaderResourceView);
        d3d11Data.deviceContext->PSSetSamplers(0, 1, &samplerState);

        d3d11Data.deviceContext->IASetVertexBuffers(0, 1, &cubeMesh.vertexBuffer, &cubeMesh.stride, &cubeMesh.offset);
        d3d11Data.deviceContext->IASetIndexBuffer(cubeMesh.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

        { // Draw player
            Constants constants = { playerModelMat * viewMat * perspectiveMat };
            d3d11OverwriteConstantBuffer(d3d11Data.deviceContext, constantBuffer, &constants, sizeof(Constants));

            d3d11Data.deviceContext->DrawIndexed(cubeMesh.numIndices, 0, 0);
        }
        
        for(int i=0; i<NUM_CUBES; ++i) {
            Constants constants = { cubeModelMats[i] * viewMat * perspectiveMat };
            d3d11OverwriteConstantBuffer(d3d11Data.deviceContext, constantBuffer, &constants, sizeof(Constants));

            d3d11Data.deviceContext->DrawIndexed(cubeMesh.numIndices, 0, 0);
        }

        d3d11Data.deviceContext->ResolveSubresource(d3d11Data.mainRenderTarget, 0, d3d11Data.msaaRenderTarget, 0, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);

        d3d11Data.swapChain->Present(1, 0);
    }

    depthStencilState->Release();
    rasterizerState->Release();
    constantBuffer->Release();
    cubeTexture.d3dShaderResourceView->Release();
    samplerState->Release();
    cubeMesh.indexBuffer->Release();
    cubeMesh.vertexBuffer->Release();
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
