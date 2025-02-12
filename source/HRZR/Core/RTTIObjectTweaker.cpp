#include <charconv>
#include "RTTI.h"
#include "RTTIObjectTweaker.h"

void HRZR::RTTIObjectTweaker::VisitObjectPath(void *Object, const RTTI *Type, const String& Path, SetValueVisitor *Visitor)
{
	auto splitStringByDelimiter = [](const std::string_view& Text, char Delimiter, auto&& Callback)
	{
		size_t last = 0;
		size_t next = 0;

		while ((next = Text.find(Delimiter, last)) != std::string_view::npos)
		{
			if (!Callback(Text.substr(last, next - last)))
				return false;

			last = next + 1;
		}

		return Callback(Text.substr(last));
	};

	// Split the RTTI path up. Periods act as delimiters for object member resolution purposes.
	const bool succeeded = splitStringByDelimiter(
		std::string_view(Path.c_str(), Path.length()),
		'.',
		[&](std::string_view PathPart)
	{
		if (PathPart.empty())
		{
			Visitor->m_LastError = "Invalid or empty RTTI path";
			return false;
		}

		if (!Object)
		{
			Visitor->m_LastError = "Attempted to dereference a null pointer";
			return false;
		}

		// Check for array indexing first and isolate brackets
		std::string_view arrayIndexPathPart;

		if (auto bracketPosition = PathPart.find_first_of("[]"); bracketPosition != std::string_view::npos)
		{
			arrayIndexPathPart = PathPart.substr(bracketPosition);
			PathPart = PathPart.substr(0, bracketPosition);
		}

		// Resolve the object's member field, optionally resolve the array element access, then dereference any pointers
		if (!ResolveCompoundMember(&Object, &Type, PathPart, Visitor))
			return false;

		if (!arrayIndexPathPart.empty())
		{
			if (!ResolveArrayAccess(&Object, &Type, arrayIndexPathPart, Visitor))
				return false;
		}

		if (Type->m_Type == ERTTIType::Pointer)
		{
			auto asContainer = static_cast<const RTTIContainer *>(Type);
			auto asData = static_cast<RTTIContainer::DataPointer *>(asContainer->m_ContainerType);

			Object = asData->m_Getter(Type, Object);
			Type = asContainer->m_ItemType;
		}

		return true;
	});

	if (succeeded)
		Visitor->Visit(Object, Type);
}

void HRZR::RTTIObjectTweaker::VisitObjectPath(
	void *Object,
	const RTTI *Type,
	const String& Path,
	SetValueVisitorFunctor::functor_type Functor)
{
	SetValueVisitorFunctor f(Functor);
	VisitObjectPath(Object, Type, Path, &f);
}

bool HRZR::RTTIObjectTweaker::ResolveCompoundMember(
	void **Object,
	const RTTI **Type,
	const std::string_view& MemberName,
	SetValueVisitor *Visitor)
{
	const bool memberFound = (*Type)->AsCompound()->VisitClassMembersByInheritance(
		*Object,
		[&](const RTTICompound::Attr& Member, void *MemberObject)
	{
		if (Member.m_Name != MemberName)
			return false;

		if (Member.IsProperty())
			throw std::runtime_error("Cannot obtain a reference to a property function");

		*Object = MemberObject;
		*Type = Member.m_Type;
		return true;
	});

	if (!memberFound)
		Visitor->m_LastError = "RTTI member not found";

	return memberFound;
}

bool HRZR::RTTIObjectTweaker::ResolveArrayAccess(void **Object, const RTTI **Type, const std::string_view& Member, SetValueVisitor *Visitor)
{
	// Ensure we're actually dealing with an array container object
	if ((*Type)->m_Type != ERTTIType::Container)
	{
		Visitor->m_LastError = "RTTI member type is not an array";
		return false;
	}

	auto containerData = static_cast<const RTTIContainer::DataContainer *>(static_cast<const RTTIContainer *>(*Type)->m_ContainerType);
	const auto containerElementCount = containerData->m_GetNumItems(*Type, *Object);

	// Extract and parse the integer index between brackets
	const auto arrayStart = Member.find_first_of('[');
	const auto arrayEnd = Member.find_first_of(']');

	if (arrayStart == std::string_view::npos || arrayEnd == std::string_view::npos || (arrayStart + 1) >= arrayEnd)
	{
		Visitor->m_LastError = "Mismatched array brackets or empty array index";
		return false;
	}

	size_t parsedIndex = 0;
	const auto [_, err] = std::from_chars(Member.data() + arrayStart + 1, Member.data() + arrayEnd, parsedIndex, 10);

	if (err != std::errc() || parsedIndex >= containerElementCount)
	{
		Visitor->m_LastError = "Invalid array index";
		return false;
	}

	// Finally grab the element's value
	*Object = containerData->m_GetItem(*Type, *Object, static_cast<int>(parsedIndex));
	*Type = (*Type)->GetContainedType();

	return true;
}
