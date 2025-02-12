#pragma once

#include <memory>
#include "DebugUIWindow.h"

namespace HRZR
{
	class NxDXGIImpl;
	class NxD3DImpl;
}

namespace HRZR::DebugUI
{
	void Initialize(NxDXGIImpl *DXGIImpl);
	void AddWindow(std::shared_ptr<Window> Handle);

	void RenderUI();
	void RenderUID3D(NxD3DImpl *D3DImpl, NxDXGIImpl *DXGIImpl);

	bool ShouldInterceptInput();
	void UpdatePlayerSpecific();
	void UpdateFreecam();
}
