<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { fetchRelays, toggleRelay, type Relay } from '$lib/services/hydroponics';
	import { notifications } from '$lib/components/toasts/notifications';

	let relays: Relay[] = [];
	let loading = true;
	let error: string | null = null;

	// For refreshing data
	let interval: ReturnType<typeof setInterval>;

	// For editing relay name
	let editingRelay: Relay | null = null;
	let newRelayName = '';

	// Load initial data
	onMount(async () => {
		try {
			await loadData();
			// Refresh data every 5 seconds
			interval = setInterval(loadData, 5000);
		} catch (err) {
			error = 'Failed to load initial data';
			console.error(err);
		} finally {
			loading = false;
		}
	});

	onDestroy(() => {
		if (interval) clearInterval(interval);
	});

	async function loadData() {
		try {
			relays = await fetchRelays();
			error = null;
		} catch (err) {
			console.error('Error refreshing data:', err);
			error = 'Failed to refresh data';
		}
	}

	async function handleToggleRelay(relay: Relay) {
		try {
			await toggleRelay(relay.id, !relay.state);
			// Update the local state immediately for better UX
			relays = relays.map((r) => (r.id === relay.id ? { ...r, state: !r.state } : r));
		} catch (err) {
			console.error('Error toggling relay:', err);
			error = 'Failed to toggle relay';
		}
	}

	function startEditingRelay(relay: Relay) {
		editingRelay = relay;
		newRelayName = relay.name;
	}

	function cancelEditingRelay() {
		editingRelay = null;
		newRelayName = '';
	}

	async function saveRelayName() {
		if (!editingRelay || !newRelayName.trim()) return;

		try {
			// In a real app, we would call an API to update the relay name
			// For now, we'll just update the local state
			relays = relays.map((r) =>
				r.id === editingRelay!.id ? { ...r, name: newRelayName.trim() } : r
			);

			editingRelay = null;
			newRelayName = '';
		} catch (err) {
			console.error('Error updating relay name:', err);
			error = 'Failed to update relay name';
		}
	}

	// Helper function to get relay type icon
	function getRelayTypeIcon(type: string): string {
		switch (type) {
			case 'pump':
				return '💧';
			case 'light':
				return '💡';
			case 'fan':
				return '🌀';
			case 'nutrient':
				return '🧪';
			default:
				return '⚙️';
		}
	}

	// Function to manually activate a relay for a specific duration
	async function activateRelayForDuration(id: number, duration: number) {
		const relay = relays.find((r) => r.id === id);
		if (relay) {
			if (!relay.state) {
				await toggleRelay(id, true);
				// Update local state
				relays = relays.map((r) => (r.id === id ? { ...r, state: true } : r));
			}

			notifications.info(`${relay.name} activated for ${duration} seconds`, 2000);

			// In a real implementation, we would send this command to the ESP32
			// and the ESP32 would handle the timing and turning off the relay
			setTimeout(async () => {
				const currentRelay = relays.find((r) => r.id === id);
				if (currentRelay && currentRelay.state) {
					await toggleRelay(id, false);
					// Update local state
					relays = relays.map((r) => (r.id === id ? { ...r, state: false } : r));
					notifications.info(
						`${relay.name} automatically turned OFF after ${duration} seconds`,
						2000
					);
				}
			}, duration * 1000);
		}
	}

	onMount(() => {
		// In a real implementation, we would set up WebSocket connections
		// to receive real-time updates from the ESP32
		notifications.success('Relay control page loaded successfully', 2000);
	});
</script>

<div class="relays-page">
	<h1 class="text-2xl font-bold mb-6">Relay Control</h1>

	{#if loading}
		<div class="loading">Loading relay data...</div>
	{:else if error}
		<div class="error bg-red-100 p-4 rounded">
			<p class="text-red-700">{error}</p>
			<button class="btn mt-2" on:click={loadData}>Retry</button>
		</div>
	{:else if relays.length > 0}
		<div class="grid grid-cols-1 md:grid-cols-2 gap-6">
			{#each relays as relay}
				<div class="card bg-white p-4 rounded shadow">
					<div class="flex justify-between items-center mb-4">
						<div class="flex items-center">
							<span class="text-2xl mr-2">{getRelayTypeIcon(relay.type)}</span>
							{#if editingRelay && editingRelay.id === relay.id}
								<input
									type="text"
									class="input border border-gray-300 rounded px-2 py-1"
									bind:value={newRelayName}
									on:keydown={(e) => e.key === 'Enter' && saveRelayName()}
								/>
							{:else}
								<h2 class="text-xl font-semibold">{relay.name}</h2>
							{/if}
						</div>

						{#if editingRelay && editingRelay.id === relay.id}
							<div class="flex gap-2">
								<button class="btn btn-sm bg-green-500 text-white" on:click={saveRelayName}>
									Save
								</button>
								<button class="btn btn-sm bg-gray-300" on:click={cancelEditingRelay}>
									Cancel
								</button>
							</div>
						{:else}
							<button class="btn btn-sm bg-gray-200" on:click={() => startEditingRelay(relay)}>
								Edit
							</button>
						{/if}
					</div>

					<div class="flex justify-between items-center">
						<div>
							<p class="text-gray-600">Pin: GPIO{relay.pin}</p>
							<p class="text-gray-600">Type: {relay.type}</p>
						</div>

						<div class="flex items-center">
							<span class="mr-2">{relay.state ? 'ON' : 'OFF'}</span>
							<button
								class="toggle-btn {relay.state ? 'bg-green-500' : 'bg-gray-300'}"
								on:click={() => handleToggleRelay(relay)}
							>
								<span class="toggle-slider {relay.state ? 'translate-x-5' : 'translate-x-0'}"
								></span>
							</button>
						</div>
					</div>
				</div>
			{/each}
		</div>
	{:else}
		<p>No relays configured</p>
	{/if}
</div>

<style>
	.card {
		border: 1px solid #e5e7eb;
	}

	.toggle-btn {
		position: relative;
		display: inline-block;
		width: 48px;
		height: 24px;
		border-radius: 12px;
		transition: background-color 0.3s;
	}

	.toggle-slider {
		position: absolute;
		top: 2px;
		left: 2px;
		width: 20px;
		height: 20px;
		background-color: white;
		border-radius: 50%;
		transition: transform 0.3s;
	}
</style>
