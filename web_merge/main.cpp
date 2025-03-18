


#include "include/web.h"

int main() {
	try {
		// Instead of calling flush() every time, you can disable buffering globally :
		std::cout.setf(std::ios::unitbuf); // Disables buffering completely
		// Use flush(); when redirecting output to files to prevent missing data! 🚀
		// //std::cout.flush();  // ✅ Ensures all data is written immediately

		std::vector<int> ports;
		ports.push_back(8080);
		ports.push_back(5050);
		// ports.push_back(8082);

		ConfigFile configFile;

		for (size_t i = 0; i < ports.size(); ++i) {
			configFile.addServer(ports[i]);
		}
		configFile.run();
		
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return 0;
}

// -rw-r--r--  1 hben-laz  candidate  7836024 Mar 17 23:08 vid.mp4
//-rw-r--r--  1 hben-laz  candidate   1740492 Mar 18 00:51 filetest.txt