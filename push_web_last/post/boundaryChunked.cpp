/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   boundaryChunked.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hben-laz <hben-laz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/14 23:49:33 by hben-laz          #+#    #+#             */
/*   Updated: 2025/04/13 22:09:13 by hben-laz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "../include/Request.hpp"
#include "../include/web.h"

// void handleChunkedBoundary(HTTPRequest& request) {
//     handleChunkedData(request);

//     if (request._requestBuffer.empty()) {
//         return;
//     }

//     request.buffer += request._requestBuffer;
//     request._requestBuffer.clear();

//     handleBoundary(request);
// }