#pragma once

struct IDXGIFactory1;
struct IDXGISwapChain3;

namespace HRZR
{
	class NxDXGIImpl
	{
	public:
		char _pad0[0x18];				  // 0x0
		void *m_DXGIModuleHandle;		  // 0x18
		IDXGIFactory1 *m_DXGIFactory;	  // 0x20
		char _pad1[0x10];				  // 0x28
		IDXGISwapChain3 *m_DXGISwapChain; // 0x38
		uint32_t m_NumBuffers;			  // 0x40
	};
	assert_offset(NxDXGIImpl, m_DXGIFactory, 0x20);
	assert_offset(NxDXGIImpl, m_DXGISwapChain, 0x38);
}
