#include <iostream>
#include <map>
#include <string>

int main() {
    std::map<std::string, int> ages;

    // Insert elements into the map
    ages["Alice"] = 25;
    ages["Bob"] = 30;
    ages["Charlie"] = 22;
    ages["hoast"] = 1000;

    // // Check if a key exists
    // if (ages.find("host") != ages.end()) {
    //     //std::cout << "host exist" << std::endl;
    // }

    std::string h = "host";
    std::string b;
    b = h;
    //std::cout << "b = " << b << std::endl;
    // key= "alice"
    // value

    return 0;
}