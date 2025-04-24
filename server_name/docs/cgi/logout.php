<?php
session_start();
session_unset();
session_destroy();

setcookie('user', '', time() - 3600, '/'); // Delete cookie
function start_html() {
    echo '<!DOCTYPE html><html><head><meta charset="UTF-8">';
    echo '<style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f7f9fc;
            color: #333;
            padding: 20px;
        }
        .container {
            background-color: white;
            border: 1px solid #ddd;
            max-width: 500px;
            margin: 50px auto;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 4px 12px rgba(0,0,0,0.1);
        }
        h1 {
            color: #2c3e50;
        }
        p {
            margin: 10px 0;
        }
        a {
            color: #3498db;
            text-decoration: none;
        }
        a:hover {
            text-decoration: underline;
        }
        .debug {
            font-size: 0.85em;
            color: #999;
            margin-bottom: 15px;
        }
    </style></head><body><div class="container">';
}

function end_html() {
echo '</div></body></html>';
}

start_html();
echo "<h1>You have been logged out.</h1>";
echo "<a href='/cgi/index.html'>Log in again</a>";
end_html();
?>

<!-- <?php
    // session_start();
    // session_destroy(); // Clear session
    // setcookie("PHPSESSID", "", time() - 3600, "/"); // Remove cookie

    // echo "<h1>You’ve been logged out.</h1>";
    // echo "<a href='/login.html'>Login again</a>";
?> -->