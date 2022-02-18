#ifndef ITI_LIB_HTTP_H
#define ITI_LIB_HTTP_H

#include <fmt/format.h>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "StatusCode.h"
#include "context.h"
#include "uri.h"

namespace iti {
namespace http {

class Method {
  public:
	static Method parse(const std::string &rawMethod);
	static bool try_parse(const std::string &rawMethod, Method &method);

	enum Value {
		unknown = 1 << 0,
		CONNECT = 1 << 1,
		DEL     = 1 << 2, // should be DELETE but wnnt.h #defines DELETE...
		GET     = 1 << 3,
		HEAD    = 1 << 4,
		OPTIONS = 1 << 5,
		PATCH   = 1 << 6,
		POST    = 1 << 7,
		PUT     = 1 << 8,
		TRACE   = 1 << 9,
	};

#pragma warning(suppress : 26812)
	static const Value ALL =
	    (Value)(http::Method::CONNECT | http::Method::DEL | http::Method::GET |
	            http::Method::HEAD | http::Method::OPTIONS |
	            http::Method::PATCH | http::Method::POST | http::Method::PUT |
	            http::Method::TRACE);

	Method() = default;
#pragma warning(suppress : 26812)
	constexpr Method(Value method) : value(method) {}

	constexpr bool operator==(const Method &a) const {
		return value == a.value;
	}
	constexpr bool operator!=(const Method &a) const {
		return value != a.value;
	}
	constexpr bool operator<(const Method &a) const { return value < a.value; }

	constexpr Value operator()() const { return value; }

	constexpr bool is_valid() const {
		// clang-format off
		return value == http::Method::CONNECT ||
			   value == http::Method::DEL ||
			   value == http::Method::GET ||
			   value == http::Method::HEAD ||
			   value == http::Method::OPTIONS ||
			   value == http::Method::PATCH ||
			   value == http::Method::POST ||
			   value == http::Method::PUT ||
			   value == http::Method::TRACE;
		// clang-format on
	}

	constexpr std::string_view str() const {
		switch (value) {
		case Value::CONNECT:
			return "CONNECT";
		case Value::DEL:
			return "DELETE";
		case Value::GET:
			return "GET";
		case Value::HEAD:
			return "HEAD";
		case Value::OPTIONS:
			return "OPTIONS";
		case Value::PATCH:
			return "PATCH";
		case Value::POST:
			return "POST";
		case Value::PUT:
			return "PUT";
		case Value::TRACE:
			return "TRACE";
		case Value::unknown:
			return "";
		case ALL:
			return "";
		default:
			throw std::logic_error(
			    fmt::format("unhandled HTTP method {}", value));
		}
	}

  private:
	Value value = Method::unknown;
};
} // namespace http
} // namespace iti

namespace std {
template <> struct hash<iti::http::Method> {
	std::size_t operator()(const iti::http::Method &m) const noexcept {
		return static_cast<size_t>(m());
	}
};
} // namespace std

namespace iti {
namespace http {

// A Header represents the key-value pairs in an HTTP header.
// The keys should be in canonical form, as returned by canonical_key().
class Header {
  public:
	// `canonical_key()` returns the canonical format of the header key.
	// The canonicalization converts the first letter and any letter following a
	// hyphen to upper case; the rest are converted to lowercase.
	//
	// For example, the canonical key for "content-type" is "Content-Type".
	//
	// If the key contains an invalid header field character, it is returned
	// without modifications.
	static std::string gen_canonical_key(const std::string &key);

	// `add()` adds the key, value pair to the header. It appends to any
	// existing values associated with key.
	// The key is case insensitive; it is canonicalized by canonical_key().
	void add(const std::string &key, const std::string &value);

	// `del()`  deletes the values associated with key.
	// The key is case insensitive; it is canonicalized by canonical_key().
	void del(const std::string &key);

	// Set sets the header entries associated with key to the single element
	// value. It replaces any existing values associated with key.
	// The key is case insensitive; it is canonicalized by canonical_key().
	void set(const std::string &key, const std::string &value);

	// `get()` gets the first value associated with the given key. If there are
	// no values associated with the key, Get returns an empty string.
	// The key is case insensitive; it is canonicalized by canonical_key().
	std::string get(const std::string &key) const;

	// `get_all_values()` returns all values associated with the given key.
	// The key is case insensitive; it is canonicalized by canonical_key().
	std::vector<std::string> get_all_values(const std::string &key) const;

	auto cbegin() { return headerMap.cbegin(); }
	auto cend() { return headerMap.cend(); }

  private:
	std::unordered_map<std::string, std::vector<std::string>> headerMap;
};

class Request {
  public:
	// method specifies the HTTP method (GET, POST, PUT, etc.).
	Method method;

	// url is parsed from the URI supplied
	iti::http::Uri url;

	// header contains the request header fields either
	// received by the server or to be sent by the
	// client.
	//
	// If a server received a request with header lines,
	//
	//	Host: example.com
	//	accept-encoding: gzip, deflate
	//	Accept-Language: en-us
	//	fOO: Bar
	//	foo: two
	//
	// then
	//
	//	header["Accept-Encoding"] == {"gzip, deflate"}
	//	header["Accept-Language"] == {"en-us"}
	//	header["Foo"] == {"Bar", "two"}
	//
	//
	// HTTP defines that header names are case-insensitive.
	// The request parser implements this by using Header::gen_canonical_key(),
	// making the first character and any characters following a hyphen
	// uppercase and the rest lowercase.
	Header header;

	// body contains the HTTP Request Body's data (if a body way sent)
	//
	// TODO:
	// replace the full body with a `get_body()` method
	// new method should accept a buffer and bufferSize
	// new method should return the number of bytes written
	//
	// will also need to add contentLenth or something
	// so users can choose to allocate the full body or not.
	std::string body;

	// context is a temporary datastore that can be used
	// to move data through the request pipeline.
	mutable iti::Context context;
};

class Response {
  public:
	// HTTP status code
	// defaults to 200
	//
	int status = iti::http::StatusCode::Status200OK;

	// header maps header keys to values. If the response had multiple
	// headers with the same key, they may be concatenated, with comma
	// delimiters.  (RFC 7230, section 3.2.2 requires that multiple headers
	// be semantically equivalent to a comma-delimited sequence.)
	//
	// Keys in the map are canonicalized (see Header::gen_canonical_key()).
	Header header;

	// context is a temporary datastore that can be used to move data through
	// the request pipeline.
	// It is not directly used when sending the response to the client.
	iti::Context context;

	// write sends the response to the client with the supplied body content
	virtual void write(const std::string &body = "") = 0;
};

class IHandler {
  public:
	template <class T>
	static inline std::shared_ptr<IHandler> make_handler(T *derived) {
		static_assert(std::is_base_of<IHandler, T>::value);

		std::shared_ptr<IHandler> p;
		p.reset(derived);
		return p;
	}
	using handlerFunc = std::function<void(const Request &, Response &)>;

	virtual void handle_request(const Request &, Response &) = 0;
};

class BasicHandler : public IHandler {
  public:
	BasicHandler() = delete;
	BasicHandler(IHandler::handlerFunc &&handlerFn)
	    : handler(std::forward<IHandler::handlerFunc>(handlerFn)) {}

	void handle_request(const Request &req, Response &resp) {
		if (handler != nullptr) {
			handler(req, resp);
		}
	}

  private:
	const IHandler::handlerFunc handler = nullptr;
};

} // namespace http
} // namespace iti

#endif // ITI_LIB_HTTP_H
