<?php
echo "Content-Type: text/html";
// print(  # Empty line to signal the end of the headers

# Print the HTML content
echo "<!DOCTYPE html>
        <html lang=\"en\">
        <head>
            <meta charset=\"UTF-8\">
            <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
            <title>My CGI Script Output</title>
        </head>
        <body>
            <h1>Hello from my PHP CGI script!</h1>
            <p>This script is running on your web server.</p>
        </body>
        </html>"
?>