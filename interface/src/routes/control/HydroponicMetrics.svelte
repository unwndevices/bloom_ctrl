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

	// Mockup data generation
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
						data: generateMockData(24, 20, 25), // 2 hours of data between 20-25°C
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

		// Update mockup data every 5 minutes
		const interval = setInterval(() => {
			tempChart.data.datasets[0].data = generateMockData(24, 20, 25);
			ecChart.data.datasets[0].data = generateMockData(24, 1.0, 2.0);
			levelChart.data.datasets[0].data = generateMockData(24, 70, 100);

			tempChart.update('none');
			ecChart.update('none');
			levelChart.update('none');
		}, 300000);

		return () => clearInterval(interval);
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
