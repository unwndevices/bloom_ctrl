<script lang="ts">
	import { onMount } from 'svelte';
	import { notifications } from '$lib/components/toasts/notifications';
	import { daisyColor } from '$lib/DaisyUiHelper';

	// Placeholder data for water level
	let currentLevel = 75; // percentage
	let optimalRange = { min: 70, max: 90 };
	let criticalLow = 30;
	let warningLow = 50;
	let warningHigh = 90;
	let criticalHigh = 95;

	// Placeholder for historical data
	let historicalData = [65, 68, 72, 75, 78, 80, 82, 79, 77, 75, 73, 75];
	let timeLabels = [
		'12:00',
		'13:00',
		'14:00',
		'15:00',
		'16:00',
		'17:00',
		'18:00',
		'19:00',
		'20:00',
		'21:00',
		'22:00',
		'23:00'
	];

	// Placeholder for alert settings
	let alertLowLevel = 40;
	let alertHighLevel = 90;
	let alertsEnabled = true;
	let autoRefillEnabled = true;
	let autoRefillThreshold = 45;
	let autoRefillTarget = 85;
	let autoRefillInProgress = false;

	// Placeholder for calibration
	let isCalibrating = false;
	let calibrationStep = 0;
	let emptyValue = 0;
	let fullValue = 100;

	// Function to get status color based on water level
	function getStatusColor(level: number): string {
		if (level <= criticalLow || level >= criticalHigh) {
			return daisyColor('error');
		} else if (level <= warningLow || level >= warningHigh) {
			return daisyColor('warning');
		} else if (level >= optimalRange.min && level <= optimalRange.max) {
			return daisyColor('success');
		} else {
			return daisyColor('info');
		}
	}

	// Function to get status text based on water level
	function getStatusText(level: number): string {
		if (level <= criticalLow) {
			return 'Critical Low';
		} else if (level >= criticalHigh) {
			return 'Critical High';
		} else if (level <= warningLow) {
			return 'Low';
		} else if (level >= warningHigh) {
			return 'High';
		} else if (level >= optimalRange.min && level <= optimalRange.max) {
			return 'Optimal';
		} else {
			return 'Normal';
		}
	}

	// Function to start calibration
	function startCalibration() {
		isCalibrating = true;
		calibrationStep = 1;
		notifications.info('Starting water level sensor calibration', 2000);
	}

	// Function to proceed to next calibration step
	function nextCalibrationStep() {
		if (calibrationStep === 1) {
			// Save empty value
			emptyValue = 0;
			calibrationStep = 2;
			notifications.info('Empty level calibrated. Please fill the reservoir.', 2000);
		} else if (calibrationStep === 2) {
			// Save full value
			fullValue = 100;
			calibrationStep = 0;
			isCalibrating = false;
			notifications.success('Calibration completed successfully', 2000);
		}
	}

	// Function to cancel calibration
	function cancelCalibration() {
		isCalibrating = false;
		calibrationStep = 0;
		notifications.info('Calibration cancelled', 2000);
	}

	// Function to save alert settings
	function saveAlertSettings() {
		if (alertLowLevel >= alertHighLevel) {
			notifications.error('Low level alert must be lower than high level alert', 2000);
			return;
		}

		if (autoRefillThreshold >= autoRefillTarget) {
			notifications.error('Auto-refill threshold must be lower than target level', 2000);
			return;
		}

		notifications.success('Water level alert settings saved', 2000);
	}

	// Function to manually trigger refill
	function startManualRefill() {
		if (currentLevel >= autoRefillTarget) {
			notifications.error('Water level is already at or above target level', 2000);
			return;
		}

		autoRefillInProgress = true;
		notifications.info('Manual refill started', 2000);

		// Simulate refill process
		const interval = setInterval(() => {
			currentLevel += 5;
			if (currentLevel >= autoRefillTarget) {
				clearInterval(interval);
				currentLevel = autoRefillTarget;
				autoRefillInProgress = false;
				notifications.success('Refill completed', 2000);
			}
		}, 1000);
	}

	// Function to stop manual refill
	function stopManualRefill() {
		autoRefillInProgress = false;
		notifications.info('Refill stopped', 2000);
	}

	onMount(() => {
		// In a real implementation, we would set up WebSocket connections
		// to receive real-time updates from the ESP32
		notifications.success('Water level monitoring page loaded successfully', 2000);
	});
</script>

<div class="p-4">
	<h1 class="text-2xl font-bold mb-4">Water Level Monitoring</h1>

	<!-- Current Water Level -->
	<div class="card bg-base-200 shadow-xl mb-6">
		<div class="card-body">
			<h2 class="card-title mb-4">Current Water Level</h2>
			<div class="grid grid-cols-1 md:grid-cols-2 gap-6">
				<div class="flex flex-col items-center justify-center">
					<div class="relative w-40 h-64 border-2 border-base-content rounded-lg overflow-hidden">
						<div
							class="absolute bottom-0 left-0 right-0 bg-primary transition-all duration-1000 ease-in-out"
							style="height: {currentLevel}%; background-color: {getStatusColor(currentLevel)}"
						></div>
						<div
							class="absolute bottom-0 left-0 right-0 border-t-2 border-dashed border-base-content opacity-50"
							style="bottom: {optimalRange.min}%"
						></div>
						<div
							class="absolute bottom-0 left-0 right-0 border-t-2 border-dashed border-base-content opacity-50"
							style="bottom: {optimalRange.max}%"
						></div>
						<div
							class="absolute inset-0 flex items-center justify-center text-3xl font-bold text-base-content"
						>
							{currentLevel}%
						</div>
					</div>
					<div class="mt-4 text-center">
						<span
							class="badge badge-lg"
							style="background-color: {getStatusColor(currentLevel)}; color: white"
						>
							{getStatusText(currentLevel)}
						</span>
					</div>
				</div>

				<div>
					<div class="stats shadow w-full">
						<div class="stat">
							<div class="stat-title">Current Level</div>
							<div class="stat-value">{currentLevel}%</div>
							<div class="stat-desc">Optimal range: {optimalRange.min}% - {optimalRange.max}%</div>
						</div>
					</div>

					<div class="mt-4">
						<h3 class="font-bold mb-2">Status Indicators</h3>
						<div class="grid grid-cols-2 gap-2">
							<div class="flex items-center gap-2">
								<div
									class="w-4 h-4 rounded-full"
									style="background-color: {daisyColor('error')}"
								></div>
								<span>Critical: &lt;{criticalLow}% or &gt;{criticalHigh}%</span>
							</div>
							<div class="flex items-center gap-2">
								<div
									class="w-4 h-4 rounded-full"
									style="background-color: {daisyColor('warning')}"
								></div>
								<span>Warning: &lt;{warningLow}% or &gt;{warningHigh}%</span>
							</div>
							<div class="flex items-center gap-2">
								<div
									class="w-4 h-4 rounded-full"
									style="background-color: {daisyColor('success')}"
								></div>
								<span>Optimal: {optimalRange.min}% - {optimalRange.max}%</span>
							</div>
							<div class="flex items-center gap-2">
								<div
									class="w-4 h-4 rounded-full"
									style="background-color: {daisyColor('info')}"
								></div>
								<span>Normal: All other levels</span>
							</div>
						</div>
					</div>

					<div class="mt-6">
						<h3 class="font-bold mb-2">Quick Actions</h3>
						<div class="flex gap-2">
							<button
								class="btn btn-primary {autoRefillInProgress ? 'btn-disabled' : ''}"
								on:click={startManualRefill}
								disabled={autoRefillInProgress || currentLevel >= autoRefillTarget}
							>
								Start Manual Refill
							</button>
							<button
								class="btn btn-error {!autoRefillInProgress ? 'btn-disabled' : ''}"
								on:click={stopManualRefill}
								disabled={!autoRefillInProgress}
							>
								Stop Refill
							</button>
							<button class="btn btn-outline" on:click={startCalibration}>Calibrate Sensor</button>
						</div>
					</div>
				</div>
			</div>
		</div>
	</div>

	<!-- Historical Data -->
	<div class="card bg-base-200 shadow-xl mb-6">
		<div class="card-body">
			<h2 class="card-title mb-4">Historical Water Level</h2>
			<div class="h-64 w-full bg-base-300 rounded-lg p-4 flex items-end justify-between">
				{#each historicalData as level, i}
					<div class="flex flex-col items-center">
						<div
							class="w-8 bg-primary rounded-t-sm transition-all duration-300"
							style="height: {level * 0.5}%; background-color: {getStatusColor(level)}"
						></div>
						<div class="text-xs mt-1 rotate-45 origin-left">{timeLabels[i]}</div>
					</div>
				{/each}
			</div>
			<div class="flex justify-between mt-2">
				<button class="btn btn-sm btn-outline">Last 12 Hours</button>
				<button class="btn btn-sm btn-outline">Last 24 Hours</button>
				<button class="btn btn-sm btn-outline">Last 7 Days</button>
				<button class="btn btn-sm btn-outline">Last 30 Days</button>
			</div>
		</div>
	</div>

	<!-- Settings -->
	<div class="card bg-base-200 shadow-xl mb-6">
		<div class="card-body">
			<h2 class="card-title mb-4">Water Level Settings</h2>
			<div class="grid grid-cols-1 md:grid-cols-2 gap-6">
				<div>
					<h3 class="font-bold mb-2">Alert Settings</h3>
					<div class="form-control mb-4">
						<label class="label cursor-pointer">
							<span class="label-text">Enable Alerts</span>
							<input type="checkbox" class="toggle toggle-primary" bind:checked={alertsEnabled} />
						</label>
					</div>
					<div class="form-control mb-4">
						<label class="label">
							<span class="label-text">Low Level Alert (%)</span>
						</label>
						<input
							type="range"
							min="0"
							max="100"
							class="range range-primary"
							bind:value={alertLowLevel}
						/>
						<div class="flex justify-between text-xs px-2">
							<span>0%</span>
							<span>25%</span>
							<span>50%</span>
							<span>75%</span>
							<span>100%</span>
						</div>
						<div class="text-center mt-1">{alertLowLevel}%</div>
					</div>
					<div class="form-control mb-4">
						<label class="label">
							<span class="label-text">High Level Alert (%)</span>
						</label>
						<input
							type="range"
							min="0"
							max="100"
							class="range range-primary"
							bind:value={alertHighLevel}
						/>
						<div class="flex justify-between text-xs px-2">
							<span>0%</span>
							<span>25%</span>
							<span>50%</span>
							<span>75%</span>
							<span>100%</span>
						</div>
						<div class="text-center mt-1">{alertHighLevel}%</div>
					</div>
				</div>

				<div>
					<h3 class="font-bold mb-2">Auto-Refill Settings</h3>
					<div class="form-control mb-4">
						<label class="label cursor-pointer">
							<span class="label-text">Enable Auto-Refill</span>
							<input
								type="checkbox"
								class="toggle toggle-primary"
								bind:checked={autoRefillEnabled}
							/>
						</label>
					</div>
					<div class="form-control mb-4">
						<label class="label">
							<span class="label-text">Refill Threshold (%)</span>
						</label>
						<input
							type="range"
							min="0"
							max="100"
							class="range range-primary"
							bind:value={autoRefillThreshold}
						/>
						<div class="flex justify-between text-xs px-2">
							<span>0%</span>
							<span>25%</span>
							<span>50%</span>
							<span>75%</span>
							<span>100%</span>
						</div>
						<div class="text-center mt-1">{autoRefillThreshold}%</div>
					</div>
					<div class="form-control mb-4">
						<label class="label">
							<span class="label-text">Refill Target (%)</span>
						</label>
						<input
							type="range"
							min="0"
							max="100"
							class="range range-primary"
							bind:value={autoRefillTarget}
						/>
						<div class="flex justify-between text-xs px-2">
							<span>0%</span>
							<span>25%</span>
							<span>50%</span>
							<span>75%</span>
							<span>100%</span>
						</div>
						<div class="text-center mt-1">{autoRefillTarget}%</div>
					</div>
				</div>
			</div>
			<div class="flex justify-end mt-4">
				<button class="btn btn-primary" on:click={saveAlertSettings}>Save Settings</button>
			</div>
		</div>
	</div>

	<!-- Calibration Modal -->
	{#if isCalibrating}
		<div class="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
			<div class="bg-base-200 p-6 rounded-lg w-full max-w-md">
				<h3 class="text-xl font-bold mb-4">Water Level Sensor Calibration</h3>

				{#if calibrationStep === 1}
					<div class="mb-4">
						<p class="mb-4">
							Step 1: Empty the reservoir completely or to the minimum desired level.
						</p>
						<div class="alert alert-info">
							<svg
								xmlns="http://www.w3.org/2000/svg"
								fill="none"
								viewBox="0 0 24 24"
								class="stroke-current shrink-0 w-6 h-6"
							>
								<path
									stroke-linecap="round"
									stroke-linejoin="round"
									stroke-width="2"
									d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"
								></path>
							</svg>
							<span>This will calibrate the 0% (empty) level of your system.</span>
						</div>
					</div>
				{:else if calibrationStep === 2}
					<div class="mb-4">
						<p class="mb-4">Step 2: Fill the reservoir to the maximum desired level.</p>
						<div class="alert alert-info">
							<svg
								xmlns="http://www.w3.org/2000/svg"
								fill="none"
								viewBox="0 0 24 24"
								class="stroke-current shrink-0 w-6 h-6"
							>
								<path
									stroke-linecap="round"
									stroke-linejoin="round"
									stroke-width="2"
									d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"
								></path>
							</svg>
							<span>This will calibrate the 100% (full) level of your system.</span>
						</div>
					</div>
				{/if}

				<div class="flex justify-end gap-2">
					<button class="btn btn-outline" on:click={cancelCalibration}>Cancel</button>
					<button class="btn btn-primary" on:click={nextCalibrationStep}>
						{calibrationStep === 1 ? 'Next Step' : 'Complete Calibration'}
					</button>
				</div>
			</div>
		</div>
	{/if}
</div>
