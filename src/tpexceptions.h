#ifndef TAILPRODUCE_EXCEPTIONS_H
#define TAILPRODUCE_EXCEPTIONS_H

namespace TailProduce {
    // Exception types.
    struct Exception : std::exception {};
    struct InternalError : Exception {};
    struct StorageException : Exception {};
    struct StorageEmptyKeyException : StorageException {};
    struct StorageEmptyValueException : StorageException {};
    struct StorageNoDataException : StorageException {};
    struct StorageOverwriteNotAllowedException : StorageException {};
    struct StorageIteratorOutOfBoundsException : StorageException {};
    struct OrderKeysGoBackwardsException : Exception {};
    struct ListenerHasNoDataToRead : Exception {};
    struct AttemptedToAdvanceListenerWithNoDataAvailable : Exception {};
    struct StreamDoesNotExistException : Exception {};
    struct MalformedStorageHeadException : Exception {};
    struct StreamAlreadyListedForCreationException : Exception {};
    struct StreamAlreadyExistsException : Exception {};
};

#endif
