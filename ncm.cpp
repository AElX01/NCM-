#include "lib/ncm_classes.hpp"
#include "lib/ncm_error_codes.hpp"
#include "lib/ncm_functions.hpp"
#include "lib/ncm_libraries.hpp"


int main(int argc, char *argv[]) {
    if (argc < 2) 
        return help_menu(MISSING_JSON);
    else if (argc > 2) 
        return help_menu(TOO_MUCH_ARGUMENTS);
    
    json file = get_json(argv[1]);
    set<Node> network_nodes = get_nodes(file);
    mkdir("backups", 0700);

    for (Node node : network_nodes) {
        string node_directory = "backups/" + node.getIP();
        mkdir(node_directory.c_str(), 0700);

        get_ssh_session(node);
    }

    return OPERATION_COMPLETED;
}