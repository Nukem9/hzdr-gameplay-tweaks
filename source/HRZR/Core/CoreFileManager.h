#pragma once

#include "../PCore/Common.h"

namespace HRZR
{
	class RTTIRefObject;

	class CoreFileManager
	{
	public:
		class Events
		{
		public:
			virtual void OnStartLoading(const String& CorePath) = 0;										  // 0
			virtual void OnFinishLoading(const String& CorePath) = 0;										  // 1
			virtual void OnStartUnloading(const String& CorePath) = 0;										  // 2
			virtual void OnFinishUnloading() = 0;															  // 3
			virtual void OnReadFile(const String& CorePath, const Array<Ref<RTTIRefObject>>& Objects) = 0;	  // 4
			virtual void OnLoadAsset(const String& CorePath, const Array<Ref<RTTIRefObject>>& Objects) = 0;	  // 5
			virtual void OnUnloadAsset(const String& CorePath, const Array<Ref<RTTIRefObject>>& Objects) = 0; // 6
		};

		virtual ~CoreFileManager(); // 0

		void RegisterEventHandler(Events *EventHandler)
		{
			const auto func = Offsets::Signature("E8 ? ? ? ? E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? B9 18 00 00 00")
								  .AsRipRelative(5)
								  .ToPointer<void(void *, Events *)>();

			func(this, EventHandler);
		}
	};
}
