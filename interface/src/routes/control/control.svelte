<script lang="ts">
	import { onMount, onDestroy } from 'svelte';
	import { user } from '$lib/stores/user';
	import { page } from '$app/state';
	import { notifications } from '$lib/components/toasts/notifications';
	import SettingsCard from '$lib/components/SettingsCard.svelte';
	import Light from '~icons/tabler/bulb';
	import Pump from '~icons/tabler/droplet';
	import Extra from '~icons/tabler/plug';
	import Info from '~icons/tabler/info-circle';
	import Apply from '~icons/tabler/checks';
	import Reload from '~icons/tabler/reload';
	import { socket } from '$lib/stores/socket';
	import type { RelayState } from '$lib/types/models';
	import HydroponicMetrics from './HydroponicMetrics.svelte';

	let relayState: RelayState = $state({
		light_relay: false,
		pump_relay: false,
		extra_relay: false
	});

	let relayControls = $state({
		light_relay: false,
		pump_relay: false,
		extra_relay: false
	});

	async function getRelayState() {
		try {
			const response = await fetch('/rest/relayState', {
				method: 'GET',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				}
			});
			const state = await response.json();
			relayControls = {
				light_relay: state.light_relay,
				pump_relay: state.pump_relay,
				extra_relay: state.extra_relay
			};
		} catch (error) {
			console.error('Error:', error);
		}
		return;
	}

	onMount(() => {
		socket.on<RelayState>('relay', (data) => {
			relayState = data;
		});
		getRelayState();
	});

	onDestroy(() => socket.off('relay'));

	async function postRelayState() {
		try {
			const response = await fetch('/rest/relayState', {
				method: 'POST',
				headers: {
					Authorization: page.data.features.security ? 'Bearer ' + $user.bearer_token : 'Basic',
					'Content-Type': 'application/json'
				},
				body: JSON.stringify(relayControls)
			});
			if (response.status == 200) {
				notifications.success('Relay states updated.', 3000);
				const state = await response.json();
				relayControls = {
					light_relay: state.light_relay,
					pump_relay: state.pump_relay,
					extra_relay: state.extra_relay
				};
			} else {
				notifications.error('User not authorized.', 3000);
			}
		} catch (error) {
			console.error('Error:', error);
		}
	}
</script>

<div class="flex flex-col gap-4">
	<SettingsCard>
		{#snippet icon()}
			<Light class="flex-shrink-0 mr-2 h-6 w-6 self-end" />
		{/snippet}
		{#snippet title()}
			<span>Remote Controls</span>
		{/snippet}
		<div class="w-full">
			<div class="flex flex-col gap-y-4">
				<div class="flex flex-row flex-wrap justify-between gap-x-2 items-center">
					<div class="form-control w-52">
						<label class="label cursor-pointer">
							<Light class="mr-2 h-5 w-5" />
							<span class="mr-4">Grow Light</span>
							<input
								type="checkbox"
								bind:checked={relayControls.light_relay}
								class="checkbox checkbox-primary"
							/>
						</label>
					</div>
					<div class="form-control w-52">
						<label class="label cursor-pointer">
							<Pump class="mr-2 h-5 w-5" />
							<span class="mr-4">Water Pump</span>
							<input
								type="checkbox"
								bind:checked={relayControls.pump_relay}
								class="checkbox checkbox-primary"
							/>
						</label>
					</div>
					<div class="form-control w-52">
						<label class="label cursor-pointer">
							<Extra class="mr-2 h-5 w-5" />
							<span class="mr-4">Extra Relay</span>
							<input
								type="checkbox"
								bind:checked={relayControls.extra_relay}
								class="checkbox checkbox-primary"
							/>
						</label>
					</div>
					<div class="flex-grow mt-5"></div>
					<div class="flex flex-row gap-x-2 pt-5">
						<button class="btn btn-primary inline-flex items-center" onclick={postRelayState}
							><Apply class="mr-2 h-5 w-5" /><span>Apply</span></button
						>
						<button class="btn btn-primary inline-flex items-center" onclick={getRelayState}
							><Reload class="mr-2 h-5 w-5" /><span>Reload</span></button
						>
					</div>
				</div>
			</div>
		</div>
	</SettingsCard>
	<HydroponicMetrics />
</div>
