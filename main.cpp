#include <iostream>
#include <string>

int main() {
    // std::string path = "index.html";
    // std::size_t pos = path.rfind('.'); // Find last '.'

    // if (pos != std::string::npos)
    //     std::cout << "Last '.' found at index: " << pos << std::endl;
    // else
    //     std::cout << "No '.' found" << std::endl;
    std::string path = "index.html";
    path += "hello";
    path += std::to_string(200);
    std::cout << path << std::endl;
    return 0;
}
