#include "D3D11Helpers.h"

#include <d3dcompiler.h>

#include <assert.h>

#include "stb_image.h"

#include "ObjLoading.h"

int d3d11Init(HWND hWindow, D3D11Data* d3d11)
{
    { // Create D3D11 Device and Context
        ID3D11Device* baseDevice;
        ID3D11DeviceContext* baseDeviceContext;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        #if defined(DEBUG_BUILD)
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif

        HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 
                                            0, creationFlags, 
                                            featureLevels, ARRAYSIZE(featureLevels), 
                                            D3D11_SDK_VERSION, &baseDevice, 
                                            0, &baseDeviceContext);
        if(FAILED(hResult)){
            MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
            return GetLastError();
        }
        
        // Get 1.1 interface of D3D11 Device and Context
        hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&d3d11->device);
        assert(SUCCEEDED(hResult));
        baseDevice->Release();

        hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&d3d11->deviceContext);
        assert(SUCCEEDED(hResult));
        baseDeviceContext->Release();
        
#ifdef DEBUG_BUILD
        // Set up debug layer to break on D3D11 errors
        ID3D11Debug *d3dDebug = nullptr;
        d3d11->device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
        if (d3dDebug)
        {
            ID3D11InfoQueue *d3dInfoQueue = nullptr;
            if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
            {
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
                d3dInfoQueue->Release();
            }
            d3dDebug->Release();
        }
#endif
    }
   
    { // Create Swap Chain
        IDXGIFactory2* dxgiFactory;
        { // Get DXGI Factory (needed to create Swap Chain)
            IDXGIDevice1* dxgiDevice;
            HRESULT hResult = d3d11->device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
            assert(SUCCEEDED(hResult));

            IDXGIAdapter* dxgiAdapter;
            hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
            assert(SUCCEEDED(hResult));
            dxgiDevice->Release();

            DXGI_ADAPTER_DESC adapterDesc;
            dxgiAdapter->GetDesc(&adapterDesc);

            OutputDebugStringA("Graphics Device: ");
            OutputDebugStringW(adapterDesc.Description);
            OutputDebugStringA("\n");

            hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
            assert(SUCCEEDED(hResult));
            dxgiAdapter->Release();
        }
        
        DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
        d3d11SwapChainDesc.Width = 0; // use window width
        d3d11SwapChainDesc.Height = 0; // use window height
        d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        d3d11SwapChainDesc.SampleDesc.Count = 1;
        d3d11SwapChainDesc.SampleDesc.Quality = 0;
        d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        d3d11SwapChainDesc.BufferCount = 2;
        d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        d3d11SwapChainDesc.Flags = 0;

        HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(d3d11->device, hWindow, &d3d11SwapChainDesc, 0, 0, &d3d11->swapChain);
        assert(SUCCEEDED(hResult));

        dxgiFactory->Release();
    }
    
    d3d11InitRenderTargetsAndDepthBuffer(d3d11);
    
    return 0;
}

#define MSAA_NUM_SAMPLES 8

bool d3d11InitRenderTargetsAndDepthBuffer(D3D11Data* d3d11)
{
    // Get SwapChain RenderTarget
    HRESULT hResult = d3d11->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11->mainRenderTarget);
    assert(SUCCEEDED(hResult));

    // Create RenderTargetView for swapchain buffer
    // Note: Can't create an sRGB swapchain when using new flipmodel, so 
    // we specify here that the RTV format should be sRGB instead of linear
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    hResult = d3d11->device->CreateRenderTargetView(d3d11->mainRenderTarget, 0, &d3d11->mainRenderTargetView);
    assert(SUCCEEDED(hResult));

    // Create MSAA RenderTarget
    D3D11_TEXTURE2D_DESC texDesc;
    d3d11->mainRenderTarget->GetDesc(&texDesc);
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    texDesc.SampleDesc.Count = MSAA_NUM_SAMPLES;
    texDesc.SampleDesc.Quality = 0;

    hResult = d3d11->device->CreateTexture2D(&texDesc, NULL, &d3d11->msaaRenderTarget);
    assert(SUCCEEDED(hResult));

    // Create MSAA RenderTargetView
    hResult = d3d11->device->CreateRenderTargetView(d3d11->msaaRenderTarget, 0, &d3d11->msaaRenderTargetView);
    assert(SUCCEEDED(hResult));

    // Create depth stencil buffer
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    d3d11->mainRenderTarget->GetDesc(&depthBufferDesc);

    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.SampleDesc.Count = texDesc.SampleDesc.Count;
    depthBufferDesc.SampleDesc.Quality = texDesc.SampleDesc.Quality;

    ID3D11Texture2D* depthBuffer;
    d3d11->device->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

    // Create depth stencil view
    d3d11->device->CreateDepthStencilView(depthBuffer, nullptr, &d3d11->depthStencilView);

    depthBuffer->Release();

    return true;
}

enum ShaderType {
    ShaderType_INVALID,
    ShaderType_VERTEX,
    ShaderType_PIXEL
};

bool _d3d11CompileShader(LPCWSTR fileName, LPCSTR shaderEntryPoint, ID3DBlob** shaderByteCode, ShaderType shaderType)
{
    UINT shaderCompileFlags = 0;
    #if defined(DEBUG_BUILD)
    shaderCompileFlags |= D3DCOMPILE_DEBUG;
    #endif

    const char* shaderTarget = NULL;
    if(shaderType == ShaderType_VERTEX)
        shaderTarget = "vs_5_0";
    else if(shaderType == ShaderType_PIXEL)
        shaderTarget = "ps_5_0";
    else 
        return false;

    ID3DBlob* shaderCompileErrors;
    HRESULT hResult = D3DCompileFromFile(fileName, nullptr, nullptr, shaderEntryPoint, shaderTarget, shaderCompileFlags, 0, shaderByteCode, &shaderCompileErrors);
    if(FAILED(hResult))
    {
        if(hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            MessageBoxA(0, "Could not compile shader; file not found", "Shader Compiler Error", MB_ICONERROR | MB_OK);
        else if(shaderCompileErrors){
            MessageBoxA(0, (const char*)shaderCompileErrors->GetBufferPointer(), "Shader Compiler Error", MB_ICONERROR | MB_OK);
            shaderCompileErrors->Release();
        }
        
        return false;
    }
    return true;
}

bool d3d11CreateVertexShaderAndInputLayout(ID3D11Device1* device, LPCWSTR fileName, LPCSTR shaderEntryPoint, ID3D11VertexShader** vertexShader, D3D11_INPUT_ELEMENT_DESC inputElementDesc[], int numInputElements, ID3D11InputLayout** inputLayout)
{
    ID3DBlob* shaderByteCode;
    _d3d11CompileShader(fileName, shaderEntryPoint, &shaderByteCode, ShaderType_VERTEX);

    HRESULT hResult = device->CreateVertexShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), nullptr, vertexShader);
    assert(SUCCEEDED(hResult));

    hResult = device->CreateInputLayout(inputElementDesc, numInputElements, shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), inputLayout);
    assert(SUCCEEDED(hResult));

    shaderByteCode->Release();

    return true;
}

ID3D11PixelShader* d3d11CreatePixelShader(ID3D11Device1* device, LPCWSTR fileName, LPCSTR shaderEntryPoint)
{
    ID3D11PixelShader* pixelShader;
    ID3DBlob* shaderByteCode;
    _d3d11CompileShader(fileName, shaderEntryPoint, &shaderByteCode, ShaderType_PIXEL);

    HRESULT hResult = device->CreatePixelShader(shaderByteCode->GetBufferPointer(), shaderByteCode->GetBufferSize(), nullptr, &pixelShader);
    assert(SUCCEEDED(hResult));
    shaderByteCode->Release();

    return pixelShader;
}

Mesh d3d11CreateMesh(ID3D11Device1* device, const LoadedObj &obj)
{
    Mesh mesh;

    mesh.stride = sizeof(VertexData);
    mesh.numVertices = obj.numVertices;
    mesh.offset = 0;
    mesh.numIndices = obj.numIndices;

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = obj.numVertices * sizeof(VertexData);
    vertexBufferDesc.Usage     = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData = { obj.vertexBuffer };

    HRESULT hResult = device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &mesh.vertexBuffer);
    assert(SUCCEEDED(hResult));

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = obj.numIndices * sizeof(uint16_t);
    indexBufferDesc.Usage     = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexSubresourceData = { obj.indexBuffer };

    hResult = device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &mesh.indexBuffer);
    assert(SUCCEEDED(hResult));

    return mesh;
}

Texture d3d11CreateTexture(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext, const char* fileName)
{
    Texture texture;

    // Load Image
    int texForceNumChannels = 4;
    unsigned char* textureBytes = stbi_load(fileName, &texture.width, &texture.height, &texture.numChannels, texForceNumChannels);
    assert(textureBytes);
    texture.bytesPerRow = 4 * texture.width;

    // Create Texture
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width              = texture.width;
        textureDesc.Height             = texture.height;
        textureDesc.MipLevels          = 0;
        textureDesc.ArraySize          = 1;
        textureDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        textureDesc.SampleDesc.Count   = 1;
        textureDesc.Usage              = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        textureDesc.MiscFlags          = D3D11_RESOURCE_MISC_GENERATE_MIPS;

        ID3D11Texture2D* d3dTexture;
        device->CreateTexture2D(&textureDesc, NULL, &d3dTexture);

        deviceContext->UpdateSubresource(d3dTexture, 0, NULL, textureBytes, texture.bytesPerRow, 0);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = (UINT)-1;

        device->CreateShaderResourceView(d3dTexture, &srvDesc, &texture.d3dShaderResourceView);
        d3dTexture->Release();

        deviceContext->GenerateMips(texture.d3dShaderResourceView);
    }

    free(textureBytes);
    return texture;
}

ID3D11Buffer* d3d11CreateConstantBuffer(ID3D11Device1* device, size_t bufferSize)
{
    ID3D11Buffer* constantBuffer;
    D3D11_BUFFER_DESC constantBufferDesc = {};
    // ByteWidth must be a multiple of 16, per the docs
    constantBufferDesc.ByteWidth = bufferSize + 0xf & 0xfffffff0;
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hResult = device->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);
    assert(SUCCEEDED(hResult));
    return constantBuffer;
}

void d3d11OverwriteConstantBuffer(ID3D11DeviceContext1* deviceContext, ID3D11Buffer* constantBuffer, void* data, size_t dataSize)
{
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    deviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    CopyMemory(mappedSubresource.pData, data, dataSize);
    deviceContext->Unmap(constantBuffer, 0);
}
