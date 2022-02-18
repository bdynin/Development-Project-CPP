#ifndef ITI_LIB_HTTP_URI_H
#define ITI_LIB_HTTP_URI_H

#include <algorithm>
#include <charconv>
#include <string>

#include "StrUtils.h"

namespace iti {
namespace http {

class Uri {
  public:
	std::string queryString, path, protocol, host, rawPort;
	long port{};

	static Uri parse(const std::string &rawUri) {
		Uri result;

		if (rawUri.empty()) {
			return result;
		}

		auto uriEnd = rawUri.end();

		// get query start
		auto queryStart = std::find(rawUri.begin(), uriEnd, '?');

		// protocol
		auto protocolStart = rawUri.begin();
		auto protocolEnd   = std::find(protocolStart, uriEnd, ':');

		if (protocolEnd != uriEnd) {
			std::string prot = &*(protocolEnd);
			if ((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
				result.protocol = std::string(protocolStart, protocolEnd);
				protocolEnd += 3; // ://
			} else {
				protocolEnd = rawUri.begin(); // no protocol
			}
		} else {
			protocolEnd = rawUri.begin(); // no protocol
		}
		// host
		auto hostStart = protocolEnd;
		auto pathStart = std::find(hostStart, uriEnd, '/'); // get pathStart

		auto hostEnd = std::find(protocolEnd,
		                         (pathStart != uriEnd) ? pathStart : queryStart,
		                         ':'); // check for port

		result.host = std::string(hostStart, hostEnd);

		// port
		if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':')) {
			// we have a port
			hostEnd++;
			auto portEnd   = (pathStart != uriEnd) ? pathStart : queryStart;
			result.rawPort = std::string(hostEnd, portEnd);
			const auto res = std::from_chars(
			    result.rawPort.data(),
			    result.rawPort.data() + result.rawPort.size(), result.port);

			if (res.ec != std::errc()) {
				result.port = 0;
			}
		}

		if (result.port == 0) {
			if (strutils::iequals(result.protocol, "http")) {
				result.port = 80;
			} else if (strutils::iequals(result.protocol, "https")) {
				result.port = 443;
			}
		}

		// path
		if (pathStart != uriEnd) {
			result.path = std::string(pathStart, queryStart);
		}

		// query
		if (queryStart != uriEnd) {
			result.queryString = std::string(queryStart, rawUri.end());
		}

		return result;
	}
};

} // namespace http
} // namespace iti

#endif // ITI_LIB_HTTP_URI_H
