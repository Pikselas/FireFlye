#pragma once
#include <d3d11.h>
#include <wrl.h>

class StencilBuffer
{
private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> view;
public:
	StencilBuffer() = default;
	StencilBuffer(ID3D11Device* device, unsigned int width, unsigned int height)
	{
		D3D11_TEXTURE2D_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.Width = width;
		depthStencilDesc.Height = height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		device->CreateTexture2D(&depthStencilDesc, NULL, &buffer);

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		device->CreateDepthStencilView(buffer.Get(), &depthStencilViewDesc, &view);
	}
public:
	ID3D11DepthStencilView* getView()
	{
		return view.Get();
	}
};