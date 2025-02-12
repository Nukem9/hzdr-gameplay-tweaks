#pragma once

#include "../PCore/Common.h"

namespace HRZR
{
	class JobHeaderCPU
	{
	private:
		virtual void UnknownJobHeaderCPU00(); // 0
		virtual ~JobHeaderCPU();			  // 1
		virtual void Destruct();			  // 2

		char _pad0[0x30]; // 0x00
		short m_RefCount; // 0x38

	public:
		void AddRef()
		{
			_InterlockedIncrement16(&m_RefCount);
		}

		void Release()
		{
			if (_InterlockedDecrement16(&m_RefCount) == 0)
				Destruct();
		}

		static Ref<JobHeaderCPU> CreateCallableJob(void (*Callback)(void *UserData), void *UserData)
		{
			const auto func = Offsets::Signature("E8 ? ? ? ? 48 8B 86 A8 00 00 00 B9 F6 FF 00 00 66 21 48 3A")
								  .AsRipRelative(5)
								  .ToPointer<Ref<JobHeaderCPU>(void *, void *, void *, void (*)(void *))>();

			return func(nullptr, nullptr, UserData, Callback);
		}

		static Ref<JobHeaderCPU> CreateCallableJob(std::function<void()> Callback)
		{
			auto userData = new std::function<void()>(std::move(Callback));
			auto wrapper = +[](void *Userdata)
			{
				auto callback = static_cast<decltype(userData)>(Userdata);
				(*callback)();

				delete callback;
			};

			return CreateCallableJob(wrapper, userData);
		}

		static void SubmitCallback(void (*Callback)())
		{
			auto job = CreateCallableJob(reinterpret_cast<void (*)(void *)>(Callback), nullptr);
			SubmitJob(job);
		}

		static void SubmitCallable(std::function<void()> Callback)
		{
			auto job = CreateCallableJob(std::move(Callback));
			SubmitJob(job);
		}

		static void SubmitJob(Ref<JobHeaderCPU> Job)
		{
			const auto mainJobQueue = Offsets::Signature("48 8D 0D ? ? ? ? E8 ? ? ? ? 66 F0 0F C1 6B 38")
										  .AsRipRelative(7)
										  .ToPointer<void>();

			const auto func = Offsets::Signature("48 89 6C 24 18 56 48 83 EC 20 48 8B F2 48 8B E9 0F B7 52 3A 83 E2 0F")
								  .ToPointer<bool(void *, JobHeaderCPU *)>();

			func(mainJobQueue, Job);
		}
	};
}
