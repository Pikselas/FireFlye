#pragma once
#include "ResourceEngine.h"

class ResourceEngineAccessor
{
protected:
	ID3D11Device* GetDevice(const ResourceEngine& engine)
	{
		return engine.graphics_device.Get();
	}
};