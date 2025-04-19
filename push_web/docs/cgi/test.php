<?php
# This is a basic PHP CGI script that prints some HTML.

# Set the content type to HTML
header('Content-Type: text/html');

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PHP CGI Output</title>
</head>
<body>
    <h1>Hello from my PHP CGI script!</h1>
    <p>This script is running on your web server using PHP.</p>
    <?php
    # You can embed PHP code within the HTML.
    $current_time = date("Y-m-d H:i:s");
    echo "<p>The current time on the server is: " . htmlspecialchars($current_time) . "</p>";
    ?>
</body>
</html>