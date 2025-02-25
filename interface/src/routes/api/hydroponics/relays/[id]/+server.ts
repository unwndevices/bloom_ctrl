import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

// Mock relay data (same as in the parent endpoint)
const mockRelays = [
  {
    id: 1,
    name: 'Water Pump',
    state: false,
    pin: 5,
    type: 'pump'
  },
  {
    id: 2,
    name: 'Grow Lights',
    state: false,
    pin: 4,
    type: 'light'
  },
  {
    id: 3,
    name: 'Ventilation Fan',
    state: false,
    pin: 2,
    type: 'fan'
  },
  {
    id: 4,
    name: 'Nutrient Pump',
    state: false,
    pin: 15,
    type: 'nutrient'
  }
];

// PUT handler for toggling a relay state
export const PUT: RequestHandler = async ({ params, request }) => {
  const id = parseInt(params.id);
  const relay = mockRelays.find(r => r.id === id);
  
  if (!relay) {
    return new Response('Relay not found', { status: 404 });
  }
  
  try {
    const data = await request.json();
    // Update the relay state
    relay.state = data.state;
    
    // In a real implementation, this would send a command to the ESP32
    return json({ success: true, relay });
  } catch (error) {
    return new Response('Invalid request body', { status: 400 });
  }
}; 