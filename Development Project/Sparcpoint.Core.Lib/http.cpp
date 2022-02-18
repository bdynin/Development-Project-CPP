#ifndef ITI_LIB_HTTP_CPP
#define ITI_LIB_HTTP_CPP

#include "pch.h"

#include "http.h"

#include <array>
#include <stdexcept>

// Helpers
// ----------------------------------------------------------------------------

static const constexpr std::array<bool, 128> isTokenTable = {
    //         Dec 	Ch	Description
    false, // 0 	NUL	Null
    false, // 1 	SOH	Start of Header
    false, // 2 	STX	Start of Text
    false, // 3 	ETX	End of Text
    false, // 4 	EOT	End of Transmission
    false, // 5 	ENQ	Enquiry
    false, // 6 	ACK	Acknowledge
    false, // 7 	BEL	Bell
    false, // 8 	BS	Backspace
    false, // 9 	HT	Horizontal Tab
    false, // 10 	LF	Line Feed
    false, // 11 	VT	Vertical Tab
    false, // 12 	FF	Form Feed
    false, // 13 	CR	Carriage Return
    false, // 14 	SO	Shift Out
    false, // 15 	SI	Shift In
    false, // 16 	DLE	Data Link Escape
    false, // 17 	DC1	Device Control 1
    false, // 18 	DC2	Device Control 2
    false, // 19 	DC3	Device Control 3
    false, // 20 	DC4	Device Control 4
    false, // 21 	NAK	Negative Acknowledge
    false, // 22 	SYN	Synchronize
    false, // 23 	ETB	End of Transmission Block
    false, // 24 	CAN	Cancel
    false, // 25 	EM	End of Medium
    false, // 26 	SUB	Substitute
    false, // 27 	ESC	Escape
    false, // 28 	FS	File Separator
    false, // 29 	GS	Group Separator
    false, // 30 	RS	Record Separator
    false, // 31 	US	Unit Separator
    false, // 32 	 	Space
    true,  // 33 	!	exclamation mark
    false, // 34 	"	double quote
    true,  // 35 	#	number
    true,  // 36 	$	dollar
    true,  // 37 	%	percent
    true,  // 38 	&	ampersand
    true,  // 39 	'	single quote
    false, // 40 	(	left parenthesis
    false, // 41 	)	right parenthesis
    true,  // 42 	*	asterisk
    true,  // 43 	+	plus
    false, // 44 	,	comma
    true,  // 45 	-	minus
    true,  // 46 	.	period
    false, // 47 	/	slash
    true,  // 48 	0	zero
    true,  // 49 	1	one
    true,  // 50 	2	two
    true,  // 51 	3	three
    true,  // 52 	4	four
    true,  // 53 	5	five
    true,  // 54 	6	six
    true,  // 55 	7	seven
    true,  // 56 	8	eight
    true,  // 57 	9	nine
    false, // 58 	:	colon
    false, // 59 	;	semicolon
    false, // 60 	<	less than
    false, // 61 	=	equality sign
    false, // 62 	>	greater than
    false, // 63 	?	question mark
    false, // 64 	@	at sign
    true,  // 65 	A
    true,  // 66 	B
    true,  // 67 	C
    true,  // 68 	D
    true,  // 69 	E
    true,  // 70 	F
    true,  // 71 	G
    true,  // 72 	H
    true,  // 73 	I
    true,  // 74 	J
    true,  // 75 	K
    true,  // 76 	L
    true,  // 77 	M
    true,  // 78 	N
    true,  // 79 	O
    true,  // 80 	P
    true,  // 81 	Q
    true,  // 82 	R
    true,  // 83 	S
    true,  // 84 	T
    true,  // 85 	U
    true,  // 86 	V
    true,  // 87 	W
    true,  // 88 	X
    true,  // 89 	Y
    true,  // 90 	Z
    false, // 91 	[	left square bracket
    false, // 92 	\	backslash
    false, // 93 	]	right square bracket
    true,  // 94 	^	caret / circumflex
    true,  // 95 	_	underscore
    true,  // 96 	`	grave / accent
    true,  // 97 	a
    true,  // 98 	b
    true,  // 99 	c
    true,  // 100 	d
    true,  // 101 	e
    true,  // 102 	f
    true,  // 103 	g
    true,  // 104 	h
    true,  // 105 	i
    true,  // 106 	j
    true,  // 107 	k
    true,  // 108 	l
    true,  // 109 	m
    true,  // 110 	n
    true,  // 111 	o
    true,  // 112 	p
    true,  // 113 	q
    true,  // 114 	r
    true,  // 115 	s
    true,  // 116 	t
    true,  // 117 	u
    true,  // 118 	v
    true,  // 119 	w
    true,  // 120 	x
    true,  // 121 	y
    true,  // 122 	z
    false, // 123 	{	left curly bracket
    true,  // 124 	|	vertical bar
    false, // 125 	}	right curly bracket
    true,  // 126 	~	tilde
    false, // 127 	DEL	delete
};

// `valid_header_field_char()` reports whether c valid in a header field name.
//  RFC 7230 says:
//   header-field   = field-name ":" OWS field-value OWS
//   field-name     = token
//   tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
//           "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
//   token = 1*tchar
static inline bool valid_header_field_char(unsigned char c) {
	return (int(c) < isTokenTable.size()) && isTokenTable[int(c)];
}

// method
// ----------------------------------------------------------------------------
iti::http::Method iti::http::Method::parse(const std::string &rawMethod) {
	auto m = rawMethod;
	std::transform(m.begin(), m.end(), m.begin(), ::toupper);

	if (m == "CONNECT")
		return Value::CONNECT;
	if (m == "DELETE")
		return Value::DEL;
	if (m == "GET")
		return Value::GET;
	if (m == "HEAD")
		return Value::HEAD;
	if (m == "OPTIONS")
		return Value::OPTIONS;
	if (m == "PATCH")
		return Value::PATCH;
	if (m == "POST")
		return Value::POST;
	if (m == "PUT")
		return Value::PUT;
	if (m == "TRACE")
		return Value::TRACE;

	throw std::logic_error(fmt::format("unknown HTTP method {}", rawMethod));
}

bool iti::http::Method::try_parse(const std::string &rawMethod,
                                  iti::http::Method &method) {
	auto m = rawMethod;
	std::transform(m.begin(), m.end(), m.begin(), ::toupper);

	// set method to "unknown"
	// will be used later
	method = Method::unknown;

	if (m == "CONNECT")
		method = Value::CONNECT;
	else if (m == "DELETE")
		method = Value::DEL;
	else if (m == "GET")
		method = Value::GET;
	else if (m == "HEAD")
		method = Value::HEAD;
	else if (m == "OPTIONS")
		method = Value::OPTIONS;
	else if (m == "PATCH")
		method = Value::PATCH;
	else if (m == "POST")
		method = Value::POST;
	else if (m == "PUT")
		method = Value::PUT;
	else if (m == "TRACE")
		method = Value::TRACE;

	return method != Method::unknown;
}

// Header
// ----------------------------------------------------------------------------
std::string iti::http::Header::gen_canonical_key(const std::string &key) {
	// check if it looks like a header key.
	// if not, return original key.
	for (const auto &c : key) {
		if (valid_header_field_char(c)) {
			continue;
		}

		// Don't canonicalize.
		return key;
	}

	bool upper       = true;
	std::string cKey = key;
	const auto len   = cKey.size();

	for (size_t i = 0; i < len; i++) {
		char c = cKey[i];
		// Canonicalize: first letter upper case
		// and upper case after each dash.
		// (Host, User-Agent, If-Modified-Since).
		// MIME headers are ASCII only, so no Unicode issues.
		if (upper && 'a' <= c && c <= 'z') {
			c = std::toupper(c);
		} else if (!upper && 'A' <= c && c <= 'Z') {
			c = std::tolower(c);
		}
		cKey[i] = c;
		upper   = c == '-'; // need to uppercase if we have a separator
	}

	return cKey;
}

void iti::http::Header::add(const std::string &key, const std::string &value) {
	headerMap[gen_canonical_key(key)].push_back(value);
}

void iti::http::Header::del(const std::string &key) {
	headerMap.erase(gen_canonical_key(key));
}

void iti::http::Header::set(const std::string &key, const std::string &value) {
	headerMap[gen_canonical_key(key)] = std::vector<std::string>{value};
}

std::string iti::http::Header::get(const std::string &key) const {
	try {
		auto &v = headerMap.at(gen_canonical_key(key));
		if (!v.empty()) {
			return std::string(v[0]);
		}
	} catch (std::out_of_range) {
		return std::string();
	}
	return std::string();
}

std::vector<std::string>
iti::http::Header::get_all_values(const std::string &key) const {
	try {
		auto &v = headerMap.at(gen_canonical_key(key));
		return std::vector<std::string>(v);
	} catch (std::out_of_range) {
		return std::vector<std::string>();
	}
	return std::vector<std::string>();
}

#endif // ITI_LIB_HTTP_CPP
