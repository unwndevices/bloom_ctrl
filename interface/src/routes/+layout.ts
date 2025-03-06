import type { LayoutLoad } from './$types';

// This can be false if you're using a fallback (i.e. SPA mode)
export const prerender = false;
export const ssr = false;

export const load = (async ({ fetch }) => {
	const result = await fetch('/rest/features');
	const item = await result.json();
	return {
		features: item,
		title: 'Bloom by Greenverse',
		github: 'unwndevices/bloom_ctrl',
		copyright: '2025 Greenverse',
		appName: 'Bloom Control'
	};
}) satisfies LayoutLoad;
