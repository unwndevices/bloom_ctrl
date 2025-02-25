import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

// Mock schedules data
let mockSchedules = [
  {
    id: 1,
    relayId: 2, // Grow Lights
    startTime: '08:00',
    endTime: '20:00',
    days: [true, true, true, true, true, true, true], // Every day
    enabled: true
  },
  {
    id: 2,
    relayId: 1, // Water Pump
    startTime: '06:00',
    endTime: '06:15',
    days: [true, true, true, true, true, true, true], // Every day
    enabled: true
  },
  {
    id: 3,
    relayId: 1, // Water Pump
    startTime: '18:00',
    endTime: '18:15',
    days: [true, true, true, true, true, true, true], // Every day
    enabled: true
  }
];

// GET handler for fetching all schedules
export const GET: RequestHandler = async () => {
  // In a real implementation, this would fetch data from the ESP32
  return json(mockSchedules);
};

// POST handler for creating a new schedule
export const POST: RequestHandler = async ({ request }) => {
  try {
    const data = await request.json();
    
    // Generate a new ID
    const newId = Math.max(0, ...mockSchedules.map(s => s.id)) + 1;
    
    // Create the new schedule
    const newSchedule = {
      id: newId,
      ...data
    };
    
    // Add to the mock data
    mockSchedules.push(newSchedule);
    
    // In a real implementation, this would send a command to the ESP32
    return json(newSchedule);
  } catch (error) {
    return new Response('Invalid request body', { status: 400 });
  }
}; 