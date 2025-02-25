import type { PageLoad } from './$types';

export const load: PageLoad = async ({ fetch, depends }) => {
	// This ID is used for reactivity when data changes
	depends('hydroponics:water-level');

	// In a real implementation, we would fetch water level data from the ESP32 here
	// For now, we'll just return an empty object
	return {
		title: 'Water Level'
	};
}; 