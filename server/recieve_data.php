<?php
require_once 'config.php';

header('Content-Type: application/json');

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    
    $temperature = isset($_POST['temperature']) ? floatval($_POST['temperature']) : null;
    $humidity = isset($_POST['humidity']) ? floatval($_POST['humidity']) : null;
    $pressure = isset($_POST['pressure']) ? floatval($_POST['pressure']) : null;
    $altitude = isset($_POST['altitude']) ? floatval($_POST['altitude']) : null;
    
    if ($temperature === null || $humidity === null || $pressure === null) {
        echo json_encode(['status' => 'error', 'message' => 'Missing parameters']);
        exit;
    }
    
    try {
        $conn = getDBConnection();
        
        $sql = "INSERT INTO " . DB_TABLE . " (temperature, humidity, pressure, altitude, timestamp) 
                VALUES (:temperature, :humidity, :pressure, :altitude, NOW())";
        
        $stmt = $conn->prepare($sql);
        $stmt->bindParam(':temperature', $temperature);
        $stmt->bindParam(':humidity', $humidity);
        $stmt->bindParam(':pressure', $pressure);
        $stmt->bindParam(':altitude', $altitude);
        
        if ($stmt->execute()) {
            echo json_encode([
                'status' => 'success',
                'message' => 'Data saved',
                'data' => compact('temperature', 'humidity', 'pressure', 'altitude')
            ]);
        }
        
    } catch(PDOException $e) {
        echo json_encode(['status' => 'error', 'message' => $e->getMessage()]);
    }
    
} else {
    echo json_encode(['status' => 'error', 'message' => 'Only POST allowed']);
}
?>