#include "Global.hpp"

static Server *g_serverInstance = NULL;

void handleSignal(int signum)
{
	if (signum == SIGINT || signum == SIGTERM || signum == SIGQUIT)
	{
		std::cout << "\nSignal caught (" << signum << "). Shutting down ...\n";
		if (g_serverInstance)
			g_serverInstance->stop();
	}
}

int main(int argc, char **argv)
{
	// Argument validation
	int port;
	if (!validate_args(argv, argc, port))
		return 1;
	std::string password = argv[2];

	// Server launch
	Server server(port, password);
	try {
        server.initServerSocket();
    } 
	catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

	// Signals
	g_serverInstance = &server;
	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);
	signal(SIGQUIT, handleSignal);

	//Server loop
	server.run();

	return (0);
}
