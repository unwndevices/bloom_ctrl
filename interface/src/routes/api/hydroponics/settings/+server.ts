import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

// Mock settings data
let mockSettings = {
  phTarget: 6.2,
  ecTarget: 1.8,
  temperatureTarget: 24.0,
  lightOnTime: '08:00',
  lightOffTime: '20:00',
  pumpCycleOnDuration: 15, // minutes
  pumpCycleOffDuration: 45, // minutes
  nutrientDosingEnabled: true,
  phCorrectionEnabled: true
};

// GET handler for fetching settings
export const GET: RequestHandler = async () => {
  // In a real implementation, this would fetch data from the ESP32
  return json(mockSettings);
};

// PUT handler for updating settings
export const PUT: RequestHandler = async ({ request }) => {
  try {
    const data = await request.json();
    
    // Update the settings
    mockSettings = {
      ...mockSettings,
      ...data
    };
    
    // In a real implementation, this would send a command to the ESP32
    return json({ success: true, settings: mockSettings });
  } catch (error) {
    return new Response('Invalid request body', { status: 400 });
  }
}; 