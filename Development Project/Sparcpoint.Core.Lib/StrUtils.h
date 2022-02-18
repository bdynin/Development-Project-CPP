#ifndef ITI_LIB_STRUTILS_H
#define ITI_LIB_STRUTILS_H

#include <algorithm>
#include <chrono>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "fmt/format.h"

namespace iti {
namespace strutils {

static const constexpr std::string_view empty_sv{""};

#define STRUTILS_IS_STRINGLIKE_3(TYP, TYP2, TYP3)                              \
	(STRUTILS_IS_STRINGLIKE_2(TYP, TYP2) && STRUTILS_IS_STRINGLIKE(TYP3))

#define STRUTILS_IS_STRINGLIKE_2(TYP, TYP2)                                    \
	(STRUTILS_IS_STRINGLIKE(TYP) && STRUTILS_IS_STRINGLIKE(TYP2))

#define STRUTILS_IS_STRINGLIKE(TYP)                                            \
	(std::is_same_v<TYP, std::string> ||                                       \
	 std::is_same_v<TYP, std::string_view> ||                                  \
	 std::is_convertible_v<TYP, std::string> ||                                \
	 std::is_convertible_v<TYP, std::string_view>)

#define STRUTILS_IS_WSTRINGLIKE_3(TYP, TYP2, TYP3)                             \
	(STRUTILS_IS_WSTRINGLIKE_2(TYP, TYP2) && STRUTILS_IS_WSTRINGLIKE(TYP3))

#define STRUTILS_IS_WSTRINGLIKE_2(TYP, TYP2)                                   \
	(STRUTILS_IS_WSTRINGLIKE(TYP) && STRUTILS_IS_WSTRINGLIKE(TYP2))

#define STRUTILS_IS_WSTRINGLIKE(TPY)                                           \
	(std::is_same_v<TPY, std::wstring> ||                                      \
	 std::is_same_v<TPY, std::wstring_view> ||                                 \
	 std::is_convertible_v<TPY, std::wstring> ||                               \
	 std::is_convertible_v<TPY, std::wstring_view>)

static inline bool iequals(const std::string &a, const std::string &b) {
	return std::equal(a.begin(), a.end(), b.begin(), b.end(),
	                  [](char a, char b) { return tolower(a) == tolower(b); });
}

static inline bool iequals(const std::wstring &a, const std::wstring &b) {
	return std::equal(
	    a.begin(), a.end(), b.begin(), b.end(),
	    [](wchar_t a, wchar_t b) { return towlower(a) == towlower(b); });
}

// replace (copy). replace 'n' instances of 'from' in 's' with 'to'
// n == -1 is the same as "all"
template <typename T, typename U, typename V>
static inline T replace_copy_impl(const T &s, const U &from, const V &to,
                                  int numToReplace = -1) {
	static_assert(STRUTILS_IS_STRINGLIKE_3(T, U, V) ||
	                  STRUTILS_IS_WSTRINGLIKE_3(T, U, V),
	              "strutils::reaplce_copy only accepts: std::string, "
	              "std::string_view, std::wstring, or std::wstring_view");

	// if the number of occurrences to replace is 0 then avoid further work and
	// the 'ns' allocation
	if (numToReplace == 0) {
		return s;
	}

	// if 's' is empty then avoid further work and the 'ns' allocation
	if (s.empty()) {
		return s;
	}

	// if the search string ('from') is empty then avoid further work and the
	// 'ns' allocation
	if (from.empty()) {
		return s;
	}

	// if 'from' and 'to' are the same then avoid further work and the 'ns'
	// allocation
	if (from == to) {
		return s;
	}

	// make sure we have something to replace in the string
	const size_t fromLen = from.size();
	size_t pos{};
	std::vector<size_t> fromCounter;
	while ((pos = s.find(from, pos)) != T::npos) {
		fromCounter.push_back(pos);
		pos += fromLen;
	}

	// if 'from' isn't in 's' then avoid further work and the 'ns' allocation
	size_t n = 0;
	if (fromCounter.size() == 0) {
		return s;
	} else if (numToReplace < 0 ||
	           fromCounter.size() < static_cast<size_t>(numToReplace)) {
		n = fromCounter.size();
	} else {
		n = static_cast<size_t>(numToReplace);
	}

	// "replace" by building a new string
	// this is MUCH faster than replacing in place.
	//
	// Benchmark is replace all instances of "back" with "foobar" in
	// The Project Gutenberg EBook of The Adventures of Sherlock Holmes
	// by Sir Arthur Conan Doyle
	//
	// Benchmark                     Time           CPU          Iterations
	// BM_ReplaceInPlace      90694709 ns   90337000 ns          6
	// BM_ReplaceByBuilding    3697276 ns    3676935 ns        186
	//
	// We are basically trading increased memory usage for speed.
	//
	// create the output string 'ns'. Reserve the amount of space we need.
	T ns;
	ns.reserve(s.length() + n * (to.length() - fromLen));

	// reset pos and create a counter to make sure we only replace 'n'
	pos = 0;
	size_t counter{};
	for (auto const &p : fromCounter) {
		// if we hit the "n" count then stop
		if (counter == n) {
			break;
		}

		// append from last position to where the position of 'from' is
		ns.append(s, pos, p - pos);

		// append the replacement ('to')
		ns.append(to);

		// update position
		pos = p + fromLen;

		// update counter
		++counter;
	}

	// append remaining 's'
	ns.append(s, pos);

	return ns;
}

// replace (copy). replace 'n' instances of 'from' in 's' with 'to'
// n == -1 is the same as "all"
static std::string replace_copy(const std::string &s, const std::string &from,
                                const std::string &to, int numToReplace = -1) {
	return replace_copy_impl(s, from, to, numToReplace);
}

// replace (copy). replace 'n' instances of 'from' in 's' with 'to'
// n == -1 is the same as "all"
static std::wstring replace_copy(const std::wstring &s,
                                 const std::wstring &from,
                                 const std::wstring &to,
                                 int numToReplace = -1) {
	return replace_copy_impl(s, from, to, numToReplace);
}

// replace 'n' instances of 'from' in 's' with 'to'
// n == -1 is the same as "all"
static void replace(std::string &s, const std::string &from,
                    const std::string &to, int numToReplace = -1) {
	s = replace_copy_impl(s, from, to, numToReplace);
}

// replace 'n' instances of 'from' in 's' with 'to'
// n == -1 is the same as "all"
static void replace(std::wstring &s, const std::wstring &from,
                    const std::wstring &to, int numToReplace = -1) {
	s = replace_copy_impl(s, from, to, numToReplace);
}

// Split a string into an array of strings based on delimiter
template <typename T, typename U>
static inline std::vector<T> split_impl(const T &s, const U &delim,
                                        bool includeEmpty = true) {
	static_assert(STRUTILS_IS_STRINGLIKE_2(T, U) ||
	                  STRUTILS_IS_WSTRINGLIKE_2(T, U),
	              "strutils::split only accepts: std::string, "
	              "std::string_view, std::wstring, or std::wstring_view");

	// if 's' is empty then avoid further work
	if (s.empty()) {
		return std::vector<T>();
	}

	// if the delimiter string ('delim') is empty then avoid further work
	if (delim.empty()) {
		return std::vector<T>{s};
	}

	// if 'from' and 'to' are the same then avoid further work and the 'ns'
	// allocation
	if (s == delim) {
		return std::vector<T>{s};
	}

	// make sure we have something to find in the string
	const auto delimLen = delim.length();
	size_t pos{};
	std::vector<size_t> delimCounter;
	while ((pos = s.find(delim, pos)) != T::npos) {
		delimCounter.push_back(pos);
		pos += delimLen;
	}

	// if 'from' isn't in 's' then avoid further work and the 'ns' allocation
	if (delimCounter.size() == 0) {
		return std::vector<T>{s};
	}

	const auto n = delimCounter.size();

	// reserve the number of slots we need in the vector
	std::vector<T> rtnVec;
	rtnVec.reserve(n + 1);

	// reset pos and create a counter to make sure we only replace 'n'
	pos = 0;
	size_t subStrLen{};
	for (auto const &p : delimCounter) {
		subStrLen = p - pos;
		if (includeEmpty || subStrLen > 0) {
			rtnVec.emplace_back(s, pos, subStrLen);
		}

		// update position
		pos = p + delimLen;
	}

	// append remaining 's'
	subStrLen = s.size() - pos;
	if (includeEmpty || subStrLen > 0) {
		rtnVec.emplace_back(s, pos, subStrLen);
	}

	return rtnVec;
}

static std::vector<std::string> split(const std::string &s,
                                      const std::string &delim,
                                      bool includeEmpty = true) {
	return split_impl(s, delim, includeEmpty);
}

static std::vector<std::wstring> split(const std::wstring &s,
                                       const std::wstring &delim,
                                       bool includeEmpty = true) {
	return split_impl(s, delim, includeEmpty);
}

// check if a string starts with the supplied string
template <typename T, typename U>
static inline bool starts_with(const T &str, const U &search) {
	static_assert(
	    STRUTILS_IS_STRINGLIKE_2(T, U) || STRUTILS_IS_WSTRINGLIKE_2(T, U),
	    "strutils::starts_with only accepts: std::string or std::wstring");

	return str.rfind(search, 0) == 0;
}

static inline std::string humanize_duration(std::chrono::nanoseconds duration) {
	long long durationInNanaseconds = duration.count();

	if (durationInNanaseconds >= 1'000'000'000) { // second threshold
		return fmt::format(FMT_STRING("{:.2f}s"),
		                   (durationInNanaseconds / 1'000'000'000.0l));
	} else if (durationInNanaseconds >= 1'000'000) { // millisecond threshold
		return fmt::format(FMT_STRING("{:.2f}ms"),
		                   (durationInNanaseconds / 1'000'000.0l));
	} else if (durationInNanaseconds >= 1000) { // microseconds threshold
		return fmt::format(FMT_STRING("{:.2f}us"),
		                   (durationInNanaseconds / 1000.0l));
	}

	// nanoseconds
	return fmt::format(FMT_STRING("{:d}ns"), durationInNanaseconds);
}

} // namespace strutils
} // namespace iti

#endif // ITI_LIB_STRUTILS_H
