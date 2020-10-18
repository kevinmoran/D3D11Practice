#pragma once

#include <d3d11_1.h>

struct D3D11Data {
    ID3D11Device1* device;
    ID3D11DeviceContext1* deviceContext;
    IDXGISwapChain1* swapChain;
    ID3D11Texture2D* mainRenderTarget;
    ID3D11RenderTargetView* mainRenderTargetView;
    ID3D11Texture2D* msaaRenderTarget;
    ID3D11RenderTargetView* msaaRenderTargetView;
    ID3D11DepthStencilView* depthStencilView;
};

int d3d11Init(HWND hWindow, D3D11Data* d3d11);

bool d3d11InitRenderTargetsAndDepthBuffer(D3D11Data* d3d11);

bool d3d11CreateVertexShaderAndInputLayout(ID3D11Device1* device, LPCWSTR fileName, LPCSTR shaderEntryPoint, ID3D11VertexShader** vertexShader, D3D11_INPUT_ELEMENT_DESC inputElementDesc[], int numInputElements, ID3D11InputLayout** inputLayout);

bool d3d11CreatePixelShader(ID3D11Device1* device, LPCWSTR fileName, LPCSTR shaderEntryPoint, ID3D11PixelShader** pixelShader);

struct Mesh 
{
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    UINT numVertices;
    UINT numIndices;
    UINT stride;
    UINT offset;
};

struct LoadedObj;
bool d3d11CreateMesh(ID3D11Device1* device, const LoadedObj &obj, Mesh* mesh);

struct Texture
{
    ID3D11ShaderResourceView* d3dShaderResourceView;
    int width, height;
    int numChannels;
    int bytesPerRow;
};

bool d3d11CreateTexture(ID3D11Device1* device, ID3D11DeviceContext1* deviceContext, const char* fileName, Texture* texture);

bool d3d11CreateConstantBuffer(ID3D11Device1* device, size_t bufferSize, ID3D11Buffer** constantBuffer);

void d3d11OverwriteConstantBuffer(ID3D11DeviceContext1* deviceContext, ID3D11Buffer* constantBuffer, void* data, size_t dataSize);
