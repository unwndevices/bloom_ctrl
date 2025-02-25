/**
 * Hydroponics API Service
 * 
 * Provides functions for interacting with the ESP32 backend for the hydroponics system.
 */

// Sensor data interface
export interface SensorData {
  temperature: number;
  ph: number;
  ec: number;
  waterLevel: number;
  humidity: number;
  timestamp: number;
}

// Relay interface
export interface Relay {
  id: number;
  name: string;
  state: boolean;
  pin: number;
  type: 'pump' | 'light' | 'extra' | 'other';
}

// Schedule interface
export interface Schedule {
  id: number;
  relayId: number;
  startTime: string; // HH:MM format
  endTime: string; // HH:MM format
  days: boolean[]; // Array of 7 booleans representing days of the week
  enabled: boolean;
}

// Settings interface
export interface HydroponicsSettings {
  phTarget: number;
  ecTarget: number;
  temperatureTarget: number;
  lightOnTime: string;
  lightOffTime: string;
  pumpCycleOnDuration: number;
  pumpCycleOffDuration: number;
}

/**
 * Fetch the latest sensor data from the ESP32
 */
export async function fetchSensorData(): Promise<SensorData> {
  const response = await fetch('/api/hydroponics/sensors');
  if (!response.ok) {
    throw new Error('Failed to fetch sensor data');
  }
  return await response.json();
}

/**
 * Fetch all relays from the ESP32
 */
export async function fetchRelays(): Promise<Relay[]> {
  const response = await fetch('/api/hydroponics/relays');
  if (!response.ok) {
    throw new Error('Failed to fetch relays');
  }
  return await response.json();
}

/**
 * Toggle a relay state
 */
export async function toggleRelay(id: number, state: boolean): Promise<void> {
  const response = await fetch(`/api/hydroponics/relays/${id}`, {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({ state })
  });
  
  if (!response.ok) {
    throw new Error('Failed to toggle relay');
  }
}

/**
 * Fetch all schedules from the ESP32
 */
export async function fetchSchedules(): Promise<Schedule[]> {
  const response = await fetch('/api/hydroponics/schedules');
  if (!response.ok) {
    throw new Error('Failed to fetch schedules');
  }
  return await response.json();
}

/**
 * Create a new schedule
 */
export async function createSchedule(schedule: Omit<Schedule, 'id'>): Promise<Schedule> {
  const response = await fetch('/api/hydroponics/schedules', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(schedule)
  });
  
  if (!response.ok) {
    throw new Error('Failed to create schedule');
  }
  
  return await response.json();
}

/**
 * Update an existing schedule
 */
export async function updateSchedule(schedule: Schedule): Promise<void> {
  const response = await fetch(`/api/hydroponics/schedules/${schedule.id}`, {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(schedule)
  });
  
  if (!response.ok) {
    throw new Error('Failed to update schedule');
  }
}

/**
 * Delete a schedule
 */
export async function deleteSchedule(id: number): Promise<void> {
  const response = await fetch(`/api/hydroponics/schedules/${id}`, {
    method: 'DELETE'
  });
  
  if (!response.ok) {
    throw new Error('Failed to delete schedule');
  }
}

/**
 * Fetch hydroponics settings
 */
export async function fetchSettings(): Promise<HydroponicsSettings> {
  const response = await fetch('/api/hydroponics/settings');
  if (!response.ok) {
    throw new Error('Failed to fetch settings');
  }
  return await response.json();
}

/**
 * Update hydroponics settings
 */
export async function updateSettings(settings: HydroponicsSettings): Promise<void> {
  const response = await fetch('/api/hydroponics/settings', {
    method: 'PUT',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(settings)
  });
  
  if (!response.ok) {
    throw new Error('Failed to update settings');
  }
} 