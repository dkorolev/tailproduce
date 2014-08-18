// TODO(dkorolev): Add mutex guards for registration / deregistration of TCP handlers.

// NOTE: This header requires g++ on my machine, clang++ can't compile it. -- D.K.
//
// Uses boost::asio.
//
// Maintains a multithreaded TCP server. Runs requests asynchoronously and is asynchronous itself.
// Implemented as a singleton to avoid server tear down issues. Friendly with TailProduce unit tests using this.
// Once certain port has been open for listening, listening on it never stops. Handlers may change though.
//
// Usage:
//
// struct MyHandler {
//     void HandleRequestSync(std::unique_ptr<boost::asio::ip::tcp::socket>&& socket) {
//         // ...
//     }
// };
//
// MyHandler handler;
//
// // Manual case.
// TCPServer::Instance()[8080].RegisterHandler(handler);
// std::this_thread::sleep_for(std::chrono::seconds(30));
// TCPServer::Instance()[8080].UnregisterHandler();
//
// // Automated case.
// {
//     TCPServer::ScopedHandlerRegisterer scope(8080, handler);
//     std::this_thread::sleep_for(std::chrono::seconds(30));
// }
//
// // Automated case with HTTP response, `curl`-able and openable by the browser.
// {
//     auto handler = ::TailProduce::HTTPResponseHandler([]() { return "Hello, World!\n"; });
//     ::TailProduce::TCPServer::ScopedHandlerRegisterer scope(8080, handler);
//     std::this_thread::sleep_for(std::chrono::seconds(30));
// }

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <memory>
#include <mutex>
#include <thread>

#include <glog/logging.h>

#include <boost/asio.hpp>

#include "tp_exceptions.h"

namespace TailProduce {
    struct TCPServer {
        using tcp = boost::asio::ip::tcp;

        struct PerPortConnectionAccepter {
            struct Handler {
                virtual void HandleRequest(std::unique_ptr<tcp::socket>&& socket) = 0;
            };
            template <typename HANDLER> struct UserHandlerWrapper : Handler {
                HANDLER& user_handler;
                explicit UserHandlerWrapper(HANDLER& user_handler) : user_handler(user_handler) {
                }
                virtual void HandleRequest(std::unique_ptr<tcp::socket>&& socket) {
                    user_handler.HandleRequestSync(std::move(socket));
                }
                UserHandlerWrapper() = delete;
                UserHandlerWrapper(const UserHandlerWrapper&) = delete;
                void operator=(const UserHandlerWrapper&) = delete;
            };

            boost::asio::io_service io_service_;
            tcp::acceptor acceptor_;
            std::unique_ptr<Handler> handler_;
            std::mutex handler_mutex_;

            PerPortConnectionAccepter(size_t port)
                : io_service_(), acceptor_(io_service_, tcp::endpoint(tcp::v4(), port)) {
                std::thread(&PerPortConnectionAccepter::ServingThread, this).detach();
            }
            void ServingThread() {
                for (;;) {
                    try {
                        std::unique_ptr<tcp::socket> socket(new tcp::socket(io_service_));
                        acceptor_.accept(*socket);
                        {
                            std::lock_guard<std::mutex> guard(handler_mutex_);
                            if (handler_) {
                                std::thread(&Handler::HandleRequest, handler_.get(), std::move(socket)).detach();
                            } else {
                                std::string message = "500\n";
                                boost::asio::write(
                                    *socket, boost::asio::buffer(message), boost::asio::transfer_all());
                            }
                        }
                    } catch (std::exception& e) {
                        throw ::TailProduce::TCPServerRuntimeException(e.what());
                    }
                }
            }

            template <typename HANDLER> void RegisterHandler(HANDLER& handler) {
                std::lock_guard<std::mutex> guard(handler_mutex_);
                if (!handler_) {
                    handler_.reset(new UserHandlerWrapper<HANDLER>(handler));
                } else {
                    throw ::TailProduce::TCPServerLogicErrorException("RegisterHandler() called twice.");
                }
            }

            void UnregisterHandler() {
                std::lock_guard<std::mutex> guard(handler_mutex_);
                if (handler_) {
                    handler_.reset(nullptr);
                } else {
                    throw ::TailProduce::TCPServerLogicErrorException("UnregisterHandler() called with no handler.");
                }
            }

            PerPortConnectionAccepter() = delete;
            PerPortConnectionAccepter(const PerPortConnectionAccepter&) = delete;
            void operator=(const PerPortConnectionAccepter&) = delete;
        };

        std::map<size_t, std::unique_ptr<PerPortConnectionAccepter>> by_port_;
        std::mutex by_port_mutex_;

        PerPortConnectionAccepter& operator[](size_t port) {
            std::lock_guard<std::mutex> guard(by_port_mutex_);
            std::unique_ptr<PerPortConnectionAccepter>& ref = by_port_[port];
            if (!ref) {
                try {
                    VLOG(2) << "Creating server on port " << port;
                    ref.reset(new PerPortConnectionAccepter(port));
                    VLOG(2) << "Creating server on port " << port << ": Done.";
                } catch (std::exception& e) {
                    throw ::TailProduce::TCPServerSpawnException(e.what());
                }
            }
            return *ref;
        }

        static TCPServer& Instance() {
            static TCPServer instance;
            return instance;
        }

        struct ScopedHandlerRegisterer {
            size_t port;
            template <typename HANDLER> ScopedHandlerRegisterer(size_t port, HANDLER& handler) : port(port) {
                Instance()[port].RegisterHandler<HANDLER>(handler);
            }
            ~ScopedHandlerRegisterer() {
                Instance()[port].UnregisterHandler();
            }
        };
    };

    struct HTTPResponseHandler {
        std::function<std::string(void)> f_;
        std::mutex mutex;
        explicit HTTPResponseHandler(std::function<std::string(void)> f) : f_(f) {
        }
        HTTPResponseHandler(const HTTPResponseHandler& rhs) : f_(rhs.f_) {
        }
        void HandleRequestSync(std::unique_ptr<boost::asio::ip::tcp::socket>&& socket) {
            const std::string response = ([=]() {
                std::lock_guard<std::mutex> guard(mutex);
                return f_();
            })();

            std::ostringstream os;
            os << "HTTP/1.1 200 OK\n";
            os << "Content-type: text/html\n";
            os << "Content-length: " << response.length() << "\n";
            os << "\n";
            os << response;

            const std::string message = os.str();
            boost::asio::write(*socket, boost::asio::buffer(message), boost::asio::transfer_all());
        }
        HTTPResponseHandler() = delete;
        void operator=(const HTTPResponseHandler&) = delete;
    };
};

#endif
