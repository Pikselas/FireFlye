#pragma once
#include<d3d11.h>
#include<wrl.h>

#include"Image.h"

class Texture
{
private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			 TEXTURE;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TEXTURE_VIEW;
private:
	unsigned int width = 0;
	unsigned int height = 0;
public:
	Texture() = default;
	Texture(ID3D11Device* graphics_device , const Image& image) : width(image.GetWidth()) , height(image.GetHeight())
	{
		// create texture data
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = image.GetWidth();
		desc.Height = image.GetHeight();
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA subresource_data = {};

		auto data = image.Raw();
		subresource_data.pSysMem = data;
		subresource_data.SysMemPitch = sizeof(*data) * image.GetWidth();

		subresource_data.SysMemSlicePitch = 0;

		graphics_device->CreateTexture2D(&desc, &subresource_data, &TEXTURE);
		graphics_device->CreateShaderResourceView(TEXTURE.Get(), nullptr, &TEXTURE_VIEW);
	}
public:
	ID3D11ShaderResourceView* GetResourceView() const
	{
		return TEXTURE_VIEW.Get();
	}
	unsigned int GetWidth() const
	{
		return width;
	}
	unsigned int GetHeight() const
	{
		return height;
	}
};