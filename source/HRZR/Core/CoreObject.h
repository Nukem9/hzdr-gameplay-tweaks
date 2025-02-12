#pragma once

#include "RTTIRefObject.h"

namespace HRZR
{
	class CoreObject : public RTTIRefObject
	{
	public:
		virtual const RTTI *GetRTTI() const override; // 0
		virtual ~CoreObject() override;				  // 1
		virtual void CoreObjectUnknown04();			  // 4
		virtual String& GetName() const;			  // 5
		virtual bool CoreObjectUnknown06();			  // 6
		virtual void CoreObjectUnknown07();			  // 7
		virtual void CoreObjectUnknown08();			  // 8
		virtual void CoreObjectUnknown09();			  // 9
		virtual void CoreObjectUnknown10();			  // 10
		virtual void CoreObjectUnknown11();			  // 11
		virtual void CoreObjectUnknown12();			  // 12
		virtual void CoreObjectUnknown13();			  // 13
	};
	static_assert(sizeof(CoreObject) == 0x20);
}
