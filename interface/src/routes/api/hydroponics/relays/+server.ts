import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

// Mock relay data
const mockRelays = [
  {
    id: 1,
    name: 'Water Pump',
    state: false,
    pin: 7,
    type: 'pump'
  },
  {
    id: 2,
    name: 'Grow Lights',
    state: false,
    pin: 10,
    type: 'light'
  },
  {
    id: 3,
    name: 'Extra',
    state: false,
    pin: 0,
    type: 'extra'
  }
];

// GET handler for fetching all relays
export const GET: RequestHandler = async () => {
  // In a real implementation, this would fetch data from the ESP32
  return json(mockRelays);
}; 