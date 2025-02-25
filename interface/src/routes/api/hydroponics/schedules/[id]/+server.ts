import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

// Mock schedules data (same as in the parent endpoint)
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

// PUT handler for updating a schedule
export const PUT: RequestHandler = async ({ params, request }) => {
  const id = parseInt(params.id);
  const scheduleIndex = mockSchedules.findIndex(s => s.id === id);
  
  if (scheduleIndex === -1) {
    return new Response('Schedule not found', { status: 404 });
  }
  
  try {
    const data = await request.json();
    
    // Update the schedule
    mockSchedules[scheduleIndex] = {
      ...mockSchedules[scheduleIndex],
      ...data,
      id // Ensure ID doesn't change
    };
    
    // In a real implementation, this would send a command to the ESP32
    return json({ success: true, schedule: mockSchedules[scheduleIndex] });
  } catch (error) {
    return new Response('Invalid request body', { status: 400 });
  }
};

// DELETE handler for deleting a schedule
export const DELETE: RequestHandler = async ({ params }) => {
  const id = parseInt(params.id);
  const scheduleIndex = mockSchedules.findIndex(s => s.id === id);
  
  if (scheduleIndex === -1) {
    return new Response('Schedule not found', { status: 404 });
  }
  
  // Remove the schedule
  mockSchedules.splice(scheduleIndex, 1);
  
  // In a real implementation, this would send a command to the ESP32
  return json({ success: true });
}; 