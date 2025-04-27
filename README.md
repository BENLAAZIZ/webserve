

 #   Webserv
This project is here to help you write your own HTTP server. You will be able to test it with a real browser. HTTP is one of the most used protocols on the internet. Knowing its intricacies will be useful, even if you won’t be working on a website.


## Key Features to Implement
______________________________

    1. Basic HTTP/1.1 Protocol Handling
    --------------------------------

        - Parsing HTTP requests (GET, POST, DELETE, etc.).
        - Sending appropriate HTTP responses with correct status codes (e.g., 200 OK, 404 Not Found).
        - Supporting headers like Content-Type, Content-Length, etc.

    2. Connection Management
    -------------------------

        - Handling multiple client connections using select, poll, or epoll.
        - Proper error handling and timeouts for connections.
    
    3. Configuration File
    ----------------------

        - Allowing server behavior to be configured via a file (similar to nginx.conf).
        - Define server blocks, routes, ports, and supported methods.
    
    4. Static File Hosting
    -----------------------

        - Serve static files (HTML, CSS, images) from a predefined directory.

    5. CGI (Common Gateway Interface) Support
    -------------------------------------------

        - Executing scripts (e.g., PHP, Python) dynamically to generate responses.
    
    6. Error Pages
    ---------------

        - Customizable error pages for specific HTTP status codes (e.g., 404.html).

    7. Concurrency
    ---------------

        - Efficiently handling multiple clients simultaneously (non-blocking IO).
    

## Recommended Tools and Resources
_____________________________________

    C++ STL: Use libraries like std::map, std::vector, std::string for parsing and data management.
    --------

    System Calls: Familiarize yourself with socket(), bind(), listen(), accept(), and related networking calls.
    --------------

    RFC 2616: The official documentation for HTTP/1.1 protocol.
    ---------

    Debugging Tools: Use tools like curl, Postman, or your browser to test your server.
    -----------------


## Common Challenges
__________________________

    1. Parsing HTTP Requests:
    --------------------------
        - Managing malformed requests and edge cases can be tricky.
        - Pay close attention to handling headers and payloads properly.

    2. Concurrency:
    ---------------
        - Implementing non-blocking IO using select or poll may require careful debugging.
        - Memory leaks or crashes can occur under heavy load if resources aren’t cleaned up properly.

    3. Timeouts and Error Handling:
    ------------------------------

        - Ensure that the server handles dropped connections and invalid requests gracefully.
        