#pragma once
#include <utility>
#include "RipeGrain/SceneManager.h"

extern "C" __declspec(dllexport) void* GetScene();
extern "C" __declspec(dllexport) void UnloadScene(void* scene);

class BaseScene : public RipeGrain::Scene
{
public:
	virtual ~BaseScene() = default;
public:
	template<typename T, typename... ParamsT>
	void LoadScene(ParamsT&& ... params)
	{
		GetSceneLoader()->LoadScene<T, ParamsT...>(std::forward<ParamsT>(params)..., UnloadScene);
	}
};