#pragma once

namespace HRZR
{
	class NetMessageListener
	{
	public:
		virtual ~NetMessageListener();		  // 0
		virtual void ProcessNetMessage() = 0; // 1
	};
}
