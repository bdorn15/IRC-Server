 #include "Global.hpp"

Server::Server(int port, const std::string &password) 
		: _port(port), _password(password), _serverSocket(-1), _running(false) {}

Server::~Server()
{
	for (auto &pair : _clients)
    {
        close(pair.first);
        delete pair.second;
    }
	for (auto &pair : _channels)
    {
        delete pair.second;
    }
	_channels.clear();
	_clients.clear();
	if(_serverSocket >= 0)
		close(_serverSocket);
}

//creates the listening socket and makes it nonblocking
void Server::initServerSocket()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket < 0)
		throw std::runtime_error("Failed to create socket");
	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt failed");
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcnlt failed");
	struct sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(_port);
	if (bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
	{
		std::cout << "Error: " << strerror(errno) << std::endl;
		throw std::runtime_error("bind failed");
	}
	if (listen(_serverSocket, SOMAXCONN) < 0)
    	throw std::runtime_error("listen failed");
	_running = true;
}

int Server::getServerSocket() const
{
	return _serverSocket;
}

void Server::setClient(int fd, Client *newClient)
{
	_clients[fd] = newClient;
}

void Server::setChannel(Channel* ch)
{
	std::string name = ch->getChannelName();
	_channels[name] = ch;
}

void Server::removeClient(int fd)
{
	if (_clients.find(fd) != _clients.end())
	{
		delete _clients[fd];
		_clients.erase(fd);
	}
}

void Server::removeChannel(const std::string& ch_name)
{
	if (_channels.find(ch_name) != _channels.end())
	{
		delete _channels[ch_name];
		_channels.erase(ch_name);
	}
}

Client* Server::getClient(int fd)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end())
		return it->second;
	return nullptr;
}

bool Server::getState()
{
	return _running;
}

void Server::stop()
{
	_running = false;
}

/**
 * @brief API to request a Channel handle with a specific name
 */
Channel* Server::findChannel(const std::string& ch_name) const
{
	const auto it = _channels.find(ch_name);
	if (it != _channels.end())
		return it->second;
	return nullptr;
}

/**
 * @brief API to request a Client handle with a specific nickname
 */
Client* Server::findClient(const std::string& cl_nick) const
{
	for (const auto& it : _clients) {
		if (it.second && it.second->getNick() == cl_nick)
			return it.second;
	}
	return nullptr;
}

void Server::run()
{
	//Set up pollfd container
	std::vector<pollfd> fds;
	pollfd server_fd;
	server_fd.fd = _serverSocket;
	server_fd.events = POLLIN;
	fds.push_back(server_fd);
	
	//Serverloop for incoming connections
	while(_running)
	{
		for (size_t i = 0; i < fds.size(); ++i)
		{
			Client *client = getClient(fds[i].fd);
			if (client && !client->isRegistered() && time(nullptr) - client->getConTime() > REG_TIMEOUT)
			{
				std::cout << "Client reg timeout FD: " << client->getFd() << std::endl;
				removeClient(fds[i].fd);
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				--i;
				continue;
			}
			int activity = poll(&fds[i], 1, 0);
			if (activity <= 0)
				continue;
			if (fds[i].revents & POLLIN)
			{
				//Handle action on serversocket (accept clients)
				if (fds[i].fd == _serverSocket)
				{
					struct sockaddr_in clientAddr;
					socklen_t clientLen = sizeof(clientAddr);
					int clientFd = accept(fds[0].fd, (struct sockaddr*)&clientAddr, &clientLen);
					if (clientFd < 0)
					{
						std::cerr << "Accept failed: " << strerror(errno) << std::endl;
						continue;
					}
					
					std::string ip = inet_ntoa(clientAddr.sin_addr);

					fcntl(clientFd, F_SETFL, O_NONBLOCK);

					Client* newClient = new Client(clientFd, ip);
					if (newClient)
						setClient(clientFd, newClient);
					
					pollfd clientPoll;
					clientPoll.fd = clientFd;
					clientPoll.events = POLLIN;
					fds.push_back(clientPoll);
					std::cout << "New Client connected. FD: " << clientFd << std::endl;
				}
				//Handle action on non server socket (client fds)
				else
				{
					char buffer[1024];
					int bytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);

					//Disconnect client
					if (bytes <= 0)
					{
						std::cout << "Client disconnected: FD " << fds[i].fd << std::endl;
						std::vector<std::string> tmpVec = {"QUIT", "Connection closed"};
						QUIT(*getClient(fds[i].fd), tmpVec);
						removeClient(fds[i].fd);
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						--i;
					}
					//Store incoming data from client + check for executable command (execute if possible)
					else
					{
						if (client)
						{
							client->getInBuffer().append(buffer, bytes);
							if (!client->processBuffer(*this))
							{
								std::cout << "Client reg fail or Client quit FD: " << client->getFd() << std::endl;
								removeClient(fds[i].fd);
								close(fds[i].fd);
								fds.erase(fds.begin() + i);
								--i;
							}
						}
					}	
				}
			}
		}
	}
	fds.clear();
}