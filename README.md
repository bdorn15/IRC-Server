A fully functional IRC server built in C++ as part of the 42 School curriculum.

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![42](https://img.shields.io/badge/42-School-black?style=for-the-badge)
![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)

---

## 📖 About

`ft_irc` is a non-blocking, single-threaded IRC server using `poll()` for I/O multiplexing. It supports multiple simultaneous clients, follows the IRC protocol (RFC 1459), and is compatible with standard IRC clients like **irssi**, **WeeChat**, or **HexChat**.

No forking is used — all I/O operations are non-blocking and handled through a single `poll()` call.

---

## ✨ Features

- 🔌 Handle multiple clients simultaneously without blocking
- 🔐 Client authentication via connection password
- 💬 Channel creation, joining, and messaging
- 📢 Broadcast messages to all clients in a channel
- 👑 Operator and regular user roles
- 📦 Packet aggregation (handles split/partial commands correctly)
- 🖥️ TCP/IP (v4) communication

---

## 🚀 Getting Started

### Prerequisites

- `g++` or `clang++`
- `make`
- An IRC client: [irssi](https://irssi.org/), [WeeChat](https://weechat.org/), or [HexChat](https://hexchat.github.io/)

### Installation

```bash
git clone https://github.com/yourusername/ft_irc.git
cd ft_irc
make
```

### Running the Server

```bash
./ircserv <port> <password>
```

| Argument   | Description                                         |
|------------|-----------------------------------------------------|
| `port`     | The port number the server listens on               |
| `password` | The connection password required by IRC clients     |

**Example:**

```bash
./ircserv 6667 mypassword
```

### Connecting with a Client

```bash
# irssi
irssi -c 127.0.0.1 -p 6667 -w mypassword

# WeeChat
/server add ft_irc 127.0.0.1/6667 -password=mypassword
/connect ft_irc
```

---

## 💬 Supported Commands

| Command   | Description                                         |
|-----------|-----------------------------------------------------|
| `PASS`    | Authenticate with the server password               |
| `NICK`    | Set or change nickname                              |
| `USER`    | Set username and real name                          |
| `JOIN`    | Join a channel                                      |
| `PART`    | Leave a channel                                     |
| `PRIVMSG` | Send a message to a user or channel                 |
| `KICK`    | Eject a user from a channel *(operator only)*       |
| `INVITE`  | Invite a user to a channel *(operator only)*        |
| `TOPIC`   | View or change a channel's topic                    |
| `MODE`    | Set channel or user modes                           |
| `QUIT`    | Disconnect from the server                          |

---

## ⚙️ Channel Modes

Set with the `MODE` command by channel operators:

| Mode | Description                                         |
|------|-----------------------------------------------------|
| `i`  | Invite-only — only invited users can join           |
| `t`  | Restrict `TOPIC` changes to operators only          |
| `k`  | Set or remove a channel password (key)              |
| `o`  | Grant or revoke channel operator privileges         |
| `l`  | Set or remove a user limit for the channel          |

---

## 🧪 Testing

Test partial data handling using `nc`:

```bash
nc -C 127.0.0.1 6667
```

Type parts of a command and flush them separately with `Ctrl+D` to simulate split packets. The server correctly aggregates and processes the full command once complete.


## 📄 License

This project is part of the 42 School curriculum. Feel free to use it as a reference.
