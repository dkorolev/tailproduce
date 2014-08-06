#ifndef STATIC_FRAMEWORK_H
#define STATIC_FRAMEWORK_H

#include <algorithm>
#include <string>
#include <set>

#include "stream_manager.h"
#include "stream_manager_params.h"

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
        typedef typename BASE::storage_type storage_type;
        storage_type& storage;
        std::set<std::string> streams_declared_;
        std::set<std::string> stream_publishers_declared_;

        StaticFramework(storage_type& storage,
                        const ::TailProduce::StreamManagerParams& params =
                            ::TailProduce::StreamManagerParams::FromCommandLineFlags())
            : storage(EnsureStreamsAreCreatedDuringInitialization(storage, params)) {
            ::TailProduce::EnsureThereAreNoStreamsWithoutPublishers(streams_declared_, stream_publishers_declared_);
        }

        static storage_type& EnsureStreamsAreCreatedDuringInitialization(
            storage_type& storage,
            const ::TailProduce::StreamManagerParams& params) {
            params.Apply(storage);
            return storage;
        }

      private:
        StaticFramework(const StaticFramework&) = delete;
        StaticFramework(StaticFramework&&) = delete;
        void operator=(const StaticFramework&) = delete;
        using TSM = ::TailProduce::StreamManagerBase;
        static_assert(std::is_base_of<TSM, BASE>::value,
                      "StaticFramework: BASE should be derived from StreamManagerBase.");
        using TS = ::TailProduce::Storage::Internal::Interface;
        static_assert(std::is_base_of<TS, storage_type>::value,
                      "StaticFramework: BASE::storage_type should be derived from Storage.");
    };
};

#endif
