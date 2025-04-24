<?php

session_start();

// echo "Session save path: " . session_save_path();
// echo "<br>Session name: " . session_name();


// echo "<pre>";
// print_r($_COOKIE);
// print_r($_SESSION);
// echo "</pre>";
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

if (isset($_SESSION['loggedin']) && $_SESSION['loggedin'] === true) {
    echo "<h1>Welcome back, " . htmlspecialchars($_SESSION['user']) . "!</h1>";
    echo "<p><a href='/cgi/logout.php'>Logout</a></p>";
} else {
    echo "<h1>Access Denied</h1>";
    echo "<p>You must log in first.</p>";
    echo "<a href='/cgi/index.html'>Go to login</a>";
}
end_html();
?>