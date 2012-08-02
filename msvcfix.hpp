#ifndef MSVCFIX_HPP
#define MSVCFIX_HPP
#ifdef _MSC_VER

	// switch statement contains 'default' but no 'case' labels
#pragma warning (disable: 4065)
	// conversion from __w64 int to int
#pragma warning (disable: 4244)
	// conversion from size_t to int
#pragma warning (disable: 4267)

#pragma warning (disable: 4018)

#define vsnprintf _vsnprintf

#undef DELETE

#endif //_MSC_VER
#endif //MSVCFIX_HPP
