#include "lib/ncm_classes.hpp"
#include "lib/ncm_error_codes.hpp"
#include "lib/ncm_functions.hpp"
#include "lib/ncm_libraries.hpp"


Node::Node(string node_ip, string node_username, string node_port, string node_password) {
    ipv4 = node_ip;
    username = node_username;
    port = node_port;
    password = node_password;
}

string Node::getNodeDetails() {
    return ipv4 + ":" + port + " connects as " + username + "\n";
}

bool Node::operator<(const Node& other) const {
    return (ipv4 < other.ipv4) || 
    (ipv4 == other.ipv4 && port < other.port) || 
    (ipv4 == other.ipv4 && port == other.port && username < other.username);
}

string Node::getIP() {
    return ipv4;
}

string Node::getUsername() {
    return username;
}

string Node::getPort() {
    return port;
}

string Node::getPassword() {
    return password;
}

STATUS_CODES help_menu(STATUS_CODES error) {
    cout << USAGE << endl;

    switch (error) {
        case MISSING_JSON:
            cerr << MISSING_JSON_ERROR << endl;
            return MISSING_JSON;
        case TOO_MUCH_ARGUMENTS:
            cerr << TOO_MUCH_ARGUMENTS_ERROR << endl;
            return TOO_MUCH_ARGUMENTS;
    }
}

json get_json(const char *devices) {
    ifstream file(devices);

    if (!file) {
        cerr << OPENING_FILE_ERROR << endl;
        return NULL;
    } 

    json nodes;
    file >> nodes;

    return nodes;
}

set<Node> get_nodes(json devices) {
    set<Node> network_nodes;

    for (const auto& device : devices.items()) {
        for (const auto& value : device.value().items()) {
            Node network_device(
                value.value()["ip"],
                value.value()["username"],
                value.value()["port"],
                value.value()["password"]
            );

            network_nodes.insert(network_device);
        }        
    }

    return network_nodes;
}

int backup_config(ssh_session session, string ip) {
    ssh_channel channel;
    int rc;
    char buffer[256];
    int nbytes;
    time_t timestamp = time(NULL);
    struct tm date = *localtime(&timestamp);
    char output[50];
    strftime(output, 50, "%d-%b-%y@%H:%M%S", &date);
    string backup_name = (string) output + "-" + ip + ".txt";
    string backup_path = "backups/" + ip + "/" + backup_name;


    channel = ssh_channel_new(session);
    if (!channel) {  
        return SSH_ERROR;
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        ssh_channel_free(channel);
        return rc;
    }

    rc = ssh_channel_request_exec(channel, "show running-config");
    if (rc != SSH_OK) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return rc;
    }

    ofstream CONF(backup_path);
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0) {
      nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
      CONF.write(buffer, nbytes); 
    }
   
    if (nbytes < 0) {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }
   
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
   
    return SSH_OK;
}

STATUS_CODES get_ssh_session(Node device) {
    ssh_session session = ssh_new();
    
    string node_ip = device.getIP();
    string node_port = device.getPort();
    string node_username = device.getUsername();
    string node_password = device.getPassword();

    const char *host = node_ip.c_str();
    const char *port = node_port.c_str();
    const char *username = node_username.c_str();
    const char *password = node_password.c_str();

    STATUS_CODES OPERATION_STATUS = OPERATION_COMPLETED;

    if (!session) {
        cerr << UNSUCCESSFULL_SSH_SESSION_ERROR << endl;
        return UNSUCCESSFULL_SSH_SESSION;
    } else {
        ssh_options_set(session, SSH_OPTIONS_HOST, host);
        ssh_options_set(session, SSH_OPTIONS_USER, username);
        ssh_options_set(session, SSH_OPTIONS_KEY_EXCHANGE, "diffie-hellman-group14-sha1");
        ssh_options_set(session, SSH_OPTIONS_HOSTKEYS, "ssh-rsa");
        ssh_options_set(session, SSH_OPTIONS_CIPHERS_C_S, "aes128-cbc");
        ssh_options_set(session, SSH_OPTIONS_CIPHERS_S_C, "aes128-cbc");
        ssh_options_set(session, SSH_OPTIONS_HMAC_C_S, "hmac-sha1");
        ssh_options_set(session, SSH_OPTIONS_HMAC_S_C, "hmac-sha1");

        if (ssh_connect(session) != SSH_OK) {
            fprintf(stderr, CANNOT_CONNECT_TO_HOST_ERROR.c_str(),
            ssh_get_error(session));
            ssh_free(session);
            return CANNOT_CONNECT_TO_HOST;
        }

        if (ssh_userauth_password(session, NULL, password) != SSH_AUTH_SUCCESS) {
            ssh_disconnect(session);
            ssh_free(session);
            return WRONG_PASSWORD;
        }

        if (backup_config(session, device.getIP()) != 0) 
            OPERATION_STATUS = CANNOT_BACKUP;
        
        ssh_disconnect(session);
        ssh_free(session);

        return OPERATION_STATUS;
    }
        
}