#include "notifier.h"
void 
TailProduce::Notifier::Notify(std::string const& event, 
                              std::string const& streamId, 
                              std::string const& value) {
    // check to see if we have any callbacks associated with this event and stream.  If we
    // do then call each one.
    auto v = callbacks_.find(createKey_(event, streamId));
    if ( v != callbacks_.end()) {
        for(auto f : v->second) {
            if (f) {
                (f)(event, streamId, value);  // calling the callback
            }
        }
    }
};

void 
TailProduce::Notifier::RegisterCallback(std::string const& event,
                                        std::string const& streamId, 
                                        TailProduce::NotifierCallback cb) {
    auto v = callbacks_.find(createKey_(event, streamId));
    if ( v == callbacks_.end()) {
        std::vector<TailProduce::NotifierCallback> vc;
        callbacks_[createKey_(event, streamId)] = vc;
        v = callbacks_.find(createKey_(event, streamId)); // now get the object we just created....
    }
    v->second.push_back(cb);
};

