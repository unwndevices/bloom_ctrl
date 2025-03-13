<script lang="ts">
	import { onDestroy, onMount } from 'svelte';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import { slide } from 'svelte/transition';
	import { cubicOut } from 'svelte/easing';
	import { Chart, registerables } from 'chart.js';
	import * as LuxonAdapter from 'chartjs-adapter-luxon';
	import { daisyColor } from '$lib/DaisyUiHelper';
	import Metrics from '~icons/tabler/chart-dots';

	Chart.register(...registerables);
	Chart.register(LuxonAdapter);

	let tempChartElement: HTMLCanvasElement;
	let tempChart: Chart;
	let ecChartElement: HTMLCanvasElement;
	let ecChart: Chart;
	let levelChartElement: HTMLCanvasElement;
	let levelChart: Chart;
	let tempSocket: WebSocket;

	let tempReadings: { x: number; y: number }[] = [];

	// Mockup data generation for EC and level (keeping these for now)
	const generateMockData = (count: number, min: number, max: number) => {
		const data = [];
		const now = Date.now();
		for (let i = 0; i < count; i++) {
			data.push({
				x: now - (count - i) * 300000, // Five minute intervals
				y: Math.random() * (max - min) + min
			});
		}
		return data;
	};

	function connectTempWebSocket() {
		const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
		const wsUrl = `${protocol}//${window.location.host}/ws/tempState`;

		tempSocket = new WebSocket(wsUrl);

		tempSocket.onmessage = (event) => {
			const data = JSON.parse(event.data);

			// Handle initial history data and updates
			if (data.history && Array.isArray(data.history)) {
				// Convert history data to chart format, ensuring proper timestamp handling
				tempReadings = data.history
					.map((reading: { temperature: number; timestamp: number }) => ({
						x: reading.timestamp, // Already in milliseconds from backend
						y: reading.temperature
					}))
					.sort((a: { x: number }, b: { x: number }) => a.x - b.x); // Sort by timestamp to ensure proper order
			} else {
				// Handle real-time updates with current temperature
				tempReadings = [
					...tempReadings,
					{
						x: data.timestamp || Date.now(), // Use server timestamp if available, fallback to client time
						y: data.temperature
					}
				];

				// Keep only last 100 readings
				if (tempReadings.length > 100) {
					tempReadings = tempReadings.slice(-100);
				}
			}

			// Update chart with new data
			if (tempChart) {
				tempChart.data.datasets[0].data = [...tempReadings];

				// Adjust x-axis scale based on data range
				const timeRange =
					tempReadings.length > 1
						? tempReadings[tempReadings.length - 1].x - tempReadings[0].x
						: 3600000; // 1 hour default

				const unit = timeRange > 86400000 ? 'day' : timeRange > 3600000 ? 'hour' : 'minute';

				tempChart.options.scales.x.time.unit = unit;
				tempChart.update('none');
			}
		};

		tempSocket.onclose = () => {
			// Attempt to reconnect after 5 seconds if connection is lost
			setTimeout(connectTempWebSocket, 5000);
		};
	}

	onMount(() => {
		// Temperature Chart
		tempChart = new Chart(tempChartElement, {
			type: 'line',
			data: {
				datasets: [
					{
						label: 'Water Temperature',
						borderColor: daisyColor('--p'),
						backgroundColor: daisyColor('--p', 50),
						borderWidth: 2,
						data: tempReadings,
						tension: 0.4
					}
				]
			},
			options: {
				maintainAspectRatio: false,
				responsive: true,
				plugins: {
					legend: {
						display: true
					},
					tooltip: {
						mode: 'index',
						intersect: false,
						callbacks: {
							title: (context) => {
								// Format timestamp to local date and time
								const timestamp = context[0].parsed.x;
								return new Date(timestamp).toLocaleString();
							}
						}
					}
				},
				scales: {
					x: {
						type: 'time',
						time: {
							displayFormats: {
								minute: 'HH:mm',
								hour: 'MMM D, HH:mm',
								day: 'MMM D'
							},
							tooltipFormat: 'MMM D, HH:mm:ss'
						},
						grid: {
							color: daisyColor('--bc', 10)
						},
						ticks: {
							color: daisyColor('--bc'),
							maxRotation: 0,
							autoSkip: true,
							maxTicksLimit: 8
						}
					},
					y: {
						title: {
							display: true,
							text: 'Temperature (°C)',
							color: daisyColor('--bc')
						},
						min: 15,
						max: 30,
						grid: { color: daisyColor('--bc', 10) },
						ticks: {
							color: daisyColor('--bc')
						}
					}
				}
			}
		});

		// Connect to temperature WebSocket
		connectTempWebSocket();

		// EC Chart
		ecChart = new Chart(ecChartElement, {
			type: 'line',
			data: {
				datasets: [
					{
						label: 'EC Level',
						borderColor: daisyColor('--s'),
						backgroundColor: daisyColor('--s', 50),
						borderWidth: 2,
						data: generateMockData(24, 1.0, 2.0), // 2 hours of data between 1.0-2.0 mS/cm
						tension: 0.4
					}
				]
			},
			options: {
				maintainAspectRatio: false,
				responsive: true,
				plugins: {
					legend: {
						display: true
					},
					tooltip: {
						mode: 'index',
						intersect: false
					}
				},
				scales: {
					x: {
						type: 'time',
						time: {
							unit: 'minute',
							displayFormats: {
								minute: 'HH:mm'
							}
						},
						grid: {
							color: daisyColor('--bc', 10)
						},
						ticks: {
							color: daisyColor('--bc')
						}
					},
					y: {
						title: {
							display: true,
							text: 'EC (mS/cm)',
							color: daisyColor('--bc')
						},
						min: 0,
						max: 3,
						grid: { color: daisyColor('--bc', 10) },
						ticks: {
							color: daisyColor('--bc')
						}
					}
				}
			}
		});

		// Water Level Chart
		levelChart = new Chart(levelChartElement, {
			type: 'line',
			data: {
				datasets: [
					{
						label: 'Water Level',
						borderColor: daisyColor('--a'),
						backgroundColor: daisyColor('--a', 50),
						borderWidth: 2,
						data: generateMockData(24, 70, 100), // 2 hours of data between 70-100%
						tension: 0.4
					}
				]
			},
			options: {
				maintainAspectRatio: false,
				responsive: true,
				plugins: {
					legend: {
						display: true
					},
					tooltip: {
						mode: 'index',
						intersect: false
					}
				},
				scales: {
					x: {
						type: 'time',
						time: {
							unit: 'minute',
							displayFormats: {
								minute: 'HH:mm'
							}
						},
						grid: {
							color: daisyColor('--bc', 10)
						},
						ticks: {
							color: daisyColor('--bc')
						}
					},
					y: {
						title: {
							display: true,
							text: 'Level (%)',
							color: daisyColor('--bc')
						},
						min: 0,
						max: 100,
						grid: { color: daisyColor('--bc', 10) },
						ticks: {
							color: daisyColor('--bc')
						}
					}
				}
			}
		});

		// Update only mockup data for EC and level every 5 minutes
		const interval = setInterval(() => {
			ecChart.data.datasets[0].data = generateMockData(24, 1.0, 2.0);
			levelChart.data.datasets[0].data = generateMockData(24, 70, 100);

			ecChart.update('none');
			levelChart.update('none');
		}, 300000);

		return () => clearInterval(interval);
	});

	onDestroy(() => {
		if (tempSocket) {
			tempSocket.close();
		}
	});
</script>

<SettingsCard>
	{#snippet icon()}
		<Metrics class="flex-shrink-0 mr-2 h-6 w-6 self-end" />
	{/snippet}
	{#snippet title()}
		<span>Hydroponic Metrics</span>
	{/snippet}

	<div class="w-full overflow-x-auto">
		<div class="flex w-full flex-col space-y-4">
			<div
				class="flex w-full flex-col space-y-1 h-52"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<canvas bind:this={tempChartElement}></canvas>
			</div>
			<div
				class="flex w-full flex-col space-y-1 h-52"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<canvas bind:this={ecChartElement}></canvas>
			</div>
			<div
				class="flex w-full flex-col space-y-1 h-52"
				transition:slide|local={{ duration: 300, easing: cubicOut }}
			>
				<canvas bind:this={levelChartElement}></canvas>
			</div>
		</div>
	</div>
</SettingsCard>
