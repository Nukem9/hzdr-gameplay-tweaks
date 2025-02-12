#pragma once

#include <spdlog/fmt/ostr.h>

namespace HRZR
{
	class String final
	{
	public:
		using CharT = char;
		using size_type = size_t;

	private:
		struct StringRefData
		{
			constexpr static uint32_t InvalidCRC = 0xFFFFFFFF;

			uint32_t m_RefCount; // 0x0  (- 0x10)
			uint32_t m_CRC;		 // 0x4  (- 0xC)
			uint32_t m_Length;	 // 0x8  (- 0x8)
			uint32_t _padC;
			// const char m_Text[];	// 0x10 (- 0x0)
		};
		static_assert(sizeof(StringRefData) == 0x10);

		const char *m_Data = nullptr;

	public:
		String()
		{
			InitInternal(nullptr, 0);
		}

		String(const char *Value)
		{
			InitInternal(Value, strlen(Value));
		}

		String(const String& Other)
		{
			InitInternal(Other.data(), Other.size());
		}

		String(const std::string& Other)
		{
			InitInternal(Other.data(), Other.size());
		}

		~String()
		{
			const auto func = Offsets::Signature("48 8B 0A 48 85 C9 74 26 48 83 C1 F0 48 8D 05")
								  .ToPointer<void(void *, String *)>();

			func(nullptr, this);
		}

		String& operator=(const char *Other)
		{
			*this = String(Other);
			return *this;
		}

		String& operator=(const String& Other)
		{
			const auto func = Offsets::Signature("48 89 5C 24 10 57 48 83 EC 20 48 8B D9 48 8B FA 48 8B 09 48 3B 0A")
								  .ToPointer<void(String *, const String *)>();

			func(this, &Other);
			return *this;
		}

		String& operator=(const std::string& Other)
		{
			*this = String(Other);
			return *this;
		}

		bool operator==(const String& Other) const
		{
			return (size() == Other.size()) && (memcmp(data(), Other.data(), size()) == 0);
		}

		bool operator==(const char *Other) const
		{
			return (size() == strlen(Other)) && (memcmp(data(), Other, size()) == 0);
		}

		auto operator<=>(const String& Other) const
		{
			const size_t s1 = size();
			const size_t s2 = Other.size();
			const int cmp = memcmp(data(), Other.data(), std::min(s1, s2));

			if (cmp < 0)
				return std::strong_ordering::less;
			else if (cmp > 0)
				return std::strong_ordering::greater;

			// cmp might be 0 but lengths don't always match
			return s1 <=> s2;
		}

		const bool starts_with(CharT Char) const
		{
			return !empty() && m_Data[0] == Char;
		}

		const char *c_str() const
		{
			return m_Data;
		}

		const char *data() const
		{
			return m_Data;
		}

		size_t size() const
		{
			return InternalData()->m_Length;
		}

		size_t length() const
		{
			return size();
		}

		bool empty() const
		{
			return size() == 0;
		}

	private:
		void InitInternal(const char *Data, size_t Size)
		{
			const auto func = Offsets::Signature("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 83 EC 20 48 63 6C 24 70")
								  .ToPointer<void(String *, const char *, size_t, const char *, size_t)>();

			func(this, Data, Size, nullptr, 0); // More like String::Combine() than a constructor
		}

		StringRefData *InternalData() const
		{
			return reinterpret_cast<StringRefData *>(reinterpret_cast<ptrdiff_t>(m_Data) - sizeof(StringRefData));
		}
	};
}

template<>
struct std::formatter<HRZR::String> : std::formatter<const char *>
{
	std::format_context::iterator format(const HRZR::String& Value, std::format_context& Context) const
	{
		return std::copy_n(Value.data(), Value.size(), Context.out());
	}
};

template<>
struct fmt::formatter<HRZR::String> : fmt::formatter<const char *>
{
	fmt::format_context::iterator format(const HRZR::String& Value, fmt::format_context& Context) const
	{
		return std::copy_n(Value.data(), Value.size(), Context.out());
	}
};
