import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

// Mock sensor data
function generateMockSensorData() {
  // Generate realistic but slightly varying sensor data
  return {
    temperature: 23 + Math.random() * 2, // 23-25°C
    ph: 6 + Math.random(), // 6.0-7.0 pH
    ec: 0.6 + Math.random() * 1.2, // 0.6-1.8 μS/cm
    waterLevel: 75 + Math.random() * 10, // 75-85%
    humidity: 60 + Math.random() * 10, // 60-70%
    timestamp: Date.now()
  };
}

// GET handler for fetching sensor data
export const GET: RequestHandler = async () => {
  // In a real implementation, this would fetch data from the ESP32
  return json(generateMockSensorData());
}; 