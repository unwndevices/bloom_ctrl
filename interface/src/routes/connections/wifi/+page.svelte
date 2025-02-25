<script lang="ts">
	import type { PageData } from './$types';
	import { goto } from '$app/navigation';
	import { onMount } from 'svelte';

	export let data: PageData;

	onMount(() => {
		// Redirect to the WiFi station page by default
		goto('/connections/wifi/sta');
	});
</script>

<div class="flex flex-col space-y-4 mx-0 my-1 sm:mx-8 sm:my-8">
	<h1 class="text-2xl font-bold">WiFi Settings</h1>
	<p>Redirecting to WiFi station settings...</p>
</div>
