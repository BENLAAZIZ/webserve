/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aben-cha <aben-cha@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 13:14:14 by aben-cha          #+#    #+#             */
/*   Updated: 2025/02/22 16:30:42 by aben-cha         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int main() {
    try {
        // Instead of calling flush() every time, you can disable buffering globally :

        std::cout.setf(std::ios::unitbuf); // Disables buffering completely
        // Use flush(); when redirecting output to files to prevent missing data! 🚀
        // std::cout.flush();  // ✅ Ensures all data is written immediately

        Server server(PORT);
        server.run();
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}