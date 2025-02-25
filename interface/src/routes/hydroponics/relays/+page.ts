import type { PageLoad } from './$types';

export const load: PageLoad = async ({ fetch, depends }) => {
	// This ID is used for reactivity when data changes
	depends('hydroponics:relays');

	// In a real implementation, we would fetch relay data from the ESP32 here
	// For now, we'll just return an empty object
	return {
		title: 'Relays'
	};
}; 