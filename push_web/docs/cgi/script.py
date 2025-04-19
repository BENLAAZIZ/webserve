#!/usr/bin/env python3

import cgi
import sys

# Set the content type to HTML
print("Content-Type: text/html")
print()  # Empty line to signal the end of the headers

# Print the HTML content
print("<!DOCTYPE html>")
print("<html lang=\"en\">")
print("<head>")
print("    <meta charset=\"UTF-8\">")
print("    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">")
print("    <title>My CGI Script Output</title>")
print("</head>")
print("<body>")
print("    <h1>Hello from my Python CGI script!</h1>")
print("    <p>This script is running on your web server.</p>")
print("</body>")
print("</html>")