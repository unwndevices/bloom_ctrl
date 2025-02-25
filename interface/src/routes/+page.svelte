<script lang="ts">
	import type { PageData } from './$types';
	import logo from '$lib/assets/logo.png';
	import { notifications } from '$lib/components/toasts/notifications';

	export let data: PageData;
</script>

<div class="hero bg-base-100 h-screen">
	<div class="card md:card-side bg-base-200 shadow-primary shadow-2xl">
		<figure class="bg-base-200"><img src={logo} alt="Logo" class="h-auto w-64" /></figure>
		<div class="card-body w-80">
			<h2 class="card-title text-center text-2xl">Welcome to Bloom Control</h2>
			<p class="py-6 text-center">
				Advanced Hydroponic Control System using ESP32-sveltekit Framework. Monitor and control your
				hydroponic system with precision and reliability.
			</p>
			<a
				class="btn btn-primary"
				href="/hydroponics"
				on:click={() => notifications.success('Loading Hydroponic Dashboard', 1000)}
				>Open Dashboard</a
			>
		</div>
	</div>
</div>
