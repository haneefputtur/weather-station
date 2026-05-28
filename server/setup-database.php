<?php
require_once 'config.php';

try {
    $conn = getDBConnection();
    
    $sql = "CREATE TABLE IF NOT EXISTS " . DB_TABLE . " (
        id INT AUTO_INCREMENT PRIMARY KEY,
        temperature FLOAT NOT NULL,
        humidity FLOAT NOT NULL,
        pressure FLOAT NOT NULL,
        altitude FLOAT,
        timestamp DATETIME NOT NULL,
        INDEX idx_timestamp (timestamp)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4";
    
    $conn->exec($sql);
    
    echo "✓ Table created successfully!<br>";
    echo "<strong>DELETE THIS FILE after setup!</strong>";
    
} catch(PDOException $e) {
    echo "Error: " . $e->getMessage();
}
?>