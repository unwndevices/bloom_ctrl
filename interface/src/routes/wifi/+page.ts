import type { PageLoad } from './$types';
import { goto } from '$app/navigation';

export const load = (async () => {
	goto('/connections/wifi');
	return;
}) satisfies PageLoad; 