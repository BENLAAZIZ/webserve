

#include "include/web.h"

// int main() {
// 	try {
// 		// Instead of calling flush() every time, you can disable buffering globally :
// 		std::cout.setf(std::ios::unitbuf); // Disables buffering completely
// 		// Use flush(); when redirecting output to files to prevent missing data! 🚀
// 		// //std::cout.flush();  // ✅ Ensures all data is written immediately

// 		std::vector<int> ports;
// 		ports.push_back(8080);
// 		ports.push_back(5050);
// 		ports.push_back(8082);

// 		ConfigFile configFile;

// 		for (size_t i = 0; i < ports.size(); ++i) {
// 			configFile.addServer(ports[i]);
// 		}
// 		configFile.run();
		
// 	} catch (const std::exception& e) {
// 		std::cerr << "Error: " << e.what() << std::endl;
// 	}
// 	return 0;
// }

int main(int ac, char **av) {
	try {
		// Instead of calling flush() every time, you can disable buffering globally :
		std::cout.setf(std::ios::unitbuf); // Disables buffering completely
		// Use flush(); when redirecting output to files to prevent missing data! 🚀
		// //std::cout.flush();  // ✅ Ensures all data is written immediately

	
		ConfigFile configFile;

		configFile.parser.process_config(ac, av);
		// configFile.parser.prints_serves_configuration();
		// configFile.parser.get_servers();

		for (size_t i = 0; i < configFile.parser.servers.size(); ++i) {
			for (size_t j = 0; j < configFile.parser.servers[i].port.size(); ++j) {
				// std::cout << "Server " << i << " Port: " << configFile.parser.servers[i].port[j] << std::endl;
				configFile.addServer(configFile.parser.servers[i].port[j], configFile.parser.servers[i]);
			}
		}
		configFile.run();

	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}