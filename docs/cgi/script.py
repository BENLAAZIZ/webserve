
import cgi
import sys
import time  # Added the time module

print("Content-Type: text/html")
print()  # Print the HTTP headers


while True:  # Using True is more Pythonic than while (1)
    print("""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI Loop</title>
</head>
<body>
    <h1>CGI test</h1>
    <p>This page will keep reloading...</p>
</body>
</html>
""")
