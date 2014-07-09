#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>

namespace TailProduce {
    // Data storage proxy, originally LevelDB.
    struct Storage {
        typedef std::string KEY_TYPE;
        typedef std::vector<uint8_t> VALUE_TYPE;
        // TODO(dkorolev): Document the expected interface.
    };

    //struct Producer {};  // Client-defined job.
};

#endif
