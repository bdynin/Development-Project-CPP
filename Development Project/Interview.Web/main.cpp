// main.cpp : This file contains the 'main' function.
// Program execution begins and ends there.
//

#include <algorithm>
#include <chrono>
#include <evhttp.h>
#include <future>
#include <list>
#include <memory>

// winsock2 for windows
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

#include "fmt/format.h"
#include "json.hpp"

#include "StatusCode.h"
#include "http.h"
#include "router.mux.h"
#include "uri.h"

#include "config.h"
#include "evHttpResponse.hpp"
#include "middlewares.hpp"

#include "IProductHandler.h"
#include "ProductUtil.h"

using iti::http::Request;
using iti::http::Response;
using iti::http::StatusCode;
using iti::http::router::IRouter;
using iti::http::router::Mux;
using iti::http::router::RoutingContext;
using nlohmann::json;

using namespace std::chrono_literals;

// we would use a real thread pool in production
std::list<std::future<evHttpResponse>> eventPool;

// we use libevent (non-blocking) as the webserver
// evHttpHandleRequest is generic handler we use for all the requests
// it grabs the request data and router and pushes all the actual work of
// processing the request to a new thread
static void evHttpHandleRequest(struct evhttp_request *req, void *pRouter) {

    if (req == nullptr) {
        throw std::logic_error("evHttpHandleRequest: request is a nullptr!");
    }

    if (pRouter == nullptr) {
        throw std::logic_error("evHttpHandleRequest: router is a nullptr!");
    }

    // do all the actual work in a new thread
    eventPool.emplace_back(
        std::async(std::launch::async, [req, pRouter]() -> evHttpResponse {
            evHttpResponse resp(req);
            Request r;
            r.url = iti::http::Uri::parse(evhttp_request_get_uri(req));

            // populate request method
            switch (req->type) {
            case EVHTTP_REQ_GET:
                r.method = iti::http::Method::GET;
                break;
            case EVHTTP_REQ_POST:
                r.method = iti::http::Method::POST;
                break;
            case EVHTTP_REQ_HEAD:
                r.method = iti::http::Method::HEAD;
                break;
            case EVHTTP_REQ_PUT:
                r.method = iti::http::Method::PUT;
                break;
            case EVHTTP_REQ_DELETE:
                r.method = iti::http::Method::DEL;
                break;
            case EVHTTP_REQ_OPTIONS:
                r.method = iti::http::Method::OPTIONS;
                break;
            case EVHTTP_REQ_TRACE:
                r.method = iti::http::Method::TRACE;
                break;
            case EVHTTP_REQ_CONNECT:
                r.method = iti::http::Method::CONNECT;
                break;
            case EVHTTP_REQ_PATCH:
                r.method = iti::http::Method::PATCH;
                break;
            default:
                r.method = iti::http::Method::unknown;
            }

            // populate request headers
            auto const headers = evhttp_request_get_input_headers(req);
            for (auto header = headers->tqh_first; header;
                 header      = header->next.tqe_next) {
                r.header.add(header->key, header->value);
            }

            // pull the body content out of the request
            auto buf    = evhttp_request_get_input_buffer(req);
            auto bufLen = evbuffer_get_length(buf);

            if (bufLen > 0) {
                r.body.resize(bufLen);
                evbuffer_remove(buf, r.body.data(), bufLen);
                r.body.resize(r.body.find('\0'));
            }

            ((Mux *)pRouter)->handle_request(r, resp);

            return resp;
        }));
}

// primary application entry point
int main() {
    // get the HTTP server running
#ifdef _WIN32
    // init the Winsock DLL
    WSADATA WSAData;
    WSAStartup(0x101, &WSAData);
#endif

    CfgService &cfg = CfgService::GetInstance();

    std::shared_ptr<Mux> router = std::make_shared<Mux>();

    iti::ProductHandlerFactory factory;
    std::shared_ptr<iti::IProductHandler> productHandler;
    productHandler.reset(factory.Create(iti::ProductHandlerType::MSSQL));
    std::wstring configJson = TEXT(R"({"ConnStr" : ")") + StrToWstr(cfg.GetConnectionString()) +
                              + TEXT(R"("}")");
    if (productHandler == nullptr) {
        std::cerr << "Couldn't create MSSQL product handler\n";
        return 1;
    } else if (auto err =
                   productHandler->Init(configJson, nullptr);
               err != iti::IProductHandler::ErrorCode::SUCCESS) {
        std::cerr << "productHandler->Init() failed: " << (int)err << '\n';
        return 1;
    }

    // add all the routes we want to handle to the router
    router->use(middlewares::trim_trailing_slash);
    router->use(middlewares::logging);
    router->get("/", [](const Request &req, Response &resp) {
        std::cout << "Hi There!" << '\n';
        resp.write("Hello from main.cpp");
    });

    // API routes for "products" resource
    router->route("/api/v1/products", [&productHandler](
                                          std::shared_ptr<IRouter> r) {
        r->get("/", [&productHandler](const Request &req, Response &resp) {
            resp.header.set("Content-Type", "application/json");

            // TODO:
            // * pull products from the database
            // * limit?
            // * paginate?
            std::wstring jsonStrW;
            if (auto err = productHandler->GetProductDefinitions(
                    L"", L"", iti::IProductHandler::StrList(),
                    iti::IProductHandler::StrList(), CfgService::GetInstance().GetPageSize(),
                    jsonStrW, nullptr);
                err == iti::IProductHandler::ErrorCode::SUCCESS) {
                std::string jsonStr = WstrToStr(jsonStrW);
                json j2             = json::parse(jsonStr);
                json j;
                j["products"] = j2;
                resp.write(j.dump(4));
            }
        });
        r->with(middlewares::extract_id)
            ->get("/{id:[\\d]+}",
                  [&productHandler](const Request &req, Response &resp) {
                resp.header.set("Content-Type", "application/json");

                long long id;
                req.context.try_get_value("id", id);

                // TODO:
                // pull product from the database
                std::wstring jsonStrW;
                if (auto err =
                        productHandler->GetProductDefinitionById(id, jsonStrW);
                    err == iti::IProductHandler::ErrorCode::SUCCESS) {

                    // json product;
                    // product["id"]   = id;
                    // product["name"] = fmt::format("Fake Product {:d}", id);

                std::string jsonStr = WstrToStr(jsonStrW);
                    json j2             = json::parse(jsonStr);
                    json j;
                    j["product"] = j2;
                    resp.write(j.dump(4));
                }
            });
    });

    // create event base and http server
    auto evbase = event_base_new();
    auto server = evhttp_new(evbase);

    // bind callbacks
    evhttp_set_gencb(server, evHttpHandleRequest, router.get());

    // bind http server to socket
    uint16_t port = cfg.GetServerPort();
    auto ok       = evhttp_bind_socket(server, "127.0.0.1", port);

    if (ok == 0) {
        std::cout << "HTTP Server bound to 127.0.0.1:" << port << '\n';

        // start processing events
        while (true) {
            // process any new requests (add request to the pool)
            auto rtn = event_base_loop(evbase, EVLOOP_NONBLOCK);
            if (rtn == -1) {
                std::cerr << "Error with event loop!" << '\n';
                if (productHandler != nullptr)
                    productHandler->Shutdown();
                return 1;
            }

            // check if we can send responses
            for (auto it = eventPool.begin(); it != eventPool.end();) {
                if (it->wait_for(0ns) == std::future_status::ready) {
                    auto resp = it->get();
                    if (resp.get_ready_to_send()) {
                        resp.process_response();
                    }
                    if (resp.get_response_sent()) {
                        it = eventPool.erase(it);
                    } else {
                        // if we didn't send the response for some reason then
                        // we'll try again later
                        it++;
                    }
                } else {
                    // future isn't ready for processing
                    // check back later
                    it++;
                }
            }
        }
    } else {
        std::cerr << "Could not bind to 127.0.0.1:" << port << '\n';
    }

    if (productHandler != nullptr)
        productHandler->Shutdown();

    return 0;
}
