#ifndef NCM_CLASSES
#define NCM_CLASSES


#include "ncm_libraries.hpp"


class Node {
    private:
        string ipv4;
        string username;
        string port;
        string password;

    public:
        Node(string node_ip, string node_username, string node_port, string node_password);
        bool operator<(const Node& other) const;
        string getIP();
        string getUsername();
        string getPort();
        string getNodeDetails();
        string getPassword();
};


#endif