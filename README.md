# NCM++

NCM++ is a C++ Linux program to backup config files from cisco network devices!

# PREREQUISITES

1. **libssh** library installed on the system
2. create a **github** repository in the program directory. Add a .gitignore file to only push the **backups** directory

## HOW TO USE IT

1. Compile the program:

```bash
g++ ncm.cpp ncm_core.cpp -lssh -o ncm++
```

2. Define the network nodes in a **data.json** file:

```json
{
    "nodes": [
        {
            "ip": "1.1.1.1",
            "username": "admin",
            "port": "22",
            "password": "admin"
        },

        {
            "ip": "1.1.1.2",
            "username": "admin",
            "port": "22",
            "password": "admin"
        }
    ]
}
```

3. Run the program:

```bash
./ncm++ data.json
```

Backups will appear under a directory named **backups**.
