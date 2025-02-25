import type { PageLoad } from './$types';

export const load: PageLoad = async ({ fetch, depends }) => {
	// This ID is used for reactivity when data changes
	depends('hydroponics:schedules');

	// In a real implementation, we would fetch schedule data from the ESP32 here
	// For now, we'll just return an empty object
	return {};
}; 