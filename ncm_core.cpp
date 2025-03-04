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

// NETWORK NODES ARE COMPARED BASED ON THEIR IPS WHEN INSERTING THEM IN A SET
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
    json nodes;

    if (!file) {
        cerr << OPENING_FILE_ERROR << endl;
        return nodes;
    } 

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

//COMPARE IF BOTH FILES HAVE THE SAME CONTENTS
bool files_are_equal(const string& file1, const string& file2) {
    ifstream f1(file1, ios::binary);
    ifstream f2(file2, ios::binary);

    if (!f1 || !f2) return false;

    if (fs::file_size(file1) != fs::file_size(file2)) return false;

    return equal(istreambuf_iterator<char>(f1), istreambuf_iterator<char>(),
                 istreambuf_iterator<char>(f2));
}

void compare_backups() {
    const string base_directory = "backups";
    unordered_map<string, vector<fs::path>> backup_groups;

    // GROUP FILES ACCORDING TO THEIR PARENT DIRECTORY
    for (const auto& entry : fs::recursive_directory_iterator(base_directory)) {
        if (!entry.is_directory()) {
            string parent_dir = entry.path().parent_path().string();
            backup_groups[parent_dir].push_back(entry.path());
        }
    }

    // COMPARE FILES INSIDE THEIR DIRECTORIES
    for (auto& group : backup_groups) {
        vector<fs::path>& backups = group.second;

        // SORTS FILES BY DATE
        sort(backups.begin(), backups.end(), [](const fs::path& a, const fs::path& b) {
            return a.filename().string() < b.filename().string();
        });

        for (size_t i = 0; i < backups.size(); ++i) {
            for (size_t j = i + 1; j < backups.size(); ++j) {
                if (fs::exists(backups[i]) && fs::exists(backups[j])) {
                    if (files_are_equal(backups[i], backups[j])) {
                        fs::remove(backups[j]); // REMOVE NEWEST DUPLICATED FILE
                    } else {
                        fs::remove(backups[i]); // REMOVE OLDEST BACKUP IF FILES ARE DIFFERENT
                    }
                }
            }
        }
    }
}


int backup_config(ssh_session session, string ip) {
    ssh_channel channel;
    int rc;
    char buffer[256];
    int nbytes;
    time_t timestamp = time(NULL);
    struct tm date = *localtime(&timestamp);
    char output[50];
    strftime(output, 50, "%d-%b-%y@%H:%M:%S", &date);
    string backup_name = (string) output + "-" + ip + ".txt"; // BACKUP NAME FORMAT IN FORMAT %d-%b-%y@%H:%M:%S-x.x.x.x.txt
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

    rc = ssh_channel_request_exec(channel, "show running-config"); // GET THE FILE CONFIGS WITH THE "show run" COMMAND
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
    
    string node_host = device.getIP();
    string node_port = device.getPort();
    string node_username = device.getUsername();
    string node_password = device.getPassword();

    const char *host = node_host.c_str();
    int port = stoi(node_port);
    const char *username = node_username.c_str();
    const char *password = node_password.c_str();

    STATUS_CODES OPERATION_STATUS = OPERATION_COMPLETED;

    if (!session) {
        cerr << UNSUCCESSFULL_SSH_SESSION_ERROR << endl;
        return UNSUCCESSFULL_SSH_SESSION;
    } else {
        ssh_options_set(session, SSH_OPTIONS_HOST, host);
        ssh_options_set(session, SSH_OPTIONS_USER, username);
        ssh_options_set(session, SSH_OPTIONS_PASSWORD_AUTH, password);
        ssh_options_set(session, SSH_OPTIONS_PORT, &port);
        
        /*
        NETWORK DEVICES MIGHT BE RUNNING A CISCO IOS VERSION THAT DOES NOT SUPPORT MODERN CIPHERS,
        THE PROGRAM WILL TRY TO CONNECT VIA SSH WITH SECURE ENOUGH CIPHERS TO HAVE FAST CONNECTIVITY,
        SECURITY AND COMPATIBILITY
        */
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

        if (backup_config(session, device.getIP())) 
            OPERATION_STATUS = CANNOT_BACKUP;
        
        ssh_disconnect(session);
        ssh_free(session);

        return OPERATION_STATUS;
    }
        
}