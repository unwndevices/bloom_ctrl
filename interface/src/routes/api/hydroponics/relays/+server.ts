import { json } from '@sveltejs/kit';
import type { RequestHandler } from './$types';

// Mock relay data
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

// GET handler for fetching all relays
export const GET: RequestHandler = async () => {
  // In a real implementation, this would fetch data from the ESP32
  return json(mockRelays);
}; 