#!/usr/bin/env python3
import cgi
import sys

# Set the content type to HTML
# print("Content-Type: text/html")
print()  # Print the HTTP headers

# Print the HTML content (use triple quotes for multi-line strings)
print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI</title>
</head>
<body>
    <h1>CGI test</h1>
</body>
</html>
""")
