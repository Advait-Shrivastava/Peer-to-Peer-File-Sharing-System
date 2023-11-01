#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <thread>
#include <fstream>
#include <bits/stdc++.h>
#define BLOCK_SIZE 1024
#define FILE_USER_DETAILS "./data/user_details.txt"
#define FILE_GROUP_ADMIN "./data/group_admin.txt"
#define FILE_GROUP_MEMBER "./data/group_members.txt"
#define FILE_PENDING_REQUEST "./data/pending_request.txt"
#define FILE_GROUP_FILE_SEEDER "./data/grp_file_seeders.txt"
#define FILE_GROUP_FILES "./data/group_files.txt"
#define FILE_FILENAME_ITS_PATH "./data/filename_path.txt"
#define FILE_GRP_USR_FILE "./data/user_file.txt"
#define FILE_GRP_FILE_SIZE "./data/file_filesize.txt"
#define FILE_GRP_FILE_SHA "./data/file_sha.txt"
using namespace std;

string IP_of_TRACKER;
int PORT_of_TRACKER;

unordered_map<string, string> user_pass;
unordered_map<string, string> login_user_port;
unordered_map<string, string> group_and_its_admin;
// unordered_map<string, vector<string>> group_and_its_users;
unordered_map<string, set<string>> group_and_its_users;
unordered_map<string, set<string>> groups_pending_request;

unordered_map<string, set<string>> groups_and_files;      // group->files;
unordered_map<string, set<string>> group_files_sleechers; // (group+files) ->seeders;     separated by space
unordered_map<string, string> grp_user_file_and_its_path;
unordered_map<string, set<string>> grp_user_to_file;

unordered_map<string, long long> file_size; // grp+filename --> filesize
unordered_map<string, string> file_SHA;     // grp+filename -->SHA

struct struct_user_details
{
    char user[50];
    char password[50];

    void insert(string u, string p)
    {
        strcpy(user, u.c_str());
        strcpy(password, p.c_str());
        return;
    }
};

struct struct_group_and_admin_details
{
    char group[50];
    char admin[50];

    void insert(string u, string p)
    {
        strcpy(group, u.c_str());
        strcpy(admin, p.c_str());
        return;
    }
};

struct struct_group_and_members
{
    char group[50];
    char member[50];

    void insert(string u, string p)
    {
        strcpy(group, u.c_str());
        strcpy(member, p.c_str());
        return;
    }
};

struct struct_waiting_to_join
{
    char group[50];
    char member[50];

    void insert(string u, string p)
    {
        strcpy(group, u.c_str());
        strcpy(member, p.c_str());
        return;
    }
};

struct struct_group_file_sleechers
{
    char group_file[50];
    char seeders[50];

    void insert(string u, string p)
    {
        strcpy(group_file, u.c_str());
        strcpy(seeders, p.c_str());
        return;
    }
};

struct struct_group_files
{
    char group[50];
    char files[50];

    void insert(string u, string p)
    {
        strcpy(group, u.c_str());
        strcpy(files, p.c_str());
        return;
    }
};

struct struct_grp_user_file_paths
{
    char file[50];
    char path[50];

    void insert(string u, string p)
    {
        strcpy(file, u.c_str());
        strcpy(path, p.c_str());
        return;
    }
};

struct struct_grp_user_to_file
{
    char user[50];
    char file[50];

    void insert(string u, string p)
    {
        strcpy(user, u.c_str());
        strcpy(file, p.c_str());
        return;
    }
};

struct struct_grp_filename_to_size
{
    char file[50];
    char size[50];

    void insert(string u, string p)
    {
        strcpy(file, u.c_str());
        strcpy(size, p.c_str());
        return;
    }
};

struct struct_grp_filename_to_SHA
{
    char file[50];
    char sha[40];

    void insert(string u, string p)
    {
        strcpy(file, u.c_str());
        strcpy(sha, p.c_str());
        return;
    }
};

void group_and_user_to_file()
{
    fstream gu;
    gu.open(FILE_GROUP_MEMBER, ios::out | ios::trunc);
    for (auto it : group_and_its_users)
    {
        for (auto a : it.second)
        {
            // cout << it.first << " " << a << endl;
            struct_group_and_members new_member;
            new_member.insert(it.first, a);
            gu.write((char *)&new_member, sizeof(new_member));
        }
    }
    gu.close();
}

void group_and_admin_to_file()
{
    fstream ga;
    ga.open(FILE_GROUP_ADMIN, ios::out | ios::trunc);
    for (auto it : group_and_its_admin)
    {

        struct_group_and_admin_details new_admin;
        new_admin.insert(it.first, it.second);
        ga.write((char *)&new_admin, sizeof(new_admin));
    }
    ga.close();
}

void group_and_pending_request_to_file()
{
    fstream pr;
    pr.open(FILE_PENDING_REQUEST, ios::out | ios::trunc);
    for (auto it : groups_pending_request)
    {
        for (auto a : it.second)
        {
            struct_waiting_to_join waiting_member;
            waiting_member.insert(it.first, a);
            pr.write((char *)&waiting_member, sizeof(waiting_member));
        }
    }
    pr.close();
}

void group_and_files_to_file()
{
    fstream pr;
    pr.open(FILE_GROUP_FILES, ios::out | ios::trunc);
    for (auto it : groups_and_files)
    {
        for (auto a : it.second)
        {
            struct_group_files grp;
            grp.insert(it.first, a);
            pr.write((char *)&grp, sizeof(grp));
        }
    }
    pr.close();
}

void group_and_sleechers_to_file()
{
    fstream pr;
    pr.open(FILE_GROUP_FILE_SEEDER, ios::out | ios::trunc);
    for (auto it : group_files_sleechers)
    {
        for (auto a : it.second)
        {
            struct_group_file_sleechers grp;
            grp.insert(it.first, a);
            pr.write((char *)&grp, sizeof(grp));
        }
    }
    pr.close();
}

void user_file_path_to_file()
{
    fstream ga;
    ga.open(FILE_FILENAME_ITS_PATH, ios::out | ios::trunc);
    for (auto it : grp_user_file_and_its_path)
    {

        struct_grp_user_file_paths path;
        path.insert(it.first, it.second);
        ga.write((char *)&path, sizeof(path));
    }
    ga.close();
}

void group_user_file_to_file()
{
    fstream pr;
    pr.open(FILE_GRP_USR_FILE, ios::out | ios::trunc);
    for (auto it : grp_user_to_file)
    {
        for (auto a : it.second)
        {
            struct_grp_user_to_file grp;
            grp.insert(it.first, a);
            pr.write((char *)&grp, sizeof(grp));
        }
    }
    pr.close();
}

void group_filesize_to_file()
{
    fstream ga;
    ga.open(FILE_GRP_FILE_SIZE, ios::out | ios::trunc);
    for (auto it : file_size)
    {

        struct_grp_filename_to_size file;
        file.insert(it.first, to_string(it.second));
        ga.write((char *)&file, sizeof(file));
    }
    ga.close();
}

void grp_file_sha_to_file()
{
    fstream ga;
    ga.open(FILE_GRP_FILE_SHA, ios::out | ios::trunc);
    for (auto it : file_SHA)
    {

        struct_grp_filename_to_SHA file;
        file.insert(it.first, it.second);
        ga.write((char *)&file, sizeof(file));
    }
    ga.close();
}

string stop_share_function(string a, string b, string c)
{

    string file_name = a;
    string group_name = b;
    string user = c;

    auto requested_group = group_and_its_admin.find(b);

    if (requested_group == group_and_its_admin.end())
    {
        return "Group ID does not exist";
    }

    if (group_and_its_users[group_name].find(user) == group_and_its_users[group_name].end())
    {
        return "You are not member of this group";
    }

    if (groups_and_files[group_name].find(file_name) == groups_and_files[group_name].end())
    {
        return "Filename not present in group";
    }

    if (grp_user_to_file.find(group_name + " " + user) == grp_user_to_file.end())
    {
        return "User not sharing any file";
    }

    if (group_files_sleechers[group_name + " " + file_name].size() == 1)
    {
        group_files_sleechers.erase(group_name + " " + file_name);
        grp_user_file_and_its_path.erase(group_name + " " + user + " " + file_name);

        auto it = grp_user_to_file[group_name + " " + user].find(file_name);
        if (it != grp_user_to_file[group_name + " " + user].end())
        {
            grp_user_to_file[group_name + " " + user].erase(it);
            if (grp_user_to_file[group_name + " " + user].size() == 0)
            {
                grp_user_to_file.erase(group_name + " " + user);
            }
        }

        it = groups_and_files[group_name].find(file_name);

        if (it != groups_and_files[group_name].end())
        {
            groups_and_files[group_name].erase(it);
            if (groups_and_files[group_name].size() == 0)
            {
                groups_and_files.erase(group_name);
            }
        }
        file_size.erase(group_name + " " + file_name);
        file_SHA.erase(group_name + " " + file_name);

        group_and_sleechers_to_file();
        user_file_path_to_file();
        group_user_file_to_file();
        group_and_files_to_file();
        group_filesize_to_file();
        grp_file_sha_to_file();

        return "Sharing Stopped for the file";
    }
    else
    {
        auto it = group_files_sleechers[group_name + " " + file_name].find(user);
        group_files_sleechers[group_name + " " + file_name].erase(it);

        grp_user_file_and_its_path.erase(group_name + " " + user + " " + file_name);

        if (grp_user_to_file[group_name + " " + user].size() == 1)
        {
            grp_user_to_file.erase(group_name + " " + user);
        }
        else
        {
            it = grp_user_to_file[group_name + " " + user].find(file_name);
            grp_user_to_file[group_name + " " + user].erase(it);
        }

        group_and_sleechers_to_file();
        user_file_path_to_file();
        group_user_file_to_file();
        return "Sharing Stopped for the file";
    }
}

vector<string> split_input_command(string input_command)
{
    string temp = "";
    vector<string> output;

    for (int i = 0; i < input_command.size(); i++)
    {
        if (isspace(input_command[i]))
        {
            if (temp != "")
            {
                output.push_back(temp);
                temp = "";
            }
        }
        else
        {
            temp += input_command[i];
        }
    }
    output.push_back(temp);
    return output;
}

string process_request(string message_from_peer)
{
    vector<string> splitted_cmd = split_input_command(message_from_peer);

    if (splitted_cmd[0] == "create_user")
    {
        if (splitted_cmd.size() != 3)
        {
            cout << "Invalid command";
        }
        else
        {
            if (user_pass.count(splitted_cmd[1]) > 0)
            {
                return "User already exist";
            }
            else
            {
                user_pass[splitted_cmd[1]] = splitted_cmd[2];
                struct_user_details new_user;
                new_user.insert(splitted_cmd[1], splitted_cmd[2]);
                fstream ud;
                ud.open(FILE_USER_DETAILS, ios::app);
                ud.write((char *)&new_user, sizeof(new_user));
                ud.close();
                return "User created successfully";
            }
        }
    }

    else if (splitted_cmd[0] == "login")
    {
        if (splitted_cmd.size() != 4)
        {
            cout << "Invalid command";
        }
        else
        {
            if (login_user_port.count(splitted_cmd[1]) > 0)
            {

                return "Already logged in";
            }
            else
            {
                if (user_pass[splitted_cmd[1]] == splitted_cmd[2])
                {
                    login_user_port[splitted_cmd[1]] = splitted_cmd[3];
                    return "Login successfully";
                }
                else
                {
                    return "Username Password not match";
                }
            }
        }
    }

    else if (splitted_cmd[0] == "create_group")
    {
        if (splitted_cmd.size() != 3)
        {
            cout << "Invalid command";
        }
        else
        {
            if (group_and_its_admin.count(splitted_cmd[1]) > 0)
            {

                return "Group ID already exist";
            }
            else
            {
                group_and_its_admin[splitted_cmd[1]] = splitted_cmd[2];
                struct_group_and_admin_details new_group;
                new_group.insert(splitted_cmd[1], splitted_cmd[2]);
                fstream ud;
                ud.open(FILE_GROUP_ADMIN, ios::app);
                ud.write((char *)&new_group, sizeof(new_group));
                ud.close();

                // group_and_its_users[splitted_cmd[1]].push_back(splitted_cmd[2]);
                group_and_its_users[splitted_cmd[1]].insert(splitted_cmd[2]);
                struct_group_and_members new_member;
                new_member.insert(splitted_cmd[1], splitted_cmd[2]);
                fstream gd;
                gd.open(FILE_GROUP_MEMBER, ios::app);
                gd.write((char *)&new_member, sizeof(new_member));
                gd.close();
                return "Group Created Succesfully";
            }
        }
    }

    else if (splitted_cmd[0] == "join_group")
    {
        if (splitted_cmd.size() != 3)
        {
            cout << "Invalid command";
        }
        else
        {
            if (group_and_its_users.count(splitted_cmd[1]) > 0)
            {

                // if (count(group_and_its_users[splitted_cmd[1]].begin(), group_and_its_users[splitted_cmd[1]].end(), splitted_cmd[2]))
                if (group_and_its_users[splitted_cmd[1]].find(splitted_cmd[2]) == group_and_its_users[splitted_cmd[1]].end())
                {

                    groups_pending_request[splitted_cmd[1]].insert(splitted_cmd[2]);
                    // group_and_its_users[splitted_cmd[1]].push_back(splitted_cmd[2]);
                    // group_and_its_users[splitted_cmd[1]].insert(splitted_cmd[2]);
                    // struct_group_and_members new_member;
                    // new_member.insert(splitted_cmd[1], splitted_cmd[2]);
                    // fstream ud;
                    // ud.open(FILE_GROUP_MEMBER, ios::app);
                    // ud.write((char *)&new_member, sizeof(new_member));
                    // ud.close();
                    struct_waiting_to_join wtj;
                    wtj.insert(splitted_cmd[1], splitted_cmd[2]);
                    fstream ud;
                    ud.open(FILE_PENDING_REQUEST, ios::app);
                    ud.write((char *)&wtj, sizeof(wtj));
                    ud.close();
                    return "Joining request sent";
                }
                else
                {
                    return "Already member of this group";
                }
            }
            else
            {
                return "Group ID does not exist";
            }
        }
    }

    else if (splitted_cmd[0] == "list_requests")
    {
        if (splitted_cmd.size() != 3)
        {
            cout << "Invalid command";
        }
        else
        {
            auto requested_group = group_and_its_admin.find(splitted_cmd[1]);

            if (requested_group == group_and_its_admin.end())
            {
                return "Group ID does not exist";
            }
            else if (strcmp((requested_group->second).c_str(), splitted_cmd[2].c_str()) == 0)
            {

                auto it = groups_pending_request.find(splitted_cmd[1]);
                if (it == groups_pending_request.end())
                {
                    return "No Pending Request for this group";
                }
                else
                {

                    string result = "Pending requests are :- ";
                    for (auto it : groups_pending_request[splitted_cmd[1]])
                    {
                        result += it + " ";
                    }
                    return result;
                }
            }
            else
            {
                return "Only admin can retrive this information";
            }
        }
    }

    else if (splitted_cmd[0] == "accept_request")
    {
        if (splitted_cmd.size() != 4)
        {
            cout << "Invalid command";
        }
        else
        {
            auto requested_group = group_and_its_admin.find(splitted_cmd[1]);

            if (requested_group == group_and_its_admin.end())
            {
                return "Group ID does not exist";
            }
            else
            {
                if (strcmp(group_and_its_admin[splitted_cmd[1]].c_str(), splitted_cmd[3].c_str()) == 0)
                {
                    auto it = groups_pending_request.find(splitted_cmd[1]);
                    if (it == groups_pending_request.end())
                    {
                        return "No Pending Request for this group";
                    }
                    else
                    {
                        auto search = groups_pending_request[splitted_cmd[1]].find(splitted_cmd[2]);
                        if (search == groups_pending_request[splitted_cmd[1]].end())
                        {
                            return "No request for this User ID";
                        }
                        else
                        {
                            groups_pending_request[splitted_cmd[1]].erase(search);
                            group_and_its_users[splitted_cmd[1]].insert(splitted_cmd[2]);
                            auto to_delete = groups_pending_request.find(splitted_cmd[1]);
                            if (groups_pending_request[splitted_cmd[1]].size() == 0)
                            {
                                groups_pending_request.erase(to_delete);
                            }
                            group_and_pending_request_to_file();
                            struct_group_and_members new_member;
                            new_member.insert(splitted_cmd[1], splitted_cmd[2]);
                            fstream gd;
                            gd.open(FILE_GROUP_MEMBER, ios::app);
                            gd.write((char *)&new_member, sizeof(new_member));
                            gd.close();
                            return "Request Accepted";
                        }
                    }
                }
                else
                {
                    return "Only Admin can accept requests";
                }
            }
        }
    }

    else if (splitted_cmd[0] == "list_groups")
    {
        if (splitted_cmd.size() != 1)
        {
            cout << "Invalid command";
        }
        else
        {
            if (group_and_its_admin.size() != 0)
            {
                string result = "Groups present are : ";
                for (auto it : group_and_its_admin)
                {
                    result += it.first + " ";
                }
                return result;
            }
            else
            {
                return "No group exists";
            }
        }
    }

    else if (splitted_cmd[0] == "list_files")
    {
        if (splitted_cmd.size() != 2)
        {
            cout << "Invalid command";
        }
        else
        {
            auto requested_group = group_and_its_users.find(splitted_cmd[1]);

            if (requested_group == group_and_its_users.end())
            {
                return "Group ID does not exist";
            }

            if (groups_and_files[splitted_cmd[1]].size() != 0)
            {
                string result = "Files present are : ";
                for (auto it : groups_and_files[splitted_cmd[1]])
                {
                    result += it + " ";
                }
                return result;
            }
            else
            {
                return "No files exists";
            }
        }
    }

    else if (splitted_cmd[0] == "leave_group")
    {
        if (splitted_cmd.size() != 3)
        {
            cout << "Invalid command";
        }
        else
        {
            auto requested_group = group_and_its_users.find(splitted_cmd[1]);

            if (requested_group == group_and_its_users.end())
            {
                return "Group ID does not exist";
            }
            else
            {
                auto is_member_present = group_and_its_users[splitted_cmd[1]].find(splitted_cmd[2]);
                if (is_member_present == group_and_its_users[splitted_cmd[1]].end())
                {
                    return "You are not member of this group";
                }
                else
                {
                    if (group_and_its_users[splitted_cmd[1]].size() == 1) // Only one member and its the admin
                    {

                        auto it = grp_user_to_file[splitted_cmd[1] + " " + splitted_cmd[2]];

                        for (auto a : it)
                        {
                            string s = stop_share_function(a, splitted_cmd[1], splitted_cmd[2]);
                        }

                        group_and_its_users.erase(splitted_cmd[1]);
                        group_and_its_admin.erase(splitted_cmd[1]);
                        groups_pending_request.erase(splitted_cmd[1]);
                        group_and_user_to_file();
                        group_and_admin_to_file();
                        group_and_pending_request_to_file();

                        // groups_and_files.erase(splitted_cmd[1]);
                        // group_and_files_to_file();
                        // int k = group_files_sleechers.size();
                        // int i = 0;
                        // for (auto it = group_files_sleechers.begin(); i < k && it != group_files_sleechers.end(); it++)
                        // {
                        //     i++;

                        //     if (group_files_sleechers.size() == 0)
                        //     {
                        //         break;
                        //     }
                        //     string s = it->first;
                        //     int found = s.find(' ');
                        //     string grp_name = s.substr(0, found);
                        //     if (strcmp(grp_name.c_str(), splitted_cmd[1].c_str()) == 0)
                        //     {
                        //         group_files_sleechers.erase(it);
                        //     }
                        // }
                        // group_and_sleechers_to_file();

                        // k = grp_user_file_and_its_path.size();
                        // i = 0;
                        // for (auto it = grp_user_file_and_its_path.begin(); i < k && it != grp_user_file_and_its_path.end(); it++)
                        // {
                        //     i++;
                        //     if (grp_user_file_and_its_path.size() == 0)
                        //     {
                        //         break;
                        //     }
                        //     string s = it->first;
                        //     int found = s.find(' ');
                        //     string grp_name = s.substr(0, found);
                        //     if (strcmp(grp_name.c_str(), splitted_cmd[1].c_str()) == 0)
                        //     {
                        //         grp_user_file_and_its_path.erase(it);
                        //     }
                        // }
                        // user_file_path_to_file();

                        // k = grp_user_to_file.size();
                        // i = 0;
                        // for (auto it = grp_user_to_file.begin(); i < k && it != grp_user_to_file.end(); it++)
                        // {
                        //     i++;
                        //     if (grp_user_to_file.size() == 0)
                        //     {
                        //         break;
                        //     }
                        //     string s = it->first;
                        //     int found = s.find(' ');
                        //     string grp_name = s.substr(0, found);
                        //     if (strcmp(grp_name.c_str(), splitted_cmd[1].c_str()) == 0)
                        //     {
                        //         grp_user_to_file.erase(it);
                        //     }
                        // }
                        // group_user_file_to_file();

                        // k = file_size.size();
                        // i = 0;
                        // for (auto it = file_size.begin(); i < k && it != file_size.end(); it++)
                        // {
                        //     i++;
                        //     if (file_size.size() == 0)
                        //     {
                        //         break;
                        //     }
                        //     string s = it->first;
                        //     int found = s.find(' ');
                        //     string grp_name = s.substr(0, found);
                        //     if (strcmp(grp_name.c_str(), splitted_cmd[1].c_str()) == 0)
                        //     {
                        //         file_size.erase(it);
                        //     }
                        // }
                        // group_filesize_to_file();

                        // k = file_SHA.size();
                        // i = 0;
                        // for (auto it = file_SHA.begin(); i < k && it != file_SHA.end(); it++)
                        // {
                        //     i++;
                        //     if (file_SHA.size() == 0)
                        //     {
                        //         break;
                        //     }
                        //     string s = it->first;
                        //     int found = s.find(' ');
                        //     string grp_name = s.substr(0, found);
                        //     if (strcmp(grp_name.c_str(), splitted_cmd[1].c_str()) == 0)
                        //     {
                        //         file_SHA.erase(it);
                        //     }
                        // }
                        // grp_file_sha_to_file();

                        return "Group Left";
                    }
                    else
                    {

                        auto it = grp_user_to_file[splitted_cmd[1] + " " + splitted_cmd[2]];

                        for (auto a : it)
                        {
                            cout << "Here" << endl;
                            string s = stop_share_function(a, splitted_cmd[1], splitted_cmd[2]);
                        }

                        if (group_and_its_admin[splitted_cmd[1]] == splitted_cmd[2])
                        {
                            group_and_its_users[splitted_cmd[1]].erase(is_member_present);
                            auto it = group_and_its_users[splitted_cmd[1]].begin();
                            int random = rand() % group_and_its_users[splitted_cmd[1]].size();
                            advance(it, random);
                            group_and_its_admin[splitted_cmd[1]] = *it;
                            group_and_user_to_file();
                            group_and_admin_to_file();
                        }
                        else
                        {
                            group_and_its_users[splitted_cmd[1]].erase(is_member_present);
                            group_and_user_to_file();
                        }

                        return "Group Left";
                    }
                }
            }
        }
    }

    else if (splitted_cmd[0] == "upload_file")
    {
        if (splitted_cmd.size() != 6)
        {
            cout << "Invalid command";
        }
        else
        {
            int found = splitted_cmd[1].find_last_of('/');
            string file_name = splitted_cmd[1].substr(found + 1, splitted_cmd[1].size() - found);
            string group_name = splitted_cmd[2];
            string uploader_name = splitted_cmd[3];
            string grp_plus_file = group_name + " " + file_name;

            if (group_and_its_users.find(group_name) == group_and_its_users.end())
            {
                return "Group does not exist";
            }

            if (group_and_its_users[group_name].find(uploader_name) == group_and_its_users[group_name].end())
            {
                return "You are not member, only members can upload";
            }

            if (group_files_sleechers.find(grp_plus_file) == group_files_sleechers.end()) // GROUP AND FILENAME DOES NOT MATCH
            {
                file_size[group_name + " " + file_name] = stoll(splitted_cmd[4]);
                file_SHA[group_name + " " + file_name] = splitted_cmd[5];

                struct_grp_filename_to_size size;
                size.insert(group_name + " " + file_name, splitted_cmd[4]);
                fstream siz;
                siz.open(FILE_GRP_FILE_SIZE, ios::app);
                siz.write((char *)&size, sizeof(size));
                siz.close();

                struct_grp_filename_to_SHA sha;
                sha.insert(group_name + " " + file_name, splitted_cmd[5]);
                fstream si;
                si.open(FILE_GRP_FILE_SHA, ios::app);
                si.write((char *)&sha, sizeof(sha));
                si.close();

                group_files_sleechers[grp_plus_file].insert(uploader_name);

                struct_group_file_sleechers new_seeder;
                new_seeder.insert(grp_plus_file, uploader_name);
                fstream ud;
                ud.open(FILE_GROUP_FILE_SEEDER, ios::app);
                ud.write((char *)&new_seeder, sizeof(new_seeder));
                ud.close();

                groups_and_files[group_name].insert(file_name);
                struct_group_files new_file;
                new_file.insert(group_name, file_name);
                fstream nf;
                nf.open(FILE_GROUP_FILES, ios::app);
                nf.write((char *)&new_file, sizeof(new_file));
                nf.close();

                grp_user_file_and_its_path[group_name + " " + uploader_name + " " + file_name] = splitted_cmd[1];

                struct_grp_user_file_paths path;
                path.insert(group_name + " " + uploader_name + " " + file_name, splitted_cmd[1]);
                fstream pa;
                pa.open(FILE_FILENAME_ITS_PATH, ios::app);
                pa.write((char *)&path, sizeof(path));
                pa.close();

                grp_user_to_file[group_name + " " + uploader_name].insert(file_name);

                struct_grp_user_to_file file;
                file.insert(group_name + " " + uploader_name, file_name);
                fstream fi;
                fi.open(FILE_GRP_USR_FILE, ios::app);
                fi.write((char *)&file, sizeof(file));
                fi.close();

                return "File uploaded successfully";
            }
            else
            {

                if (group_files_sleechers[grp_plus_file].find(uploader_name) == group_files_sleechers[grp_plus_file].end()) // GROUP AND FILENAME MATCH BUT NOT SEEDER
                {

                    file_size[group_name + " " + file_name] = stoll(splitted_cmd[4]);
                    file_SHA[group_name + " " + file_name] = splitted_cmd[5];

                    struct_grp_filename_to_size size;
                    size.insert(group_name + " " + file_name, splitted_cmd[4]);
                    fstream siz;
                    siz.open(FILE_GRP_FILE_SIZE, ios::app);
                    siz.write((char *)&size, sizeof(size));
                    siz.close();

                    struct_grp_filename_to_SHA sha;
                    sha.insert(group_name + " " + file_name, splitted_cmd[5]);
                    fstream si;
                    si.open(FILE_GRP_FILE_SHA, ios::app);
                    si.write((char *)&sha, sizeof(sha));
                    si.close();

                    group_files_sleechers[grp_plus_file].insert(uploader_name);
                    struct_group_file_sleechers new_seeder;
                    new_seeder.insert(grp_plus_file, uploader_name);
                    fstream ud;
                    ud.open(FILE_GROUP_FILE_SEEDER, ios::app);
                    ud.write((char *)&new_seeder, sizeof(new_seeder));
                    ud.close();

                    groups_and_files[group_name].insert(file_name);
                    struct_group_files new_file;
                    new_file.insert(group_name, file_name);
                    fstream nf;
                    nf.open(FILE_GROUP_FILES, ios::app);
                    nf.write((char *)&new_file, sizeof(new_file));
                    nf.close();

                    grp_user_file_and_its_path[group_name + " " + uploader_name + " " + file_name] = splitted_cmd[1];

                    struct_grp_user_file_paths path;
                    path.insert(group_name + " " + uploader_name + " " + file_name, splitted_cmd[1]);
                    fstream pa;
                    pa.open(FILE_FILENAME_ITS_PATH, ios::app);
                    pa.write((char *)&path, sizeof(path));
                    pa.close();

                    grp_user_to_file[group_name + " " + uploader_name].insert(file_name);

                    struct_grp_user_to_file file;
                    file.insert(group_name + " " + uploader_name, file_name);
                    fstream fi;
                    fi.open(FILE_GRP_USR_FILE, ios::app);
                    fi.write((char *)&file, sizeof(file));
                    fi.close();

                    return "File uploaded successfully";
                }
                else
                {
                    return "You are already a seeder in this group";
                }
            }
        }
    }

    else if (splitted_cmd[0] == "download_file")
    {
        if (splitted_cmd.size() != 4)
        {
            cout << "Invalid command";
        }
        else
        {
            string group_name = splitted_cmd[1];
            string file_name = splitted_cmd[2];
            string downloader_name = splitted_cmd[3];

            string grp_plus_file = group_name + " " + file_name;

            if (group_and_its_users.find(group_name) == group_and_its_users.end())
            {
                return "Group does not exist";
            }

            if (group_and_its_users[group_name].find(downloader_name) == group_and_its_users[group_name].end())
            {
                return "You are not member, only members can download";
            }

            if (group_files_sleechers.find(grp_plus_file) == group_files_sleechers.end())
            {
                return "No seeder exists for this file";
            }
            else
            {
                auto any = group_files_sleechers[grp_plus_file];
                string output = "";
                int flag = 0;

                for (auto it : any)
                {
                    if (login_user_port.find(it) == login_user_port.end())
                    {
                        continue;
                    }
                    else
                    {
                        flag = 1;
                        output += " " + login_user_port[it] + " " + grp_user_file_and_its_path[group_name + " " + it + " " + file_name] + " " + to_string(file_size[group_name + " " + file_name]) + " " + file_SHA[group_name + " " + file_name];
                    }
                }
                if (flag == 0)
                {
                    return "No seeders online.";
                }
                else
                {
                    return output;
                }
            }
        }
    }

    else if (splitted_cmd[0] == "stop_share")
    {
        if (splitted_cmd.size() != 4)
        {
            cout << "Invalid command";
        }
        else
        {
            return stop_share_function(splitted_cmd[2], splitted_cmd[1], splitted_cmd[3]);
        }
    }

    else if (splitted_cmd[0] == "logout")
    {
        login_user_port.erase(splitted_cmd[1]);
        return "Logout Successfully";
    }

    return "Invalid Command";
}

void receive_request_from_peer(int peer_socket, struct sockaddr_in peer_details)
{
    if (peer_socket < 0)
    {
        cout << "[-] Not connected to client" << endl;
        return;
    }
    else
    {
        char temp[BLOCK_SIZE];
        bzero(temp, sizeof(temp));
        read(peer_socket, temp, sizeof(temp));
        int port_of_peer = ntohs(peer_details.sin_port);
        int ip_of_peer = peer_details.sin_addr.s_addr;
        string message_from_peer(temp);

        cout << "[+] Got Request : " << message_from_peer << endl;
        string response_for_peer = process_request(message_from_peer);

        write(peer_socket, response_for_peer.c_str(), response_for_peer.size());
        close(peer_socket);
    }
}

void save_details_to_file()
{
}

void fill_up_data_structures()
{

    // cout << "-------------------------USER DETAILS------------------------------" << endl;
    ifstream ud(FILE_USER_DETAILS, ios::ate | ios::binary);

    if (ud.good())
    {
        long long length = ud.tellg();
        ud.close();
        if (length > 0)
        {
            ud.open(FILE_USER_DETAILS, ios::in);
            struct_user_details prev_user;
            while (!ud.eof())
            {
                ud.read((char *)&prev_user, sizeof(prev_user));
                string name(prev_user.user);
                string pass(prev_user.password);
                user_pass[name] = pass;
            }
            ud.close();
        }
    }

    // for (auto it : user_pass)
    // {
    //     cout << it.first << "--->" << it.second << endl;
    // }

    // cout << "------------------------GROUP ADMIN-------------------" << endl;

    ifstream gd(FILE_GROUP_ADMIN, ios::ate | ios::binary);
    if (gd.good())
    {
        long long length = gd.tellg();
        gd.close();
        if (length > 0)
        {
            gd.open(FILE_GROUP_ADMIN, ios::in);
            struct_group_and_admin_details admins;
            while (!gd.eof())
            {
                gd.read((char *)&admins, sizeof(admins));
                string name(admins.group);
                string pass(admins.admin);
                group_and_its_admin[name] = pass;
            }
            gd.close();
        }
    }

    // for (auto it : group_and_its_admin)
    // {
    //     cout << it.first << "--->" << it.second << endl;
    // }

    // cout << "------------------------GROUP MEMBERS-----------------------" << endl;
    ifstream gm(FILE_GROUP_MEMBER, ios::ate | ios::binary);
    if (gm.good())
    {
        long long length = gm.tellg();
        gm.close();
        if (length > 0)
        {
            gm.open(FILE_GROUP_MEMBER, ios::in);
            struct_group_and_members member;
            while (!gm.eof())
            {
                gm.read((char *)&member, sizeof(member));
                string name(member.group);
                string mem(member.member);
                // group_and_its_users[name].push_back(mem);
                group_and_its_users[name].insert(mem);
            }
            gm.close();
        }
    }

    // for (auto it : group_and_its_users)
    // {
    //     cout << it.first << "--->";
    //     for (auto a : it.second)
    //     {
    //         cout << a << " ";
    //     }
    //     cout << endl;
    // }

    // cout << "------------------------GROUPS PENDING REQUEST-----------------------" << endl;
    ifstream pr(FILE_PENDING_REQUEST, ios::ate | ios::binary);
    if (pr.good())
    {
        long long length = pr.tellg();
        pr.close();
        if (length > 0)
        {

            pr.open(FILE_PENDING_REQUEST, ios::in);
            struct_waiting_to_join member;
            while (!pr.eof())
            {
                pr.read((char *)&member, sizeof(member));
                string name(member.group);
                string mem(member.member);
                // group_and_its_users[name].push_back(mem);
                groups_pending_request[name].insert(mem);
            }
            pr.close();
        }
    }

    // for (auto it : groups_pending_request)
    // {
    //     cout << it.first << "--->";
    //     for (auto a : it.second)
    //     {
    //         cout << a << " ";
    //     }
    //     cout << endl;
    // }

    // cout << "------------------------GROUP FILE SEEDERS REQUEST-----------------------" << endl;

    ifstream a(FILE_GROUP_FILE_SEEDER, ios::ate | ios::binary);
    if (a.good())
    {
        long long length = a.tellg();
        a.close();
        if (length > 0)
        {

            a.open(FILE_GROUP_FILE_SEEDER, ios::in);
            struct_group_file_sleechers adv;
            while (!a.eof())
            {
                a.read((char *)&adv, sizeof(adv));
                string grp_file(adv.group_file);
                string seed(adv.seeders);
                group_files_sleechers[grp_file].insert(seed);
            }
            a.close();
        }
    }

    // for (auto it : group_files_sleechers)
    // {
    //     cout << it.first << "--->";
    //     for (auto a : it.second)
    //     {
    //         cout << a << " ";
    //     }
    //     cout << endl;
    // }

    // cout << "------------------------GROUP AND ITS FILES-----------------------" << endl;
    ifstream b(FILE_GROUP_FILES, ios::ate | ios::binary);
    if (b.good())
    {
        long long length = b.tellg();
        b.close();
        if (length > 0)
        {

            b.open(FILE_GROUP_FILES, ios::in);
            struct_group_files adv;
            while (!b.eof())
            {
                b.read((char *)&adv, sizeof(adv));
                string grp(adv.group);
                string file(adv.files);
                groups_and_files[grp].insert(file);
            }
            b.close();
        }
    }

    // for (auto it : groups_and_files)
    // {
    //     cout << it.first << "--->";
    //     for (auto a : it.second)
    //     {
    //         cout << a << " ";
    //     }
    //     cout << endl;
    // }

    // cout << "------------------------GROUP USER FILE PATH-------------------" << endl;
    ifstream up(FILE_FILENAME_ITS_PATH, ios::ate | ios::binary);
    if (up.good())
    {
        long long length = up.tellg();
        up.close();
        if (length > 0)
        {

            up.open(FILE_FILENAME_ITS_PATH, ios::in);
            struct_grp_user_file_paths path;
            while (!up.eof())
            {
                up.read((char *)&path, sizeof(path));
                string name(path.file);
                string pass(path.path);
                grp_user_file_and_its_path[name] = pass;
            }
            up.close();
        }
    }

    // for (auto it : grp_user_file_and_its_path)
    // {
    //     cout << it.first << "--->" << it.second << endl;
    // }

    // cout << "------------------------GROUP USER to FILE-------------------" << endl;
    ifstream uf(FILE_GRP_USR_FILE, ios::ate | ios::binary);
    if (uf.good())
    {
        long long length = uf.tellg();
        uf.close();
        if (length > 0)
        {

            uf.open(FILE_GRP_USR_FILE, ios::in);
            struct_grp_user_to_file file;
            while (!uf.eof())
            {
                uf.read((char *)&file, sizeof(file));
                string usr(file.user);
                string fil(file.file);
                grp_user_to_file[usr].insert(fil);
            }
            uf.close();
        }
    }

    // for (auto it : grp_user_to_file)
    // {
    //     cout << it.first << "--->";
    //     for (auto a : it.second)
    //     {
    //         cout << a << " ";
    //     }
    //     cout << endl;
    // }

    // cout << "-------------------------FILENAME TO SIZE------------------------------" << endl;
    ifstream us(FILE_GRP_FILE_SIZE, ios::ate | ios::binary);

    if (us.good())
    {
        long long length = us.tellg();
        us.close();
        if (length > 0)
        {

            us.open(FILE_GRP_FILE_SIZE, ios::in);
            struct_grp_filename_to_size size;
            while (!us.eof())
            {
                us.read((char *)&size, sizeof(size));
                string name(size.file);
                string pass(size.size);
                file_size[name] = stoll(pass);
            }
            us.close();
        }
    }

    // for (auto it : file_size)
    // {
    //     cout << it.first << "--->" << it.second << endl;
    // }

    // cout << "-------------------------FILENAME TO SHA------------------------------" << endl;
    ifstream sh(FILE_GRP_FILE_SHA, ios::ate | ios::binary);

    if (sh.good())
    {
        long long length = sh.tellg();
        sh.close();
        if (length > 0)
        {

            sh.open(FILE_GRP_FILE_SHA, ios::in);
            struct_grp_filename_to_SHA sha;
            while (!sh.eof())
            {
                sh.read((char *)&sha, sizeof(sha));
                string name(sha.file);
                string pass(sha.sha);
                file_SHA[name] = pass;
            }
            sh.close();
        }
    }

    // for (auto it : file_SHA)
    // {
    //     cout << it.first << "--->" << it.second << endl;
    // }

    return;
}

void tracker_input()
{

    while (1)
    {
        string input_command;
        getline(cin, input_command, '\n');

        if (input_command == "quit")
        {

            save_details_to_file();
            exit(1);
        }
        else
        {
            cout << "[-] Invalid Command" << endl;
        }
    }
}

void extract_ip_port(string path)
{
    vector<string> iport;
    fstream adv;
    adv.open(path, ios::in);
    string read = "";
    while (getline(adv, read, ' '))
    {
        iport.push_back(read);
        read = "";
    }
    int found = iport[0].find_last_of(':');
    IP_of_TRACKER = iport[0].substr(0, found);
    PORT_of_TRACKER = stoi(iport[0].substr(found + 1, iport[0].size() - found));
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        cout << "[-] Invalid number of arguments" << endl;
        exit(1);
    }
    else
    {
        int tracker_number = stoi(argv[2]);
        string info_path = argv[1];

        extract_ip_port(info_path);

        // cout << "IP of TRACKER :" << IP_of_TRACKER << endl;
        // cout << "PORT of TRACKER :" << PORT_of_TRACKER << endl;

        int tracker_socket;
        tracker_socket = socket(AF_INET, SOCK_STREAM, 0);

        if (tracker_socket < 0)
        {
            cout << "[-] Error occured in opening socket" << endl;
        }
        struct sockaddr_in tracker_details;
        tracker_details.sin_family = AF_INET;
        tracker_details.sin_port = htons(PORT_of_TRACKER);
        tracker_details.sin_addr.s_addr = INADDR_ANY;

        int b = bind(tracker_socket, (struct sockaddr *)&tracker_details, sizeof(tracker_details));
        if (b < 0)
        {
            cout << "[-] Error in binding" << endl;
            return 0;
        }

        thread separating_tracker_to_read_input(tracker_input);
        separating_tracker_to_read_input.detach();

        fill_up_data_structures();
        cout << "[+] Tracker listening..." << endl;
        listen(tracker_socket, 5);

        struct sockaddr_in peer_details;
        while (true)
        {
            int peer_address_length = sizeof(peer_details);
            int peer_socket;
            peer_socket = accept(tracker_socket, (struct sockaddr *)&peer_details, (socklen_t *)&peer_address_length); // accept return value of a client socket
            // char temp[256];
            // recv(tracker_socket, &temp, sizeof(temp), 0);
            // string peers_request(temp);
            // cout<<peers_request<<endl;
            thread a(receive_request_from_peer, peer_socket, peer_details);
            a.detach();
        }
    }

    return 0;
}