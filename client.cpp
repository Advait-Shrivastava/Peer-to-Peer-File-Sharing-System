#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <dirent.h>
#include <fcntl.h>
#include <thread>
#include <pthread.h>
#include <bits/stdc++.h>
#include <string.h>
#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <cmath>
#define BLOCK_SIZE 1024
#define CHUNK_SIZE 524288
#define UPLOADER "./data/SHA_uploaders/"
#define THREAD_NUM 1
using namespace std;

string IP_of_PEER;
int PORT_of_PEER;
string IP_of_TRACKER;
int PORT_of_TRACKER;
string last_login;
bool current_online = false;
// long long int count_of_chunk_downloaded = 0;

unordered_map<string, vector<string>> chunks_present; // grp + filename --->> chunks
unordered_map<string, bool> complete_file;            // grp + filename ----> True/False

unordered_map<string, vector<string>> download_done;    // grp -->filesdownloaded
unordered_map<string, vector<string>> download_ongoing; // grp -->filesdownloaded

string chunk_download_request(string destination_file, int chunk_number, vector<string> sleechers, unordered_map<string, string> peer_path, int *count_of_chunk_downloaded, string grp, string filesize, string correct_SHA);

bool sort_by_second(const pair<long long int, long long int> &a, const pair<long long int, long long int> &b)
{
    return (a.second < b.second);
}

typedef struct Task
{
    string destination_loc;
    int chunk_number;
    vector<string> peer;
    unordered_map<string, string> src_path;
    int *count_of_chunk_downloaded;
    string grp;
    string filesize;
    string correct_SHA;

} Task;

Task taskQueue[10000];
int taskCount = 0;

pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

pthread_mutex_t chunk_count_lock;

void executeTask(Task *task)
{
    usleep(1000);
    string s = chunk_download_request(task->destination_loc, task->chunk_number, task->peer, task->src_path, task->count_of_chunk_downloaded, task->grp, task->filesize, task->correct_SHA);
    // pthread_exit(NULL);
}

void submitTask(Task task)
{
    pthread_mutex_lock(&mutexQueue);
    taskQueue[taskCount] = task;
    taskCount++;
    pthread_mutex_unlock(&mutexQueue);
    pthread_cond_signal(&condQueue);
}

void *startThread(void *args)
{
    while (1)
    {
        Task task;

        pthread_mutex_lock(&mutexQueue);
        while (taskCount == 0)
        {
            pthread_cond_wait(&condQueue, &mutexQueue);
        }

        task = taskQueue[0];
        int i;
        for (i = 0; i < taskCount - 1; i++)
        {
            taskQueue[i] = taskQueue[i + 1];
        }
        taskCount--;
        pthread_mutex_unlock(&mutexQueue);
        // cout<<"Start Thread"<<endl;d
        executeTask(&task);
    }
}

struct struct_chunk_details
{
    char name[50];
    char chunks[30000];
    void insert(string n, string c)
    {
        strcpy(name, n.c_str());
        strcpy(chunks, c.c_str());
        return;
    }
};

void chunks_present_to_file()
{
    fstream ga;
    ga.open("./data/Users_Upload/" + last_login + ".txt", ios::out | ios::trunc);
    for (auto it : chunks_present)
    {
        string str = "";
        for (auto a : it.second)
        {
            str += a;
        }
        struct_chunk_details chunk;
        chunk.insert(it.first, str);
        ga.write((char *)&chunk, sizeof(chunk));
    }
    ga.close();
}

void fill_up_data_structures()
{
    ifstream ud("./data/Users_Upload/" + last_login + ".txt");
    if (ud.good())
    {
        ud.close();
        ud.open("./data/Users_Upload/" + last_login + ".txt", ios::in | ios::binary);
        struct_chunk_details chunks;
        while (!ud.eof())
        {
            ud.read((char *)&chunks, sizeof(chunks));
            string name(chunks.name);
            string pass(chunks.chunks);

            vector<string> temp;

            for (auto it : pass)
            {
                string s = "";
                s += it;
                temp.push_back(s);
            }

            chunks_present[name] = temp;
        }

        // for (auto it : chunks_present)
        // {
        //     cout << it.first << "-->";
        //     for (auto a : it.second)
        //     {
        //         cout << a;
        //     }
        //     cout << endl;
        // }
        ud.close();
    }
}

unordered_map<string, vector<string>> file_and_SHA; // grp+file --> SHA(SHA_of_file then SHA of first chunk them SHA of second chunk etc)

vector<string> split_input_command(string input_command) // Funtion to split the sentence entered by user into tokens
{
    string temp = "";
    vector<string> output;

    for (int i = 0; i < input_command.size(); i++)
    {
        if (isspace(input_command[i])) // splitting the string whenever a blankspace is found
        {
            if (temp != "") // ignoring the extra entered space between the two tokens
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

string connection_request_to_tracker(string message_for_tracker)
{
    int peer_client;
    peer_client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in tracker_address;
    tracker_address.sin_family = AF_INET;
    tracker_address.sin_port = htons(PORT_of_TRACKER);
    tracker_address.sin_addr.s_addr = INADDR_ANY;
    int tracker_status = connect(peer_client, (struct sockaddr *)&tracker_address, sizeof(tracker_address));
    if (tracker_status == -1)
    {
        cout << "[-] Error in making connection" << endl;
        return "";
    }

    write(peer_client, message_for_tracker.c_str(), message_for_tracker.size());

    char tracker_response[BLOCK_SIZE];
    bzero(tracker_response, sizeof(tracker_response));
    read(peer_client, tracker_response, sizeof(tracker_response)); // last param is flag
    string s(tracker_response);

    return s;
}

string SHA_of_chunk(char s[], int total)
{
    unsigned char c[SHA_DIGEST_LENGTH];
    unsigned char chunk[total];

    SHA_CTX shaContext;
    SHA1_Init(&shaContext);
    SHA1_Update(&shaContext, s, total);
    SHA1_Final(chunk, &shaContext);
    stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)chunk[i];
    }
    return ss.str();
}

void accept_downloading_request(int peer_socket, struct sockaddr_in peer_details)
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
        int port_of_peers_client = ntohs(peer_details.sin_port);
        int ip_of_peer_clien = peer_details.sin_addr.s_addr;
        string message_from_peers_client(temp);
        string filepath = message_from_peers_client;

        vector<string> splitted_cmd = split_input_command(message_from_peers_client);
        cout << "[+] Got Request from peer : " << message_from_peers_client << endl;

        if (strcmp(splitted_cmd[0].c_str(), "INFO") == 0)
        {
            auto it = chunks_present[splitted_cmd[1] + " " + splitted_cmd[2]];
            cout << splitted_cmd[1] + " " + splitted_cmd[2] << endl;
            string reply = "";

            for (auto a : it)
            {
                reply += a;
            }
            write(peer_socket, reply.c_str(), reply.size());
            close(peer_socket);
            return;
        }

        else if (strcmp(splitted_cmd[0].c_str(), "CHUNK") == 0)
        {
            long long chunk_number = stoll(splitted_cmd[1]);
            string path = splitted_cmd[2];

            int src = open(path.c_str(), O_RDWR, 0777);

            long long int data_read;
            char temp[CHUNK_SIZE];
            bzero(temp, sizeof(temp));
            data_read = pread(src, temp, CHUNK_SIZE, chunk_number * CHUNK_SIZE);

            cout << "Path is :" << path << endl;
            cout << "Chunk is :" << chunk_number << endl;
            cout << "PEER SERVER SIDE : " << data_read << endl;

            if (data_read < 0)
            {
                return;
            }

            write(peer_socket, temp, data_read);
            // int a = 1;
            // setsockopt(peer_socket,SOL_SOCKET,SO_REUSEADDR,&a,sizeof(int));
            close(peer_socket);
            close(src);
            return;
        }

        else
        {

            int requested_file = open(filepath.c_str(), 0);
            if (requested_file < 0)
            {
                cout << "Some Error occured";
            }

            int count = 0;

            while (true)
            {
                char temp1[CHUNK_SIZE];
                int amount = read(requested_file, temp1, CHUNK_SIZE);

                if (amount >= 1)
                {
                    write(peer_socket, temp1, amount);
                }
                else if (amount == 0)
                {
                    cout << "File sent from peers server" << endl;
                    break;
                }
                else
                {
                    cout << "Error in reading file" << endl;
                    break;
                }
            }
            close(peer_socket);
            return;
        }
    }
}

void request_peer_to_download(string details, string destination_path, string filename)
{
    vector<string> splitted_cmd = split_input_command(details);
    int found = splitted_cmd[0].find_last_of(':');
    string IP_of_PEER_SERVER = splitted_cmd[0].substr(0, found);
    uint16_t PORT_of_PEER_SERVER = stoi(splitted_cmd[0].substr(found + 1, splitted_cmd[0].size() - found));

    string file_path = splitted_cmd[1];
    long long file_size = stoll(splitted_cmd[2]);

    int peer_client;

    peer_client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in peers_server_address;
    peers_server_address.sin_family = AF_INET;
    peers_server_address.sin_port = htons(PORT_of_PEER_SERVER);
    peers_server_address.sin_addr.s_addr = INADDR_ANY;
    int tracker_status = connect(peer_client, (struct sockaddr *)&peers_server_address, sizeof(peers_server_address));
    if (tracker_status == -1)
    {
        cout << "[-] Error in making connection to peer's server" << endl;
    }

    write(peer_client, file_path.c_str(), file_path.size());

    string dest = destination_path + "/" + filename;
    int result = open(dest.c_str(), O_CREAT | O_RDWR, 0777);

    if (result == -1)
    {
        cout << "Error in creating file at destination";
    }

    long long int data;
    char peers_server_response[CHUNK_SIZE];
    while ((data = read(peer_client, peers_server_response, CHUNK_SIZE)) > 0)
    {
        fflush(stdout);
        write(result, peers_server_response, data);
    }

    if (data != 0)
    {
        cout << "Error in downloading" << endl;
    }
    else
    {
        cout << "FILE DOWNLOADDED SUCCESSFULLY" << endl;
    }
}

int global = 0;

string chunk_download_request(string destination_file, int chunk_number, vector<string> sleechers, unordered_map<string, string> all_paths, int *count_of_chunk_downloaded, string grp, string filesize, string correct_SHA)
{
    int times = 0;
    int flag = 0;

    while (times < 10)
    {

        int random = rand() % sleechers.size();
        string details = sleechers[random];

        string peer_path = all_paths[details];
        int found = details.find_last_of(':');
        string IP_of_PEER_SERVER = details.substr(0, found);
        uint16_t PORT_of_PEER_SERVER = stoi(details.substr(found + 1, details.size() - found));

        int peer_client;

        peer_client = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in peers_server_address;
        peers_server_address.sin_family = AF_INET;
        peers_server_address.sin_port = htons(PORT_of_PEER_SERVER);
        peers_server_address.sin_addr.s_addr = INADDR_ANY;
        int tracker_status = connect(peer_client, (struct sockaddr *)&peers_server_address, sizeof(peers_server_address));

        if (tracker_status == -1)
        {
            cout << "[-] Error in making connection to peer's servers" << endl;
            // return "Error";
            continue;
        }

        string msg = "CHUNK " + to_string(chunk_number) + " " + peer_path;
        write(peer_client, msg.c_str(), msg.size());

        int result = open(destination_file.c_str(), O_RDWR, 0777);

        long long int data;
        char peers_server_response[CHUNK_SIZE];
        bzero(peers_server_response, sizeof(peers_server_response));
        // while ((data = read(peer_client, peers_server_response, sizeof(peers_server_response))) > 0)
        // {
        //     cout<<"Number of data read is : "<<data<<endl;
        //     fflush(stdout);
        //     // write(result, peers_server_response, data);
        //
        // }

        long long int offset = chunk_number * CHUNK_SIZE;
        char final[CHUNK_SIZE];
        long long int total = 0;

        int n;
        while (true)
        {
            // cout<<"Inside while"<<endl;
            bzero(peers_server_response, sizeof(peers_server_response));
            n = recv(peer_client, peers_server_response, CHUNK_SIZE, 0);
            if (n == 0)
            {
                break;
            }
            else
            {
                for (int i = 0; i < n; i++)
                {
                    final[i + total] = peers_server_response[i];
                }
                total += n;
            }
        }
        // cout<<"Outside While"<<endl;
        string sha = SHA_of_chunk(final, total);
        pwrite(result, final, total, offset);
        // int a = 1;
        // setsockopt(peer_client,SOL_SOCKET,SO_REUSEADDR,&a,sizeof(int));
        close(peer_client);
        close(result);

        if (n < 0)
        {
            continue;
            // return "Error";
        }
        else
        {
            flag = 1;
            break;
        }
    }

    if (flag == 1)
    {
        int found = destination_file.find_last_of('/');
        string file_name = destination_file.substr(found + 1, destination_file.size() - found);
        pthread_mutex_lock(&chunk_count_lock);
        *count_of_chunk_downloaded = *count_of_chunk_downloaded + 1;
        chunks_present[grp + " " + file_name].at(chunk_number) = "1";

        if (*count_of_chunk_downloaded == 1)
        {
            string message_for_tracker = "upload_file " + destination_file + " " + grp + " " + last_login + " " + filesize + " " + correct_SHA;
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            // cout << "[+] " << reply_from_tracker << endl;
        }
        pthread_mutex_unlock(&chunk_count_lock);
    }
    else
    {
        return "Error in download";
    }

    return "Done";
}

string calculate_SHA(string filename, vector<string> &chunks_SHA)
{
    unsigned char c[SHA_DIGEST_LENGTH];
    FILE *inFile = fopen(filename.c_str(), "rb");
    SHA_CTX shaContext;
    SHA_CTX shaContext1;
    int bytes;
    unsigned char data[CHUNK_SIZE];

    if (inFile == NULL)
    {
        cout << "Cannot open file" << endl;
        return "";
    }
    SHA1_Init(&shaContext);
    while ((bytes = fread(data, 1, CHUNK_SIZE, inFile)) != 0)
    {
        SHA1_Update(&shaContext, data, bytes);

        unsigned char chunk[SHA_DIGEST_LENGTH];
        SHA1_Init(&shaContext1);
        SHA1_Update(&shaContext1, data, bytes);
        SHA1_Final(chunk, &shaContext1);
        stringstream ss;
        string s = "";
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        {
            ss << hex << setw(2) << setfill('0') << (int)chunk[i];
        }
        chunks_SHA.push_back(ss.str());
    }
    SHA1_Final(c, &shaContext);
    string SHA_of_file = "";
    stringstream ff;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        ff << hex << setw(2) << setfill('0') << (int)c[i];
    }
    SHA_of_file = ff.str();
    fclose(inFile);
    return SHA_of_file;
}

string calculate_File_SHA(string filename)
{
    unsigned char c[SHA_DIGEST_LENGTH];
    FILE *inFile = fopen(filename.c_str(), "rb");
    SHA_CTX shaContext;
    int bytes;
    unsigned char data[CHUNK_SIZE];

    if (inFile == NULL)
    {
        cout << "Cannot open file" << endl;
        return "";
    }
    SHA1_Init(&shaContext);
    while ((bytes = fread(data, 1, CHUNK_SIZE, inFile)) != 0)
    {
        SHA1_Update(&shaContext, data, bytes);

    }
    SHA1_Final(c, &shaContext);
    string SHA_of_file = "";
    stringstream ff;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
    {
        ff << hex << setw(2) << setfill('0') << (int)c[i];
    }
    SHA_of_file = ff.str();
    fclose(inFile);
    return SHA_of_file;
}

string request_peer_file_info(vector<string> parameters)
{
    string ip_and_port = parameters[0];

    int found = ip_and_port.find_last_of(':');
    string ip_of_sleecher = ip_and_port.substr(0, found);
    int port_of_sleecher = stoi(ip_and_port.substr(found + 1, ip_and_port.size() - found));

    int peer_client;

    peer_client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in peers_server_address;
    peers_server_address.sin_family = AF_INET;
    peers_server_address.sin_port = htons(port_of_sleecher);
    peers_server_address.sin_addr.s_addr = INADDR_ANY;
    int tracker_status = connect(peer_client, (struct sockaddr *)&peers_server_address, sizeof(peers_server_address));
    if (tracker_status == -1)
    {
        cout << "[-] Error in making connection to peer's server" << endl;
        return "";
    }

    string msg = "INFO ";
    for (int i = 1; i < parameters.size(); i++)
    {
        msg += parameters[i] + " ";
    }

    write(peer_client, msg.c_str(), msg.size());

    char peer_server_response[CHUNK_SIZE];
    bzero(peer_server_response, sizeof(peer_server_response));
    read(peer_client, peer_server_response, sizeof(peer_server_response)); // last param is flag
    string s(peer_server_response);
    return s;
}

void make_hashtable_of_chunks(string ip_and_port, string chunks, unordered_map<int, vector<string>> &hash)
{
    for (int i = 0; i < chunks.size(); i++)
    {
        if (chunks[i] == '1')
        {
            hash[i].push_back(ip_and_port);
        }
    }
    return;
}

void download_in_another_thread(unordered_map<int, vector<string>> chunk_to_sleechers, string desti_path, string name, long long int file_size_of_downloading_file, unordered_map<string, string> port_to_location, string grpname, string correct_SHA)
{
    vector<pair<long long int, long long int>> no_of_peers;
    for (auto it : chunk_to_sleechers)
    {
        pair<int, int> pr;
        pr.first = it.first;
        pr.second = it.second.size();
        no_of_peers.push_back(pr);
    }
    sort(no_of_peers.begin(), no_of_peers.end(), sort_by_second);

    unordered_map<int, vector<string>> piece_selection;

    for (auto it : no_of_peers)
    {
        piece_selection[it.first] = chunk_to_sleechers[it.first];
    }

    cout << "Size of hashtable : " << chunk_to_sleechers.size() << endl;

    vector<char> zeros(1, 0);
    ofstream out(desti_path + "/" + name, ios::binary | ios::out);

    for (int i = 0; i < file_size_of_downloading_file; i++)
    {
        if (!out.write(&zeros[0], zeros.size()))
        {
            cout << "Error in downloading" << endl;
            return;
        }
    }
    out.close();

    int count_of_chunk_downloaded = 0;
    vector<string> empty(chunk_to_sleechers.size(), "0");
    chunks_present[grpname + " " + name] = empty;

    int thread = min((int)chunk_to_sleechers.size(), (int)THREAD_NUM);
    pthread_t th[thread];
    taskCount = 0;
    pthread_mutex_init(&mutexQueue, NULL);
    pthread_cond_init(&condQueue, NULL);
    int i;
    for (i = 0; i < thread; i++)
    {
        if (pthread_create(&th[i], NULL, &startThread, NULL) != 0)
        {
            perror("Failed to create the thread");
        }
    }

    pthread_mutex_init(&chunk_count_lock, NULL);

    for (auto it : piece_selection)
    {
        Task t = {
            .destination_loc = (desti_path + "/" + name),
            .chunk_number = it.first,
            .peer = it.second,
            .src_path = port_to_location,
            .count_of_chunk_downloaded = &count_of_chunk_downloaded,
            .grp = grpname,
            .filesize = to_string(file_size_of_downloading_file),
            .correct_SHA = correct_SHA};

        submitTask(t);
    }
    // pthread_exit(NULL);
    while (count_of_chunk_downloaded < chunk_to_sleechers.size())
    {
        // continue;
    }
    // for (i = 0; i < thread; i++)
    // {
    //     cout<<"Inside for loop"<<endl;
    //     cout<<"Value of thread is : "<<thread<<endl;
    //     // pthread_cancel(th[i]);
    //     // if (pthread_join(th[i], NULL) != 0)
    //     // {
    //     //     cout<<"Error in joining"<<endl;
    //     //     perror("Failed to join the thread");
    //     // }
    // }

    // pthread_mutex_destroy(&mutexQueue);
    // pthread_cond_destroy(&condQueue);
    usleep(200);
    // pthread_exit(NULL);

    string result_SHA = calculate_File_SHA(desti_path + "/" + name);

    if(strcmp(result_SHA.c_str(),correct_SHA.c_str())==0)
    {
        // cout<<"SHA Matched"<<endl;
    }
    else
    {
        // cout<<"SHA not Matched"<<endl;
    }
    // cout<<"Correct SHA : "<<correct_SHA<<endl;
    // cout<<"Downloaded_Files SHA :"<<result_SHA<<endl;
    cout << "[+] Complete File Downloaded" << endl;
    chunks_present_to_file();

    download_done[grpname].push_back(name);
    if (download_ongoing[grpname].size() == 1)
    {
        download_ongoing.erase(grpname);
    }
    else
    {

        auto vec = download_ongoing[grpname];
        auto it = find(vec.begin(), vec.end(), name);
        if (it != vec.end())
        {
            download_ongoing[grpname].erase(it);
        }
    }
    return;

    // vector<string> empty(chunk_to_sleechers.size(), "0");
    // chunks_present[splitted_cmd[1] + " " + splitted_cmd[2]] = empty;

    // for (auto it : chunk_to_sleechers)
    // {
    //     x++;
    //     string reply = chunk_download_request(desti_path + "/" + splitted_cmd[2], it.first, it.second[0], port_to_location[it.second[0]]);
    //     if (strcmp(reply.c_str(), "Done") == 0)
    //     {
    //         chunks_present[splitted_cmd[1] + " " + splitted_cmd[2]].at(it.first) = "1";
    //     }

    //     if (flag == 1)
    //     {
    //         flag = 0;
    //         string message_for_tracker = "upload_file " + desti_path + "/" + splitted_cmd[2] + " " + splitted_cmd[1] + " " + last_login + " " + to_string(file_size_of_downloading_file) + " " + Correct_SHA;
    //         string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
    //     }
    // }
    // cout << "Complete File Downloaded" << endl;

    // request_peer_to_download(reply_from_tracker, desti_path, splitted_cmd[2]);   //Commenting complete file download
    // ifstream fd(desti_path + "/" + splitted_cmd[2], ios::ate | ios::binary);
    // long long file_size = fd.tellg();
    // fd.close();
    // string message_for_tracker = "upload_file " + desti_path + "/" + splitted_cmd[2] + " " + splitted_cmd[1] + " " + last_login + " " + to_string(file_size);
    // string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
    // cout << "[+] Trackers reply : " << reply_from_tracker << endl;
}

void commands_collection(string input_command)
{
    vector<string> splitted_cmd = split_input_command(input_command);
    // cout << splitted_cmd[0] << endl;

    if (!current_online && splitted_cmd[0] == "create_user")
    {
        if (splitted_cmd.size() != 3)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1] + " " + splitted_cmd[2];
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;
        }
    }

    else if (!current_online && splitted_cmd[0] == "login")
    {
        if (splitted_cmd.size() != 3)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1] + " " + splitted_cmd[2] + " " + IP_of_PEER + ":" + to_string(PORT_of_PEER);
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;

            if (strcmp(reply_from_tracker.c_str(), "Login successfully") == 0)
            // if (reply_from_tracker == "Login successfully")
            {
                last_login = splitted_cmd[1];
                current_online = true;
                fill_up_data_structures();
            }
        }
    }

    else if (current_online && splitted_cmd[0] == "create_group")
    {
        if (splitted_cmd.size() != 2)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1] + " " + last_login;
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;

            // if (strcmp(reply_from_tracker.c_str(), "Login successfully") == 0)
            // // if (reply_from_tracker == "Login successfully")
            // {
            //     last_login = splitted_cmd[1];
            //     current_online = true;
            // }
        }
    }

    else if (current_online && splitted_cmd[0] == "join_group")
    {
        if (splitted_cmd.size() != 2)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1] + " " + last_login;
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;

            // if (strcmp(reply_from_tracker.c_str(), "Login successfully") == 0)
            // // if (reply_from_tracker == "Login successfully")
            // {
            //     last_login = splitted_cmd[1];
            //     current_online = true;
            // }
        }
    }

    else if (current_online && splitted_cmd[0] == "list_requests")
    {
        if (splitted_cmd.size() != 2)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1] + " " + last_login;
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;
        }
    }

    else if (current_online && splitted_cmd[0] == "list_groups")
    {
        if (splitted_cmd.size() != 1)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            string message_for_tracker = splitted_cmd[0];
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;
        }
    }

    else if (current_online && splitted_cmd[0] == "list_files")
    {
        if (splitted_cmd.size() != 2)
        {
            cout << "[-] Invalid commandsss" << endl;
        }
        else
        {
            string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1];
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;
        }
    }

    else if (current_online && splitted_cmd[0] == "leave_group")
    {
        if (splitted_cmd.size() != 2)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1] + " " + last_login;
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;


            if (strcmp("Group Left", reply_from_tracker.c_str()) == 0)
            {
                int k = chunks_present.size();
                int i = 0;
                for (auto it = chunks_present.begin(); i < k && it != chunks_present.end(); it++)
                {
                    i++;
                    if (chunks_present.size() == 0)
                    {
                        break;
                    }
                    string s = it->first;
                    int found = s.find(' ');
                    string grp_name = s.substr(0, found);
                    if (strcmp(grp_name.c_str(), splitted_cmd[1].c_str()) == 0)
                    {
                        chunks_present.erase(it);
                    }
                }
                chunks_present_to_file();
            }
        }
    }

    else if (current_online && splitted_cmd[0] == "accept_request")
    {
        if (splitted_cmd.size() != 3)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1] + " " + splitted_cmd[2] + " " + last_login;
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;
        }
    }

    else if (current_online && splitted_cmd[0] == "upload_file")
    {
        if (splitted_cmd.size() != 3)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            ifstream ud(splitted_cmd[1]);

            if (ud.good())
            {
                ud.close();
                ifstream fd(splitted_cmd[1], ios::ate | ios::binary);
                long long file_size = fd.tellg();
                fd.close();

                vector<string> SHA_CHUNKS;
                string SHA_FILE = calculate_SHA(splitted_cmd[1], SHA_CHUNKS);
                long long int number_of_chunks = ceil(float(file_size) / CHUNK_SIZE);

                string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1] + " " + splitted_cmd[2] + " " + last_login + " " + to_string(file_size) + " " + SHA_FILE;
                // cout<<message_for_tracker<<endl;
                string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
                cout << "[+] Trackers reply : " << reply_from_tracker << endl;

                if (strcmp(reply_from_tracker.c_str(), "File uploaded successfully") == 0)
                {
                    int found = splitted_cmd[1].find_last_of('/');
                    string file_name = splitted_cmd[1].substr(found + 1, splitted_cmd[1].size() - found);

                    string dest = "./data/SHA_uploaders/" + splitted_cmd[2] + "_" + file_name;
                    // cout<<dest<<endl;

                    fstream pa;
                    pa.open(dest, ios::app | ios::binary);
                    for (auto it : SHA_CHUNKS)
                    {
                        pa.write(SHA_FILE.c_str(), 20);
                    }
                    pa.write(SHA_FILE.c_str(), 20);
                    pa.close();

                    vector<string> temp(number_of_chunks, "1");
                    chunks_present[splitted_cmd[2] + " " + file_name] = temp;

                    string str = "";

                    for (auto it : temp)
                    {
                        str += it;
                    }
                    fstream ud;
                    ud.open("./data/Users_Upload/" + last_login + ".txt", ios::app);
                    struct_chunk_details ch_det;
                    ch_det.insert(splitted_cmd[2] + " " + file_name, str);
                    ud.write((char *)&ch_det, sizeof(ch_det));
                    ud.close();
                }
            }
            else
            {
                cout << "File does not exist at given path" << endl;
            }
        }
    }

    else if (current_online && splitted_cmd[0] == "download_file")
    {
        if (splitted_cmd.size() != 4)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            struct stat buffer;
            string desti_path = splitted_cmd[3];
            DIR *location;
            location = opendir(desti_path.c_str());

            if (location == NULL)
            {
                cout << "Destination path does not exists" << endl;
            }
            else
            {
                string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1] + " " + splitted_cmd[2] + " " + last_login;
                string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
                long long int file_size_of_downloading_file;

                vector<vector<string>> info_of_sleechers;
                if (isspace(reply_from_tracker[0]))
                {
                    vector<string> temp = split_input_command(reply_from_tracker);
                    vector<string> info_of_one;
                    unordered_map<string, string> port_to_location;
                    file_size_of_downloading_file = stoll(temp[2]);
                    string Correct_SHA = temp[3];

                    int count = 1;
                    string iport;
                    for (auto it : temp)
                    {

                        if (count % 4 == 1)
                        {
                            iport = it;
                            info_of_one.push_back(it);
                            count++;
                        }
                        else if (count % 4 == 0)
                        {
                            info_of_one.push_back(splitted_cmd[1]);
                            info_of_one.push_back(splitted_cmd[2]);
                            info_of_sleechers.push_back(info_of_one);
                            info_of_one.clear();
                            count++;
                        }
                        else if (count % 4 == 3)
                        {
                            count++;
                        }
                        else
                        {
                            port_to_location[iport] = it;
                            count++;
                        }
                    }

                    unordered_map<int, vector<string>> chunk_to_sleechers;
                    for (auto it : info_of_sleechers)
                    {
                        string s = request_peer_file_info(it);
                        make_hashtable_of_chunks(it[0], s, chunk_to_sleechers);
                    }

                    long long int number_of_chunks = ceil(float(file_size_of_downloading_file) / CHUNK_SIZE);
                    // cout << "Size of downloading file :" << file_size_of_downloading_file << endl;
                    // cout << "Size of hashtable : " << chunk_to_sleechers.size() << endl;
                    // cout << "Correct SHA is : " << Correct_SHA << endl;

                    if (chunk_to_sleechers.size() != number_of_chunks)
                    {
                        cout << "No Seeders online" << endl;
                        return;
                    }

                    thread download_parallel(download_in_another_thread, chunk_to_sleechers, desti_path, splitted_cmd[2], file_size_of_downloading_file, port_to_location, splitted_cmd[1], Correct_SHA);
                    download_parallel.detach();
                    download_ongoing[splitted_cmd[1]].push_back(splitted_cmd[2]);
                    return;
                }
                else
                {
                    cout << "[+] Trackers reply : " << reply_from_tracker << endl;
                }
            }
        }
    }

    else if (current_online && splitted_cmd[0] == "stop_share")
    {
        if (splitted_cmd.size() != 3)
        {
            cout << "[-] Invalid command" << endl;
        }

        else
        {
            string message_for_tracker = splitted_cmd[0] + " " + splitted_cmd[1] + " " + splitted_cmd[2] + " " + last_login;
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;

            if (strcmp("Sharing Stopped for the file", reply_from_tracker.c_str()) == 0)
            {
                int k = chunks_present.size();
                int i = 0;
                for (auto it = chunks_present.begin(); i < k && it != chunks_present.end(); it++)
                {
                    i++;
                    if (chunks_present.size() == 0)
                    {
                        break;
                    }
                    string s = it->first;
                    if (strcmp((splitted_cmd[1] + " " + splitted_cmd[2]).c_str(),s.c_str()) == 0)
                    {
                        chunks_present.erase(it);
                    }
                }
                chunks_present_to_file();
            }
            
        }
    }

    else if (current_online && splitted_cmd[0] == "logout")
    {
        if (splitted_cmd.size() != 1)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {
            string message_for_tracker = splitted_cmd[0] + " " + last_login;
            string reply_from_tracker = connection_request_to_tracker(message_for_tracker);
            cout << "[+] Trackers reply : " << reply_from_tracker << endl;
            current_online = false;
        }
    }

    else if (current_online && splitted_cmd[0] == "show_downloads")
    {
        if (splitted_cmd.size() != 1)
        {
            cout << "[-] Invalid command" << endl;
        }
        else
        {

            for (auto it : download_done)
            {
                cout << "[C] " << it.first << " ---> ";
                for (auto a : it.second)
                {
                    cout << a << " ";
                }
                cout << endl;
            }

            for (auto it : download_ongoing)
            {
                cout << "[D] " << it.first << " ---> ";
                for (auto a : it.second)
                {
                    cout << a << " ";
                }
                cout << endl;
            }
        }
    }

    else
    {
        cout << "[-] Invalid Command" << endl;
    }
    return;
}

void initialize_peer_as_server()
{
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        cout << "[-] Error occured in opening socket" << endl;
    }
    struct sockaddr_in server_details;
    server_details.sin_family = AF_INET;
    server_details.sin_port = htons(PORT_of_PEER);
    server_details.sin_addr.s_addr = INADDR_ANY;

    int b = bind(server_socket, (struct sockaddr *)&server_details, sizeof(server_details));
    if (b < 0)
    {
        cout << "[-] Error in binding" << endl;
        return;
    }
    listen(server_socket, CHUNK_SIZE);

    struct sockaddr_in client_details;

    while (true)
    {
        int client_address_length = sizeof(client_details);
        int client_socket;
        cout << "[+] Peer's server Listening..." << endl;
        client_socket = accept(server_socket, (struct sockaddr *)&client_details, (socklen_t *)&client_address_length); // accept return value of a client socket

        thread a(accept_downloading_request, client_socket, client_details);
        a.detach();

        // accept_downloading_request(client_socket, client_details);

        // if (client_socket < 0)
        // {
        //     cout << "[-] Not connected to client" << endl;
        //     break;
        // }
        // else
        // {
        //     int port_of_client = ntohs(client_details.sin_port);
        //     int ip_of_client = client_details.sin_addr.s_addr;

        //     cout << "[+] PORT of client is : " << port_of_client << endl;
        //     cout << "[+] Address of client is : " << ip_of_client << endl;
        // }
        // send(client_socket, "THIS IS A TEST", sizeof("THIS IS A TEST"), 0);
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
        string IP_plus_PORT(argv[1]);
        string info_path = argv[2];
        extract_ip_port(info_path);

        int found = IP_plus_PORT.find_last_of(':');
        IP_of_PEER = IP_plus_PORT.substr(0, found);
        PORT_of_PEER = stoi(IP_plus_PORT.substr(found + 1, IP_plus_PORT.size() - found));
        cout << IP_of_TRACKER << endl;
        cout << PORT_of_TRACKER << endl;
        // initialize_peer_as_server();
        thread separating_server_and_client(initialize_peer_as_server);
        separating_server_and_client.detach();

        // cout << "[+] After threading.." << endl;

        while (true)
        {
            string input_command;
            getline(cin, input_command, '\n');
            commands_collection(input_command);
        }
    }

    return 0;
}