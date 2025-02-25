<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { fetchSensorData, type SensorData } from '$lib/services/hydroponics';
	import { daisyColor } from '$lib/DaisyUiHelper';
	import { notifications } from '$lib/components/toasts/notifications';

	let sensorData: SensorData | null = null;
	let loading = true;
	let error: string | null = null;

	// For refreshing data
	let interval: ReturnType<typeof setInterval>;

	// Mock historical data (in a real app, this would come from the API)
	let historicalData = {
		temperature: [23.5, 23.8, 24.0, 24.2, 24.5, 24.3, 24.1, 24.0, 23.9, 24.0],
		ph: [6.0, 6.1, 6.2, 6.2, 6.1, 6.0, 5.9, 6.0, 6.1, 6.2],
		ec: [1.7, 1.7, 1.8, 1.8, 1.9, 1.8, 1.8, 1.7, 1.7, 1.8],
		waterLevel: [85, 84, 83, 82, 81, 80, 79, 78, 77, 76],
		humidity: [55, 56, 57, 58, 57, 56, 55, 54, 55, 56],
		timestamps: Array.from({ length: 10 }, (_, i) => {
			const date = new Date();
			date.setHours(date.getHours() - (9 - i));
			return date.toLocaleTimeString();
		})
	};

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
			sensorData = await fetchSensorData();

			// In a real app, we would also fetch historical data here
			// For now, we'll just add the latest data point to our mock historical data
			if (sensorData) {
				historicalData.temperature.push(sensorData.temperature);
				historicalData.temperature.shift();

				historicalData.ph.push(sensorData.ph);
				historicalData.ph.shift();

				historicalData.ec.push(sensorData.ec);
				historicalData.ec.shift();

				historicalData.waterLevel.push(sensorData.waterLevel);
				historicalData.waterLevel.shift();

				historicalData.humidity.push(sensorData.humidity);
				historicalData.humidity.shift();

				const now = new Date().toLocaleTimeString();
				historicalData.timestamps.push(now);
				historicalData.timestamps.shift();
			}

			error = null;
		} catch (err) {
			console.error('Error refreshing data:', err);
			error = 'Failed to refresh data';
		}
	}

	// Helper function to get status class based on value
	function getStatusClass(value: number, min: number, max: number): string {
		if (value < min || value > max) return 'text-red-500';
		return 'text-green-500';
	}

	// Helper function to get status text based on value
	function getStatusText(value: number, min: number, max: number): string {
		if (value < min) return 'Too Low';
		if (value > max) return 'Too High';
		return 'Optimal';
	}

	// Placeholder for sensor calibration
	let calibrating = false;
	let calibrationType = '';
	let calibrationStep = 0;
	let calibrationValue = '';

	function startCalibration(type: string) {
		calibrating = true;
		calibrationType = type;
		calibrationStep = 1;
		calibrationValue = '';
		notifications.info(`Starting ${type} calibration`, 2000);
	}

	function nextCalibrationStep() {
		if (calibrationValue === '') {
			notifications.error('Please enter a calibration value', 2000);
			return;
		}

		calibrationStep++;
		if (calibrationStep > 2) {
			// In a real implementation, we would send the calibration data to the ESP32
			notifications.success(`${calibrationType} calibration completed`, 2000);
			calibrating = false;
			calibrationType = '';
			calibrationStep = 0;
			calibrationValue = '';
		} else {
			calibrationValue = '';
			notifications.info(`Proceeding to step ${calibrationStep}`, 2000);
		}
	}

	function cancelCalibration() {
		calibrating = false;
		calibrationType = '';
		calibrationStep = 0;
		calibrationValue = '';
		notifications.warning('Calibration cancelled', 2000);
	}
</script>

<div class="sensors-page">
	<h1 class="text-2xl font-bold mb-6">Sensor Readings</h1>

	{#if loading}
		<div class="loading">Loading sensor data...</div>
	{:else if error}
		<div class="error bg-red-100 p-4 rounded">
			<p class="text-red-700">{error}</p>
			<button class="btn mt-2" on:click={loadData}>Retry</button>
		</div>
	{:else if sensorData}
		<div class="grid grid-cols-1 md:grid-cols-2 gap-6">
			<!-- Temperature Sensor -->
			<div class="card bg-white p-4 rounded shadow">
				<h2 class="text-xl font-semibold mb-2">Temperature</h2>
				<div class="flex items-end justify-between mb-4">
					<div>
						<span class="text-3xl font-bold">{sensorData.temperature.toFixed(1)}</span>
						<span class="text-xl">°C</span>
					</div>
					<div class={getStatusClass(sensorData.temperature, 18, 26)}>
						{getStatusText(sensorData.temperature, 18, 26)}
					</div>
				</div>

				<div class="chart-container h-40 mb-2">
					<!-- In a real app, we would use a chart library here -->
					<div class="chart">
						<div class="chart-labels">
							{#each historicalData.timestamps as time, i}
								{#if i % 2 === 0}
									<div class="chart-label">{time}</div>
								{:else}
									<div class="chart-label"></div>
								{/if}
							{/each}
						</div>
						<div class="chart-bars">
							{#each historicalData.temperature as temp, i}
								<div
									class="chart-bar"
									style="height: {((temp - 15) / 15) * 100}%"
									title="{temp.toFixed(1)}°C at {historicalData.timestamps[i]}"
								></div>
							{/each}
						</div>
					</div>
				</div>

				<div class="text-sm text-gray-500">
					<p>Optimal range: 18°C - 26°C</p>
					<p>Last 10 hours of readings</p>
				</div>
			</div>

			<!-- pH Sensor -->
			<div class="card bg-white p-4 rounded shadow">
				<h2 class="text-xl font-semibold mb-2">pH Level</h2>
				<div class="flex items-end justify-between mb-4">
					<div>
						<span class="text-3xl font-bold">{sensorData.ph.toFixed(1)}</span>
					</div>
					<div class={getStatusClass(sensorData.ph, 5.5, 6.5)}>
						{getStatusText(sensorData.ph, 5.5, 6.5)}
					</div>
				</div>

				<div class="chart-container h-40 mb-2">
					<!-- In a real app, we would use a chart library here -->
					<div class="chart">
						<div class="chart-labels">
							{#each historicalData.timestamps as time, i}
								{#if i % 2 === 0}
									<div class="chart-label">{time}</div>
								{:else}
									<div class="chart-label"></div>
								{/if}
							{/each}
						</div>
						<div class="chart-bars">
							{#each historicalData.ph as ph, i}
								<div
									class="chart-bar"
									style="height: {((ph - 4) / 4) * 100}%"
									title="pH {ph.toFixed(1)} at {historicalData.timestamps[i]}"
								></div>
							{/each}
						</div>
					</div>
				</div>

				<div class="text-sm text-gray-500">
					<p>Optimal range: 5.5 - 6.5</p>
					<p>Last 10 hours of readings</p>
				</div>
			</div>

			<!-- EC Sensor -->
			<div class="card bg-white p-4 rounded shadow">
				<h2 class="text-xl font-semibold mb-2">EC (Electrical Conductivity)</h2>
				<div class="flex items-end justify-between mb-4">
					<div>
						<span class="text-3xl font-bold">{sensorData.ec.toFixed(2)}</span>
						<span class="text-xl">mS/cm</span>
					</div>
					<div class={getStatusClass(sensorData.ec, 1.0, 2.0)}>
						{getStatusText(sensorData.ec, 1.0, 2.0)}
					</div>
				</div>

				<div class="chart-container h-40 mb-2">
					<!-- In a real app, we would use a chart library here -->
					<div class="chart">
						<div class="chart-labels">
							{#each historicalData.timestamps as time, i}
								{#if i % 2 === 0}
									<div class="chart-label">{time}</div>
								{:else}
									<div class="chart-label"></div>
								{/if}
							{/each}
						</div>
						<div class="chart-bars">
							{#each historicalData.ec as ec, i}
								<div
									class="chart-bar"
									style="height: {(ec / 3) * 100}%"
									title="{ec.toFixed(2)} mS/cm at {historicalData.timestamps[i]}"
								></div>
							{/each}
						</div>
					</div>
				</div>

				<div class="text-sm text-gray-500">
					<p>Optimal range: 1.0 - 2.0 mS/cm</p>
					<p>Last 10 hours of readings</p>
				</div>
			</div>

			<!-- Water Level Sensor -->
			<div class="card bg-white p-4 rounded shadow">
				<h2 class="text-xl font-semibold mb-2">Water Level</h2>
				<div class="flex items-end justify-between mb-4">
					<div>
						<span class="text-3xl font-bold">{sensorData.waterLevel}</span>
						<span class="text-xl">%</span>
					</div>
					<div class={getStatusClass(sensorData.waterLevel, 70, 100)}>
						{getStatusText(sensorData.waterLevel, 70, 100)}
					</div>
				</div>

				<div class="chart-container h-40 mb-2">
					<!-- In a real app, we would use a chart library here -->
					<div class="chart">
						<div class="chart-labels">
							{#each historicalData.timestamps as time, i}
								{#if i % 2 === 0}
									<div class="chart-label">{time}</div>
								{:else}
									<div class="chart-label"></div>
								{/if}
							{/each}
						</div>
						<div class="chart-bars">
							{#each historicalData.waterLevel as level, i}
								<div
									class="chart-bar"
									style="height: {level}%"
									title="{level}% at {historicalData.timestamps[i]}"
								></div>
							{/each}
						</div>
					</div>
				</div>

				<div class="text-sm text-gray-500">
					<p>Optimal range: 70% - 100%</p>
					<p>Last 10 hours of readings</p>
				</div>
			</div>

			<!-- Humidity Sensor -->
			<div class="card bg-white p-4 rounded shadow">
				<h2 class="text-xl font-semibold mb-2">Humidity</h2>
				<div class="flex items-end justify-between mb-4">
					<div>
						<span class="text-3xl font-bold">{sensorData.humidity.toFixed(1)}</span>
						<span class="text-xl">%</span>
					</div>
					<div class={getStatusClass(sensorData.humidity, 40, 70)}>
						{getStatusText(sensorData.humidity, 40, 70)}
					</div>
				</div>

				<div class="chart-container h-40 mb-2">
					<!-- In a real app, we would use a chart library here -->
					<div class="chart">
						<div class="chart-labels">
							{#each historicalData.timestamps as time, i}
								{#if i % 2 === 0}
									<div class="chart-label">{time}</div>
								{:else}
									<div class="chart-label"></div>
								{/if}
							{/each}
						</div>
						<div class="chart-bars">
							{#each historicalData.humidity as humidity, i}
								<div
									class="chart-bar"
									style="height: {humidity}%"
									title="{humidity.toFixed(1)}% at {historicalData.timestamps[i]}"
								></div>
							{/each}
						</div>
					</div>
				</div>

				<div class="text-sm text-gray-500">
					<p>Optimal range: 40% - 70%</p>
					<p>Last 10 hours of readings</p>
				</div>
			</div>

			<!-- Last Updated -->
			<div class="card bg-white p-4 rounded shadow">
				<h2 class="text-xl font-semibold mb-2">Sensor Information</h2>
				<div class="grid grid-cols-1 gap-2">
					<div class="info-item">
						<span class="label">Last Updated:</span>
						<span>{new Date(sensorData.timestamp).toLocaleString()}</span>
					</div>
					<div class="info-item">
						<span class="label">Update Frequency:</span>
						<span>Every 10 seconds</span>
					</div>
					<div class="info-item">
						<span class="label">Sensor Status:</span>
						<span class="text-green-500">All sensors operational</span>
					</div>
				</div>
			</div>
		</div>
	{:else}
		<p>No sensor data available</p>
	{/if}
</div>

<style>
	.card {
		border: 1px solid #e5e7eb;
	}

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

	.chart {
		display: flex;
		flex-direction: column;
		height: 100%;
		width: 100%;
	}

	.chart-bars {
		display: flex;
		justify-content: space-between;
		align-items: flex-end;
		height: 100%;
		width: 100%;
	}

	.chart-bar {
		flex: 1;
		background-color: #3b82f6;
		margin: 0 1px;
		min-height: 1px;
		border-radius: 2px 2px 0 0;
	}

	.chart-labels {
		display: flex;
		justify-content: space-between;
		width: 100%;
		margin-top: 4px;
	}

	.chart-label {
		font-size: 0.7rem;
		color: #6b7280;
		transform: rotate(-45deg);
		transform-origin: top left;
		white-space: nowrap;
	}
</style>
