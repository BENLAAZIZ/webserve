<?php

session_start();
// $debug_session_path = session_save_path();
// $debug_session_name = session_name();

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

// start_html();

// echo "<div class='debug'>Session save path: " . $debug_session_path . "<br>Session name: " . $debug_session_name . "</div>";

    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        $username = isset($_POST['username']) ? $_POST['username'] : '';
        $password = isset($_POST['password']) ? $_POST['password'] : '';
        
        if ($username === 'admin' && $password === '1234') {
            $_SESSION['user'] = $username;
            $_SESSION['loggedin'] = true;

            setcookie('user', $username, time() + 3600, '/');
            start_html();
            echo "<h1>Login Successful!</h1>";
            echo "<p>Welcome, " . htmlspecialchars($username) . "!</p>";
            echo "<p><a href='/cgi/dashboard.php'>Go to Dashboard</a></p>";
        } else {
            start_html();
            echo "<h1>Login Failed</h1>";
            echo "<p>Invalid username or password.</p>";
            echo "<p><a href='/cgi/index.html'>Try Again?</a></p>";
        }
    } else {
        start_html();
        echo "<h1>Invalid Request</h1>";
        echo "<p>This page only accepts POST requests.</p>";
    }

end_html();
?>