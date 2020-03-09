#ifndef P2P_TECHDEMO_LIB_CORE_ERRORS_H_
#define P2P_TECHDEMO_LIB_CORE_ERRORS_H_

enum class Error : int {
    ERR_ALREADY_IN_USE,
    ERR_BUSY,
    CANT_ALLOCATE,
    CANT_CREATE,
    DOES_NOT_EXIST,
    ERR_INVALID_PARAMETER,
    ERR_UNAVAILABLE,
    ERR_UNCONFIGURED,
    ERROR,
    FAILED,
    OK
};

#endif // P2P_TECHDEMO_LIB_CORE_ERRORS_H_
