import type { PageLoad } from './$types';
import { goto } from '$app/navigation';

export const load = (async () => {
	goto('/connections/wifi/ap');
	return;
}) satisfies PageLoad; 