#include "ValidArgs.hpp"
#include "bot.hpp"
#include <csignal>


static bot *g_botInstance = NULL;

void handleSignal(int signum)
{
	if (signum == SIGINT || signum == SIGTERM || signum == SIGQUIT)
	{
		std::cout << "\nSignal caught (" << signum << "). Shutting down ...\n";
		if (g_botInstance)
		{
			g_botInstance->stop();
			g_botInstance->closeFd();
		}
	}
}

int main(int argc, char **argv)
{
	int port;
	if (!validate_args(argv, argc, port))
		return 1;
	std::string password = argv[2];
	std::string host = "127.0.0.1";

	bot bot(password, port, host);

	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);
	signal(SIGQUIT, handleSignal);

	if (bot.connectToServer() < 0)
		return 1;
	if (!bot.authenticate())
		return 1;

	g_botInstance = &bot;

	bot.run();

	return 0;
};
