#!/usr/bin/env python3
# import cgi
# import sys

# # Set the content type to HTML
# # print("Content-Type: text/html")
# print()  # Print the HTTP headers

# # Print the HTML content (use triple quotes for multi-line strings)
# while (1) {
# print("""
# <!DOCTYPE html>
# <html lang="en">
# <head>
#     <meta charset="UTF-8">
#     <meta name="viewport" content="width=device-width, initial-scale=1.0">
#     <title>CGI</title>
# </head>
# <body>
#     <h1>CGI test</h1>
# </body>
# </html>
# """)
# }


#!/usr/bin/env python3
import cgi
import sys
import time  # Added the time module

# Set the content type to HTML
print("Content-Type: text/html")
print()  # Print the HTTP headers

# Print the HTML content (use triple quotes for multi-line strings)
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
    # sys.stdout.flush()  # Ensure the output is sent immediately
    # time.sleep(1)       # Wait for 1 second before the next iteration