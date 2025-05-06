#pragma once
#include "ImageSprite.h"
#include "StencilBuffer.h"

class ResourceEngine
{
	friend class ResourceEngineAccessor;
protected:
	Microsoft::WRL::ComPtr<ID3D11Device> graphics_device;
public:
	virtual StencilBuffer CreateStencilBuffer(unsigned int width, unsigned int height) = 0;
	virtual Texture CreateTexture(const Image& image) = 0;
public:
	virtual ImageSprite CreateSprite(const Image& image) = 0;
	virtual ImageSprite CreateSprite(Texture texture, unsigned int width, unsigned int height) = 0;
};