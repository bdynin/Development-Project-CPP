#ifndef ITI_LIB_HTTP_STATUSCODE_H
#define ITI_LIB_HTTP_STATUSCODE_H

#include <string_view>
#include <unordered_map>
#include <vector>

namespace iti {
namespace http {

class StatusCode {
  public:
	static StatusCode parse(int rawStatusCode);
	static bool try_parse(int rawStatusCode, StatusCode &statusCode);

	enum Value : int {
		Status100Continue          = 100,
		Status101SwitchingProtocol = 101,
		Status102Processing        = 102,
		Status103EarlyHints        = 103,

		Status200OK                   = 200,
		Status201Created              = 201,
		Status202Accepted             = 202,
		Status203NonAuthoritativeInfo = 203,
		Status204NoContent            = 204,
		Status205ResetContent         = 205,
		Status206ParitalContent       = 206,
		Status207MultiStatus          = 207,
		Status208AlreadyReported      = 208,
		Status226IMUsed               = 226,

		Status300MultipleChoice    = 300,
		Status301MovedPermanently  = 301,
		Status302Found             = 302,
		Status303SeeOther          = 303,
		Status304NotModified       = 304,
		Status305UseProxy          = 305,
		Status307TemporaryRedirect = 307,
		Status308PermanentRedirect = 308,

		Status400BadRequest                  = 400,
		Status401Unauthorized                = 401,
		Status402PaymentRequired             = 402,
		Status403Forbidden                   = 403,
		Status404NotFound                    = 404,
		Status405MethodNotAllowed            = 405,
		Status406NotAcceptable               = 406,
		Status407ProxyAuthRequired           = 407,
		Status408RequestTimeout              = 408,
		Status409Conflict                    = 409,
		Status410Gone                        = 410,
		Status411LengthRequired              = 411,
		Status412PreconditionFailed          = 412,
		Status413PayloadTooLarge             = 413,
		Status414UriTooLong                  = 414,
		Status415UnsupportedMediaType        = 415,
		Status416RangeNotSatisfiable         = 416,
		Status417ExpectationFailed           = 417,
		Status418Teapot                      = 418,
		Status421MisdirectedRequest          = 421,
		Status422UnprocessableEntity         = 422,
		Status423Locked                      = 423,
		Status424FailedDependency            = 424,
		Status425TooEarly                    = 425,
		Status426UpgradeRequired             = 426,
		Status428PreconditionRequired        = 428,
		Status429TooManyRequests             = 429,
		Status431RequestHeaderFieldsTooLarge = 431,
		Status451UnavailableForLegalReasons  = 451,

		Status500InternalServerError     = 500,
		Status501NotImplemented          = 501,
		Status502BadGateway              = 502,
		Status503ServiceUnavailable      = 503,
		Status504GatewayTimeout          = 504,
		Status505HttpVersionNotSupported = 505,
		Status506VariantAlsoNegotiates   = 506,
		Status507InsufficientStorage     = 507,
		Status508LoopDetected            = 508,
		Status510NotExtended             = 510,
		Status511NetworkAuthRequired     = 511,
	};

	StatusCode() = default;
#pragma warning(suppress : 26812)
	constexpr StatusCode(Value statusCode) : value(statusCode) {}

	constexpr bool operator==(const StatusCode &a) const {
		return value == a.value;
	}
	constexpr bool operator!=(const StatusCode &a) const {
		return value != a.value;
	}
	constexpr Value operator()() const { return value; }
	std::string_view str() const;

  private:
	Value value;
};

} // namespace http
} // namespace iti

#endif // ITI_LIB_HTTP_STATUSCODE_H
