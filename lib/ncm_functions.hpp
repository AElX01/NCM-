#ifndef NCM_FUNCTIONS
#define NCM_FUNCTIONS


#include "ncm_error_codes.hpp"
#include "ncm_libraries.hpp"


STATUS_CODES help_menu(STATUS_CODES error);
json get_json(const char *json_file);
set<Node> get_nodes(json devices);
STATUS_CODES get_ssh_session(Node device);
void compare_backups();


#endif