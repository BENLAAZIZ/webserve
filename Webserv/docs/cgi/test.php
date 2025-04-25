<?php
// Infinite loop test
header("Content-Type: text/plain");
while (true) {
    echo "This script is running in an infinite loop!\n";
    // Sleep to reduce CPU usage (but it still loops infinitely)
    sleep(1);
}
?>
