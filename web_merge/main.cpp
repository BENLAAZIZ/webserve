
#include "web.h"
#include <signal.h>

volatile sig_atomic_t g_running = 1;

// Signal handler for graceful termination
void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down..." << std::endl;
    g_running = 0;
}

void setupSignalHandlers() {
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
	
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Failed to set SIGINT handler");
        exit(EXIT_FAILURE);
    }
	
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Failed to set SIGTERM handler");
        exit(EXIT_FAILURE);
    }
}

void printUsage(const char* programName) {
	std::cout << "Usage: " << programName << " [config_file]" << std::endl;
	std::cout << "  config_file: Path to the server configuration file" << std::endl;
	std::cout << "  If no config file is specified, default configuration will be used." << std::endl;
}
//========================================================
int main(int argc, char* argv[]) {
	std::string configPath = "default.conf";
	std::vector<Server> servers;
	std::vector<pollfd> fds;
	// std::vector<server> servers;
	std::map<int , Client> clients;
	std::vector<std::string> created;

	if (argc > 2) {
	    printUsage(argv[0]);
	    return EXIT_FAILURE;
	} else if (argc == 2) {
	    configPath = argv[1];
	    std::cout << "Using configuration file: " << configPath << std::endl;
	}
	
	// Set up signal handlers for graceful termination
	setupSignalHandlers();
	
	try {
	//     // Parse configuration
		Config config;
		// if (!config.parseFile(configPath)) {
		//     std::cerr << "Failed to load configuration from " << configPath << std::endl;
		//     return EXIT_FAILURE;
		// }
		
		std::cout << "Configuration loaded successfully from " << configPath << std::endl;
		
	//     // Initialize servers based on configuration
		const std::vector<ServerConfig>& serverConfigs = config.getServerConfigs();
		
		for (size_t i = 0; i < serverConfigs.size(); ++i) {
			try {
				Server server(serverConfigs[i]);
				if (server.createServer()) {
					servers.push_back(server);
					std::cout << "Server initialized on " << serverConfigs[i].getHost() 
							  << ":" << serverConfigs[i].getPort() << std::endl;
				} else {
					std::cerr << "Failed to initialize server on " << serverConfigs[i].getHost() 
							  << ":" << serverConfigs[i].getPort() << std::endl;
				}
			} catch (const std::exception& e) {
				std::cerr << "Error initializing server: " << e.what() << std::endl;
			}
		}

		
	//     if (servers.empty()) {
	//         std::cerr << "No servers could be initialized. Exiting." << std::endl;
	//         return EXIT_FAILURE;
	//     }
		
	//     std::cout << "Webserver started successfully. Press Ctrl+C to stop." << std::endl;
		
	//     // Main server loop
	    while (g_running) {
	        for (auto& server : servers) {
	            server.processEvents();
	        }
		}
		
	//     std::cout << "Shutting down servers..." << std::endl;
	//     // Cleanup will happen automatically as servers go out of scope
		
	} catch (const std::exception& e) {
	    std::cerr << "Fatal error: " << e.what() << std::endl;
	    return EXIT_FAILURE;
	}
	
	// std::cout << "Webserver shutdown complete." << std::endl;
	return 0;
}