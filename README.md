# Peer to Peer File Sharing
A user-friendly multi-threaded file-sharing system developed using socket programming, designed to facilitate seamless and concurrent file-sharing among connected peers. It allows users to share, download, and delete files within their designated groups. Downloads occur concurrently, with files being retrieved in parts from multiple peers simultaneously.


![Image](https://github.com/Advait-Shrivastava/Peer-to-Peer-File-Sharing-System/assets/59224726/34ab1157-b58b-47cd-b4d0-ede639ca3ffb)

## Description

The system has 2 main components :
1. Tracker
2. Client

<br>

### Tracker:
- It is a centralized server which maintains the information of all the clients.
- It retains all the data regarding which files are shared by each client and the specific pieces available with each client.

<br>

### Client:

- Client is the user which download or uploads the file.
- Before beginning, users must create an account and register with the tracker.
- Login Using The User Credentials
- Create Group and hence will become owner of that group
- Fetch list of all Groups in server
- Request to Join Group
- Leave Group
- Accept Group join requests(if owner)
- Share file across group: Share the filename and SHA1 hash of the
complete file as well as piecewise SHA1 with the tracker
- Fetch list of all sharable files in a Group
- Download file
- Retrieve peer information from tracker for the file
- Core Part: Download file from multiple peers (different pieces of file
from different peers - piece selection algorithm) simultaneously and
all the files which client downloads will be shareable to other users
in the same group. Ensure file integrity from SHA1 comparison
- Show downloads
- Stop sharing file
- Stop sharing all files(Logout)
- Whenever client logins, all previously shared files before logout should
automatically be on sharing mode

## Client Commands

- Create User Account: `create_user <user_id> <password>`

- Login: `login <user_id> <password>`
- Create Group: `create_group <group_id>`
- Join Group: `join_group <group_id>`
- Leave Group: `leave_group <group_id>`
- List Pending Join: `list_requests<group_id>`
- Accept Group Joining Request:
`accept_request <group_id> <user_id>`
- List All Group In Network: `list_groups`
- List All sharable Files In Group: `list_files <group_id>`
- Upload File: `upload_file <file_path> <group_id>`
- Download File:
`download_file <group_id> <file_name>
<destination_path>`
- Logout: `logout`
- Show_downloads: `show_downloads`
- Stop sharing: `stop_share <group_id> <file_name>`


## Execution
`tracker_info.txt` file have the IP and Port details of all the trackers.

#### Tracker
 1. `g++ tracker.cpp -o tracker`
 2. To run more than one trackers mention the count in the command. `./tracker tracker_info.txt 2`.

#### Client
 1. `g++ client.cpp -o client1` (Multiple clients can be created using different names)
 2. `./client tracker_info.txt`
