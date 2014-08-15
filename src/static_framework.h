#ifndef STATIC_FRAMEWORK_H
#define STATIC_FRAMEWORK_H

#include <algorithm>
#include <memory>
#include <set>
#include <string>

#include <boost/asio.hpp>

#include "stream_manager.h"
#include "stream_manager_params.h"
#include "config_values.h"
#include "tcp_server_singleton.h"

namespace TailProduce {
    inline void EnsureThereAreNoStreamsWithoutPublishers(const std::set<std::string>& streams_declared,
                                                         const std::set<std::string>& stream_publishers_declared) {
        std::vector<std::string> diff;
        std::set_difference(streams_declared.begin(),
                            streams_declared.end(),
                            stream_publishers_declared.begin(),
                            stream_publishers_declared.end(),
                            std::back_inserter(diff));
        if (!diff.empty()) {
            std::ostringstream os;
            for (const auto cit : diff) {
                os << ',' << cit;
                VLOG(3) << "Stream '" << cit << "' has been declared but has no writer associated with it.";
            }
            throw ::TailProduce::StreamHasNoWriterDefinedException(os.str().substr(1));
        }
    }

    template <typename BASE> class StaticFramework {
      public:
        typedef BASE T_BASE;
        typedef typename T_BASE::T_STORAGE T_STORAGE;
        T_STORAGE& storage;

        std::set<std::string> streams_declared_;
        std::set<std::string> stream_publishers_declared_;

        std::map<std::string, ::TailProduce::StreamExporter*> exporters_;

        void HandleRequestSync(std::unique_ptr<boost::asio::ip::tcp::socket>&& socket) {
            std::string query;
            char c;
            while (true) {
                boost::asio::read(*socket, boost::asio::buffer(&c, 1), boost::asio::transfer_all());
                if (c == '\n' || c == '\r') {
                    break;
                }
                query += c;
            }

            std::istringstream is(query);

            std::string query_resource;

            is >> query_resource;
            if (query_resource == "GET") {
                is >> query_resource;
            }

            VLOG(2) << this << " StaticFramework::HandleRequestSync(\"" << query_resource << "\")";
            VLOG(2) << this << " StaticFramework::HandleRequestSync(): " << exporters_.size() << " exporters.";

            auto cit = exporters_.find(query_resource);
            if (cit != exporters_.end()) {
                cit->second->ListenAndStreamData(std::move(socket));
            } else {
                std::string response = "Not found: " + query_resource + "\n";
                for (auto cit : exporters_) {
                    response += cit.first + '\n';
                }
                response += "That's it.\n";

                std::ostringstream os;
                os << "HTTP/1.1 200 OK\n";
                os << "Content-type: text/html\n";
                os << "Content-length: " << response.length() << "\n";
                os << "\n";
                os << response;

                const std::string message = os.str();
                boost::asio::write(*socket, boost::asio::buffer(message), boost::asio::transfer_all());
            }
        }
        std::unique_ptr<::TailProduce::TCPServer::ScopedHandlerRegisterer> scoped_http_handler_registerer;

        void AddExporter(const std::string& endpoint, ::TailProduce::StreamExporter* handler) {
            // TODO(dkorolev): Add more logic here. Add a scoped adder.
            VLOG(2) << this << " StaticFramework::AddExporter(\"" << endpoint << "\")";
            exporters_[endpoint] = handler;
        }

        void RemoveExporter(const std::string& endpoint, ::TailProduce::StreamExporter* handler) {
            VLOG(2) << this << " StaticFramework::RemoveExporter(\"" << endpoint << "\")";
            // TODO(dkorolev): Add more logic here.
        }

        StaticFramework(T_STORAGE& storage,
                        const ::TailProduce::ConfigValues& cv,
                        const ::TailProduce::StreamManagerParams& params =
                            ::TailProduce::StreamManagerParams::FromCommandLineFlags())
            : storage(EnsureStreamsAreCreatedDuringInitialization(storage, cv, params)) {
            ::TailProduce::EnsureThereAreNoStreamsWithoutPublishers(streams_declared_, stream_publishers_declared_);
            scoped_http_handler_registerer.reset(new ::TailProduce::TCPServer::ScopedHandlerRegisterer(8080, *this));
        }

        static T_STORAGE& EnsureStreamsAreCreatedDuringInitialization(
            T_STORAGE& storage,
            const ::TailProduce::ConfigValues& cv,
            const ::TailProduce::StreamManagerParams& params) {
            params.Apply(storage, cv);
            return storage;
        }

      private:
        StaticFramework(const StaticFramework&) = delete;
        StaticFramework(StaticFramework&&) = delete;
        void operator=(const StaticFramework&) = delete;
        using TSM = ::TailProduce::StreamManagerBase;
        static_assert(std::is_base_of<TSM, T_BASE>::value,
                      "StaticFramework: T_BASE should be derived from StreamManagerBase.");
        using TS = ::TailProduce::Storage::Internal::Interface;
        static_assert(std::is_base_of<TS, T_STORAGE>::value,
                      "StaticFramework: T_BASE::T_STORAGE should be derived from Storage.");
    };
};

#endif
