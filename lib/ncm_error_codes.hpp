#ifndef NCM_MANAGEMENT_SYSTEM
#define NCM_MANAGEMENT_SYSTEM


#include "ncm_libraries.hpp"


typedef enum {
    OPERATION_COMPLETED,
    MISSING_JSON,
    TOO_MUCH_ARGUMENTS,
    ERROR_OPENING_FILE,
    UNSUCCESSFULL_SSH_SESSION,
    CANNOT_CONNECT_TO_HOST,
    WRONG_PASSWORD,
    CANNOT_BACKUP
} STATUS_CODES;

const string USAGE = "usage: ncm++ file.json\n";
const string MISSING_JSON_ERROR = "ncm++: missing json file, unable to know what to backup\n";
const string TOO_MUCH_ARGUMENTS_ERROR = "ncm++: expecting one argument but more were provided\n";
const string OPENING_FILE_ERROR = "ncm++: error opening file\n";
const string UNSUCCESSFULL_SSH_SESSION_ERROR = "ncm++: cannot establish ssh session\n";
const string CANNOT_CONNECT_TO_HOST_ERROR = "ncm++: cannot establish a connection with host\n";
const string WRONG_PASSWORD_ERROR = "ncm++: wrong password!\n";
const string CANNOT_BACKUP_ERROR = "ncm++: cannot backup\n";


#endif 