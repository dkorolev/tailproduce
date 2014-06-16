
#ifndef _NOTIFIER_H
#define _NOTIFIER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace TailProduce {

    typedef std::function<void(std::string const&, 
                               std::string const&, 
                               std::string const &)> NotifierCallback;

    class Notifier {
    public:
        Notifier()=default;
    
        void notify(std::string const& event, std::string const& streamId, std::string const& value);
        void registerCallback(std::string const& event, 
                              std::string const& streamId, 
                              NotifierCallback cb);

    private:
        std::string createKey_(std::string event, std::string const& streamId) {
            return event + streamId;
        };

        std::unordered_map<std::string, std::vector<NotifierCallback>> callbacks_;
    };
};

#endif

