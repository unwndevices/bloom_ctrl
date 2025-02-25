<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import {
		fetchSensorData,
		fetchRelays,
		type SensorData,
		type Relay
	} from '$lib/services/hydroponics';

	let sensorData: SensorData | null = null;
	let relays: Relay[] = [];
	let loading = true;
	let error: string | null = null;

	// For refreshing data
	let interval: ReturnType<typeof setInterval>;

	// Load initial data
	onMount(async () => {
		try {
			await loadData();
			// Refresh data every 10 seconds
			interval = setInterval(loadData, 10000);
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
			const [sensorResponse, relaysResponse] = await Promise.all([
				fetchSensorData(),
				fetchRelays()
			]);

			sensorData = sensorResponse;
			relays = relaysResponse;
			error = null;
		} catch (err) {
			console.error('Error refreshing data:', err);
			error = 'Failed to refresh data';
		}
	}

	// Helper function to format date
	function formatDate(timestamp: number): string {
		return new Date(timestamp).toLocaleString();
	}

	// Helper function to get status class based on value
	function getStatusClass(value: number, min: number, max: number): string {
		if (value < min || value > max) return 'text-red-500';
		return 'text-green-500';
	}
</script>

<div class="dashboard">
	<h1 class="text-2xl font-bold mb-6">Hydroponics Dashboard</h1>

	{#if loading}
		<div class="loading">Loading system data...</div>
	{:else if error}
		<div class="error bg-red-100 p-4 rounded">
			<p class="text-red-700">{error}</p>
			<button class="btn mt-2" on:click={loadData}>Retry</button>
		</div>
	{:else}
		<div class="grid grid-cols-1 md:grid-cols-2 gap-6">
			<!-- Sensor Readings Panel -->
			<div class="card bg-white p-4 rounded shadow">
				<h2 class="text-xl font-semibold mb-4">Sensor Readings</h2>

				{#if sensorData}
					<div class="grid grid-cols-2 gap-4">
						<div class="sensor-item">
							<span class="label">Temperature</span>
							<span class={getStatusClass(sensorData.temperature, 18, 26)}>
								{sensorData.temperature.toFixed(1)}°C
							</span>
						</div>

						<div class="sensor-item">
							<span class="label">pH Level</span>
							<span class={getStatusClass(sensorData.ph, 5.5, 6.5)}>
								{sensorData.ph.toFixed(1)}
							</span>
						</div>

						<div class="sensor-item">
							<span class="label">EC</span>
							<span class={getStatusClass(sensorData.ec, 1.0, 2.0)}>
								{sensorData.ec.toFixed(2)} mS/cm
							</span>
						</div>

						<div class="sensor-item">
							<span class="label">Water Level</span>
							<span class={getStatusClass(sensorData.waterLevel, 70, 100)}>
								{sensorData.waterLevel}%
							</span>
						</div>

						<div class="sensor-item">
							<span class="label">Humidity</span>
							<span class={getStatusClass(sensorData.humidity, 40, 70)}>
								{sensorData.humidity.toFixed(1)}%
							</span>
						</div>

						<div class="sensor-item">
							<span class="label">Last Updated</span>
							<span class="text-gray-600">
								{formatDate(sensorData.timestamp)}
							</span>
						</div>
					</div>

					<div class="mt-4">
						<a href="/hydroponics/sensors" class="btn btn-sm">View Detailed Sensor Data</a>
					</div>
				{:else}
					<p>No sensor data available</p>
				{/if}
			</div>

			<!-- Relay Status Panel -->
			<div class="card bg-white p-4 rounded shadow">
				<h2 class="text-xl font-semibold mb-4">System Status</h2>

				{#if relays.length > 0}
					<div class="grid grid-cols-2 gap-4">
						{#each relays as relay}
							<div class="relay-item flex items-center">
								<span class={relay.state ? 'status-on' : 'status-off'}>●</span>
								<span class="ml-2">{relay.name}</span>
							</div>
						{/each}
					</div>

					<div class="mt-4">
						<a href="/hydroponics/relays" class="btn btn-sm">Manage Relays</a>
					</div>
				{:else}
					<p>No relay data available</p>
				{/if}
			</div>

			<!-- Quick Actions Panel -->
			<div class="card bg-white p-4 rounded shadow">
				<h2 class="text-xl font-semibold mb-4">Quick Actions</h2>

				<div class="grid grid-cols-2 gap-4">
					<a href="/hydroponics/schedules" class="btn"> Manage Schedules </a>
					<a href="/hydroponics/settings" class="btn"> System Settings </a>
				</div>
			</div>

			<!-- System Info Panel -->
			<div class="card bg-white p-4 rounded shadow">
				<h2 class="text-xl font-semibold mb-4">System Information</h2>

				<div class="grid grid-cols-1 gap-2">
					<div class="info-item">
						<span class="label">System Name:</span>
						<span>Bloom Control</span>
					</div>
					<div class="info-item">
						<span class="label">Firmware Version:</span>
						<span>1.0.0</span>
					</div>
					<div class="info-item">
						<span class="label">Uptime:</span>
						<span>3 days, 7 hours</span>
					</div>
					<div class="info-item">
						<span class="label">WiFi Signal:</span>
						<span>Strong (-42 dBm)</span>
					</div>
				</div>
			</div>
		</div>
	{/if}
</div>

<style>
	.sensor-item,
	.info-item {
		display: flex;
		flex-direction: column;
		margin-bottom: 0.5rem;
	}

	.label {
		font-weight: 500;
		color: #666;
		font-size: 0.9rem;
	}

	.status-on {
		color: #10b981;
		font-size: 1.2rem;
	}

	.status-off {
		color: #6b7280;
		font-size: 1.2rem;
	}

	.card {
		border: 1px solid #e5e7eb;
	}
</style>
