#ifndef ENTRY_H
#define ENTRY_H

namespace TailProduce {
    // A serializable entry.
    template<typename T1, typename T2>
    struct OrderKeyExtractorImpl {};
    struct Entry {
        // 1) Need the following fully specialized template within namespace ::TailProduce:
        //       template<> struct OrderKeyExtractorImpl<OrderKeyType, EntryType> {
        //           static OrderKeyType ExtractOrderKey(const EntryType& entry) { ... }
        //        };
        //    for each desired pair of { OrderKeyType, EntryType }.
        // 2) Needs template<typename T> static void SerializeEntry(std::ostream& os, const T& entry);
        // 3) Needs template<typename T> static void DeSerializeEntry(std::istream& is, T& entry);
    };
};

#endif
