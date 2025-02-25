<script lang="ts">
	import { onMount } from 'svelte';
	import { notifications } from '$lib/components/toasts/notifications';
	import {
		fetchRelays,
		fetchSchedules,
		createSchedule,
		updateSchedule,
		deleteSchedule
	} from '$lib/services/hydroponics';

	// Define types
	type Relay = {
		id: number;
		name: string;
		state: boolean;
		pin: number;
		type: 'pump' | 'light' | 'fan' | 'nutrient' | 'other';
	};

	type Schedule = {
		id: number;
		relayId: number;
		startTime: string; // HH:MM format
		endTime: string; // HH:MM format
		days: boolean[]; // Array of 7 booleans representing days of the week
		enabled: boolean;
	};

	// Days of the week
	const daysOfWeek = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];

	// State variables
	let relays: Relay[] = [];
	let schedules: Schedule[] = [];
	let loading = true;
	let error: string | null = null;

	// For editing schedule
	let editingSchedule: Schedule | null = null;
	let editingRelayId: number | null = null;
	let newSchedule: Omit<Schedule, 'id'> = {
		relayId: 0,
		startTime: '06:00',
		endTime: '18:00',
		days: [true, true, true, true, true, true, true],
		enabled: true
	};

	// Time slider values (in minutes from midnight)
	let sliderStartValue = 360; // 6:00 AM in minutes
	let sliderEndValue = 1080; // 18:00 (6:00 PM) in minutes

	// Day selection state
	let daySelections = [true, true, true, true, true, true, true];
	let enabledSelection = true;

	// Load initial data
	onMount(async () => {
		try {
			await loadData();
		} catch (err) {
			error = 'Failed to load initial data';
			console.error(err);
		} finally {
			loading = false;
		}
	});

	async function loadData() {
		try {
			[relays, schedules] = await Promise.all([fetchRelays(), fetchSchedules()]);
			error = null;
		} catch (err) {
			console.error('Error loading data:', err);
			error = 'Failed to load data';
		}
	}

	// Convert HH:MM to minutes from midnight
	function timeToMinutes(time: string): number {
		const [hours, minutes] = time.split(':').map(Number);
		return hours * 60 + minutes;
	}

	// Convert minutes from midnight to HH:MM
	function minutesToTime(minutes: number): string {
		// Round to nearest 15 minutes
		minutes = Math.round(minutes / 15) * 15;

		const hours = Math.floor(minutes / 60);
		const mins = minutes % 60;
		return `${hours.toString().padStart(2, '0')}:${mins.toString().padStart(2, '0')}`;
	}

	// Update time values when slider changes
	function handleSliderChange() {
		// Ensure start is before end
		if (sliderStartValue >= sliderEndValue) {
			// If they're too close, keep a minimum 15 min difference
			sliderEndValue = sliderStartValue + 15;
		}

		// Update the time strings
		if (editingSchedule) {
			editingSchedule.startTime = minutesToTime(sliderStartValue);
			editingSchedule.endTime = minutesToTime(sliderEndValue);
		} else {
			newSchedule.startTime = minutesToTime(sliderStartValue);
			newSchedule.endTime = minutesToTime(sliderEndValue);
		}
	}

	// Start creating a new schedule for a relay
	function startNewSchedule(relayId: number) {
		editingSchedule = null;
		editingRelayId = relayId;
		newSchedule = {
			relayId: relayId,
			startTime: '06:00',
			endTime: '18:00',
			days: [true, true, true, true, true, true, true],
			enabled: true
		};
		sliderStartValue = timeToMinutes('06:00');
		sliderEndValue = timeToMinutes('18:00');
		daySelections = [...newSchedule.days];
		enabledSelection = newSchedule.enabled;
	}

	// Start editing an existing schedule
	function startEditSchedule(schedule: Schedule) {
		editingSchedule = { ...schedule };
		editingRelayId = schedule.relayId;
		sliderStartValue = timeToMinutes(schedule.startTime);
		sliderEndValue = timeToMinutes(schedule.endTime);
		daySelections = [...schedule.days];
		enabledSelection = schedule.enabled;
	}

	// Cancel editing/creating
	function cancelEdit() {
		editingSchedule = null;
		editingRelayId = null;
	}

	// Check if a new schedule overlaps with existing ones
	function hasOverlap(schedule: Schedule, scheduleId?: number): boolean {
		const startMinutes = timeToMinutes(schedule.startTime);
		const endMinutes = timeToMinutes(schedule.endTime);

		return schedules.some((existingSchedule) => {
			// Skip the current schedule being edited
			if (scheduleId && existingSchedule.id === scheduleId) return false;

			// Only check schedules for the same relay
			if (existingSchedule.relayId !== schedule.relayId) return false;

			// Check if any day overlaps
			const hasCommonDay = schedule.days.some((day, index) => day && existingSchedule.days[index]);
			if (!hasCommonDay) return false;

			// Check time overlap
			const existingStart = timeToMinutes(existingSchedule.startTime);
			const existingEnd = timeToMinutes(existingSchedule.endTime);

			return (
				(startMinutes >= existingStart && startMinutes < existingEnd) ||
				(endMinutes > existingStart && endMinutes <= existingEnd) ||
				(startMinutes <= existingStart && endMinutes >= existingEnd)
			);
		});
	}

	// Save a schedule (create new or update existing)
	async function saveSchedule() {
		try {
			// Apply the current day and enabled selections
			if (editingSchedule) {
				editingSchedule.days = [...daySelections];
				editingSchedule.enabled = enabledSelection;
			} else {
				newSchedule.days = [...daySelections];
				newSchedule.enabled = enabledSelection;
			}

			const scheduleToSave = editingSchedule
				? { ...editingSchedule }
				: ({ ...newSchedule } as Schedule);

			// Calculate duration from start and end time
			const startMinutes = timeToMinutes(scheduleToSave.startTime);
			const endMinutes = timeToMinutes(scheduleToSave.endTime);

			// Check for time validity
			if (startMinutes >= endMinutes) {
				notifications.error('End time must be after start time', 3000);
				return;
			}

			// Check if any day is selected
			if (!scheduleToSave.days.some((day) => day)) {
				notifications.error('At least one day must be selected', 3000);
				return;
			}

			// Check for overlaps
			if (hasOverlap(scheduleToSave, editingSchedule?.id)) {
				notifications.error(
					'This schedule overlaps with an existing schedule for this relay',
					3000
				);
				return;
			}

			if (editingSchedule) {
				// Update existing schedule
				await updateSchedule(scheduleToSave);
				schedules = schedules.map((s) => (s.id === scheduleToSave.id ? scheduleToSave : s));
				notifications.success('Schedule updated successfully', 2000);
			} else {
				// Create new schedule
				const createdSchedule = await createSchedule(scheduleToSave);
				schedules = [...schedules, createdSchedule];
				notifications.success('New schedule created', 2000);
			}

			// Reset editing state
			editingSchedule = null;
			editingRelayId = null;
		} catch (err) {
			console.error('Error saving schedule:', err);
			notifications.error('Failed to save schedule', 3000);
		}
	}

	// Delete a schedule
	async function removeSchedule(scheduleId: number) {
		try {
			if (confirm('Are you sure you want to delete this schedule?')) {
				await deleteSchedule(scheduleId);
				schedules = schedules.filter((s) => s.id !== scheduleId);
				notifications.info('Schedule deleted', 2000);
			}
		} catch (err) {
			console.error('Error deleting schedule:', err);
			notifications.error('Failed to delete schedule', 3000);
		}
	}

	// Toggle schedule enabled state
	async function toggleScheduleEnabled(schedule: Schedule) {
		try {
			const updatedSchedule = { ...schedule, enabled: !schedule.enabled };
			await updateSchedule(updatedSchedule);
			schedules = schedules.map((s) => (s.id === schedule.id ? updatedSchedule : s));
			notifications.info(`Schedule ${updatedSchedule.enabled ? 'enabled' : 'disabled'}`, 2000);
		} catch (err) {
			console.error('Error toggling schedule:', err);
			notifications.error('Failed to update schedule', 3000);
		}
	}

	// Helper function to get schedules for a specific relay
	function getRelaySchedules(relayId: number): Schedule[] {
		return schedules.filter((schedule) => schedule.relayId === relayId);
	}

	// Format time range for display
	function formatTimeRange(startTime: string, endTime: string): string {
		return `${startTime} - ${endTime}`;
	}

	// Format days for display
	function formatDays(days: boolean[]): string {
		const activeDays = days
			.map((active, index) => (active ? daysOfWeek[index] : null))
			.filter((day) => day !== null);

		if (activeDays.length === 7) {
			return 'Every day';
		} else if (activeDays.length === 0) {
			return 'Never';
		} else if (days[0] && days[1] && days[2] && days[3] && days[4] && !days[5] && !days[6]) {
			return 'Weekdays';
		} else if (!days[0] && !days[1] && !days[2] && !days[3] && !days[4] && days[5] && days[6]) {
			return 'Weekends';
		} else {
			return activeDays.join(', ');
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

	// Set day presets
	function setDayPreset(preset: 'all' | 'weekdays' | 'weekends' | 'none') {
		if (preset === 'all') {
			daySelections = [true, true, true, true, true, true, true];
		} else if (preset === 'weekdays') {
			daySelections = [true, true, true, true, true, false, false];
		} else if (preset === 'weekends') {
			daySelections = [false, false, false, false, false, true, true];
		} else {
			daySelections = [false, false, false, false, false, false, false];
		}
	}

	// Handle day toggle
	function handleDayToggle(index: number) {
		daySelections[index] = !daySelections[index];
		daySelections = [...daySelections]; // Trigger reactivity
	}

	// Handle dragging the start thumb
	function handleStartThumbDrag(event: MouseEvent | TouchEvent) {
		event.preventDefault();

		const isTouch = 'touches' in event;
		const startX = isTouch
			? (event as TouchEvent).touches[0].clientX
			: (event as MouseEvent).clientX;
		const sliderRect = (event.currentTarget as HTMLElement).parentElement?.getBoundingClientRect();

		if (!sliderRect) return;

		const initialValue = sliderStartValue;
		const sliderWidth = sliderRect.width;

		function onMove(moveEvent: MouseEvent | TouchEvent) {
			const moveX =
				'touches' in moveEvent
					? (moveEvent as TouchEvent).touches[0].clientX
					: (moveEvent as MouseEvent).clientX;

			const deltaX = moveX - startX;
			const deltaRatio = deltaX / sliderWidth;
			const deltaMinutes = Math.round((deltaRatio * 1440) / 15) * 15; // Snap to 15 min intervals

			let newValue = initialValue + deltaMinutes;
			newValue = Math.max(0, Math.min(sliderEndValue - 15, newValue)); // Ensure it doesn't go beyond end value

			sliderStartValue = newValue;
			handleSliderChange();
		}

		function onEnd() {
			document.removeEventListener('mousemove', onMove);
			document.removeEventListener('touchmove', onMove);
			document.removeEventListener('mouseup', onEnd);
			document.removeEventListener('touchend', onEnd);
		}

		document.addEventListener('mousemove', onMove);
		document.addEventListener('touchmove', onMove);
		document.addEventListener('mouseup', onEnd);
		document.addEventListener('touchend', onEnd);
	}

	// Handle dragging the end thumb
	function handleEndThumbDrag(event: MouseEvent | TouchEvent) {
		event.preventDefault();

		const isTouch = 'touches' in event;
		const startX = isTouch
			? (event as TouchEvent).touches[0].clientX
			: (event as MouseEvent).clientX;
		const sliderRect = (event.currentTarget as HTMLElement).parentElement?.getBoundingClientRect();

		if (!sliderRect) return;

		const initialValue = sliderEndValue;
		const sliderWidth = sliderRect.width;

		function onMove(moveEvent: MouseEvent | TouchEvent) {
			const moveX =
				'touches' in moveEvent
					? (moveEvent as TouchEvent).touches[0].clientX
					: (moveEvent as MouseEvent).clientX;

			const deltaX = moveX - startX;
			const deltaRatio = deltaX / sliderWidth;
			const deltaMinutes = Math.round((deltaRatio * 1440) / 15) * 15; // Snap to 15 min intervals

			let newValue = initialValue + deltaMinutes;
			newValue = Math.max(sliderStartValue + 15, Math.min(1440, newValue)); // Ensure it doesn't go below start value

			sliderEndValue = newValue;
			handleSliderChange();
		}

		function onEnd() {
			document.removeEventListener('mousemove', onMove);
			document.removeEventListener('touchmove', onMove);
			document.removeEventListener('mouseup', onEnd);
			document.removeEventListener('touchend', onEnd);
		}

		document.addEventListener('mousemove', onMove);
		document.addEventListener('touchmove', onMove);
		document.addEventListener('mouseup', onEnd);
		document.addEventListener('touchend', onEnd);
	}
</script>

<div class="p-4">
	<h1 class="text-2xl font-bold mb-4">Automation Schedules</h1>

	{#if loading}
		<div class="loading-spinner flex justify-center my-8">
			<div class="animate-spin rounded-full h-12 w-12 border-t-2 border-b-2 border-primary"></div>
		</div>
	{:else if error}
		<div class="alert alert-error shadow-lg mb-6">
			<div>
				<svg
					xmlns="http://www.w3.org/2000/svg"
					class="stroke-current flex-shrink-0 h-6 w-6"
					fill="none"
					viewBox="0 0 24 24"
					><path
						stroke-linecap="round"
						stroke-linejoin="round"
						stroke-width="2"
						d="M10 14l2-2m0 0l2-2m-2 2l-2-2m2 2l2 2m7-2a9 9 0 11-18 0 9 9 0 0118 0z"
					/></svg
				>
				<span>{error}</span>
			</div>
			<div class="flex-none">
				<button class="btn btn-sm" on:click={loadData}>Retry</button>
			</div>
		</div>
	{:else}
		<!-- Relay Cards Grid -->
		<div class="grid grid-cols-1 md:grid-cols-2 gap-6">
			{#each relays as relay}
				<div class="card bg-base-200 shadow-xl">
					<div class="card-body">
						<div class="flex items-center justify-between mb-4">
							<div class="flex items-center">
								<span class="text-2xl mr-2">{getRelayTypeIcon(relay.type)}</span>
								<h2 class="card-title">{relay.name}</h2>
							</div>
							<button
								class="btn btn-primary btn-sm"
								on:click={() => startNewSchedule(relay.id)}
								disabled={editingRelayId !== null}
							>
								Add Schedule
							</button>
						</div>

						<!-- Schedules for this relay -->
						{#if getRelaySchedules(relay.id).length === 0}
							<div class="bg-base-300 p-4 rounded-lg text-center">
								<p>No schedules configured for this relay</p>
							</div>
						{:else}
							<div class="space-y-4">
								{#each getRelaySchedules(relay.id) as schedule}
									<div class="bg-base-300 p-4 rounded-lg">
										<div class="flex justify-between items-center mb-2">
											<div class="font-semibold">
												{formatTimeRange(schedule.startTime, schedule.endTime)}
											</div>
											<div class="flex gap-2">
												<label class="swap">
													<input
														type="checkbox"
														checked={schedule.enabled}
														on:change={() => toggleScheduleEnabled(schedule)}
													/>
													<div class="swap-on btn btn-xs btn-success">ON</div>
													<div class="swap-off btn btn-xs btn-error">OFF</div>
												</label>
												<button
													class="btn btn-xs btn-outline"
													on:click={() => startEditSchedule(schedule)}
													disabled={editingRelayId !== null}
												>
													Edit
												</button>
												<button
													class="btn btn-xs btn-outline btn-error"
													on:click={() => removeSchedule(schedule.id)}
													disabled={editingRelayId !== null}
												>
													Delete
												</button>
											</div>
										</div>
										<div class="text-sm opacity-70">
											{formatDays(schedule.days)}
										</div>
									</div>
								{/each}
							</div>
						{/if}
					</div>
				</div>
			{/each}
		</div>

		<!-- System Time Card -->
		<div class="card bg-base-200 shadow-xl mt-6">
			<div class="card-body">
				<h2 class="card-title mb-4">System Time</h2>
				<div class="grid grid-cols-1 md:grid-cols-2 gap-4">
					<div class="bg-base-300 p-4 rounded-lg">
						<h3 class="font-bold text-lg mb-2">Current Time</h3>
						<p>System time: {new Date().toLocaleString()}</p>
						<p class="mt-2">Time zone: {Intl.DateTimeFormat().resolvedOptions().timeZone}</p>
						<button class="btn btn-sm btn-outline mt-4">Sync Time with Browser</button>
					</div>

					<div class="bg-base-300 p-4 rounded-lg">
						<h3 class="font-bold text-lg mb-2">Next Scheduled Events</h3>
						<ul class="list-disc list-inside">
							{#if schedules.length === 0}
								<li>No upcoming events</li>
							{:else}
								{#each schedules.slice(0, 3) as schedule}
									{#if schedule.enabled}
										<li>
											{relays.find((r) => r.id === schedule.relayId)?.name || 'Unknown'} -
											{schedule.startTime} to {schedule.endTime}
										</li>
									{/if}
								{/each}
							{/if}
						</ul>
					</div>
				</div>
			</div>
		</div>
	{/if}

	<!-- Schedule Edit Modal -->
	{#if editingRelayId !== null}
		<div class="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
			<div class="bg-base-200 p-6 rounded-lg w-full max-w-lg">
				<h3 class="text-xl font-bold mb-4">
					{editingSchedule ? 'Edit Schedule' : 'Create New Schedule'}
				</h3>

				<!-- Time Range Slider -->
				<div class="form-control mb-6">
					<label class="label">
						<span class="label-text">Time Range</span>
						<span class="label-text-alt">
							{editingSchedule ? editingSchedule.startTime : newSchedule.startTime} -
							{editingSchedule ? editingSchedule.endTime : newSchedule.endTime}
						</span>
					</label>
					<div class="relative pt-10 pb-6">
						<!-- Time markers -->
						<div class="flex justify-between text-xs text-opacity-70 mb-1">
							<span>00:00</span>
							<span>06:00</span>
							<span>12:00</span>
							<span>18:00</span>
							<span>24:00</span>
						</div>

						<!-- Slider container -->
						<div class="relative h-5 w-full touch-none select-none">
							<!-- Slider track -->
							<div
								class="absolute h-2 w-full rounded-full bg-base-300 top-1/2 -translate-y-1/2"
							></div>

							<!-- Active range -->
							<div
								class="absolute h-2 rounded-full bg-primary top-1/2 -translate-y-1/2"
								style="left: {(sliderStartValue / 1440) * 100}%; right: {100 -
									(sliderEndValue / 1440) * 100}%;"
							></div>

							<!-- Start thumb with label -->
							<div
								class="absolute w-5 h-5 rounded-full border-2 border-primary bg-background shadow-md cursor-pointer touch-none"
								style="left: calc({(sliderStartValue / 1440) *
									100}% - 0.625rem); top: 50%; transform: translateY(-50%);"
								on:mousedown={(e) => handleStartThumbDrag(e)}
								on:touchstart={(e) => handleStartThumbDrag(e)}
							>
								<div
									class="absolute -top-8 left-1/2 -translate-x-1/2 bg-primary text-primary-content text-xs px-2 py-1 rounded whitespace-nowrap"
								>
									{minutesToTime(sliderStartValue)}
								</div>
							</div>

							<!-- End thumb with label -->
							<div
								class="absolute w-5 h-5 rounded-full border-2 border-primary bg-background shadow-md cursor-pointer touch-none"
								style="left: calc({(sliderEndValue / 1440) *
									100}% - 0.625rem); top: 50%; transform: translateY(-50%);"
								on:mousedown={(e) => handleEndThumbDrag(e)}
								on:touchstart={(e) => handleEndThumbDrag(e)}
							>
								<div
									class="absolute -top-8 left-1/2 -translate-x-1/2 bg-primary text-primary-content text-xs px-2 py-1 rounded whitespace-nowrap"
								>
									{minutesToTime(sliderEndValue)}
								</div>
							</div>

							<!-- Hidden range inputs for accessibility -->
							<input
								type="range"
								min="0"
								max="1440"
								step="15"
								bind:value={sliderStartValue}
								on:input={handleSliderChange}
								class="sr-only"
								aria-label="Start time"
							/>
							<input
								type="range"
								min="0"
								max="1440"
								step="15"
								bind:value={sliderEndValue}
								on:input={handleSliderChange}
								class="sr-only"
								aria-label="End time"
							/>
						</div>

						<!-- Hour markers -->
						<div class="flex justify-between w-full mt-1">
							{#each Array(25) as _, i}
								{#if i % 6 === 0}
									<div class="h-1 w-0.5 bg-base-content opacity-30"></div>
								{:else}
									<div class="h-0.5 w-0.5 bg-base-content opacity-20"></div>
								{/if}
							{/each}
						</div>
					</div>
				</div>

				<!-- Days Selection -->
				<div class="form-control mb-4">
					<label class="label">
						<span class="label-text">Days Active</span>
					</label>
					<div class="flex flex-wrap gap-2">
						{#each daysOfWeek as day, index}
							<label class="label cursor-pointer gap-2 bg-base-300 px-3 py-1 rounded-lg">
								<span class="label-text">{day}</span>
								<input
									type="checkbox"
									class="checkbox checkbox-primary"
									checked={daySelections[index]}
									on:change={() => handleDayToggle(index)}
								/>
							</label>
						{/each}
					</div>
					<div class="flex gap-2 mt-2">
						<button class="btn btn-xs btn-outline" on:click={() => setDayPreset('all')}>
							All
						</button>
						<button class="btn btn-xs btn-outline" on:click={() => setDayPreset('weekdays')}>
							Weekdays
						</button>
						<button class="btn btn-xs btn-outline" on:click={() => setDayPreset('weekends')}>
							Weekends
						</button>
						<button class="btn btn-xs btn-outline" on:click={() => setDayPreset('none')}>
							None
						</button>
					</div>
				</div>

				<!-- Enabled Toggle -->
				<div class="form-control mb-6">
					<label class="label cursor-pointer">
						<span class="label-text">Enabled</span>
						<input
							type="checkbox"
							class="toggle toggle-primary"
							checked={enabledSelection}
							on:change={() => (enabledSelection = !enabledSelection)}
						/>
					</label>
				</div>

				<div class="flex justify-end gap-2">
					<button class="btn btn-outline" on:click={cancelEdit}>Cancel</button>
					<button class="btn btn-primary" on:click={saveSchedule}>
						{editingSchedule ? 'Save Changes' : 'Create Schedule'}
					</button>
				</div>
			</div>
		</div>
	{/if}
</div>

<style>
	/* Custom styling for range inputs */
	input[type='range'] {
		height: 2rem;
	}

	input[type='range']::-webkit-slider-thumb {
		-webkit-appearance: none;
		appearance: none;
		width: 1.5rem;
		height: 1.5rem;
		border-radius: 50%;
		background: hsl(var(--p));
		cursor: pointer;
		border: 2px solid white;
	}

	input[type='range']::-moz-range-thumb {
		width: 1.5rem;
		height: 1.5rem;
		border-radius: 50%;
		background: hsl(var(--p));
		cursor: pointer;
		border: 2px solid white;
	}

	/* Prevent text selection during slider interaction */
	.touch-none {
		touch-action: none;
	}
</style>
