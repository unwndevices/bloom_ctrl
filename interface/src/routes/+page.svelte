<script lang="ts">
	import type { PageData } from './$types';
	import logo from '$lib/assets/logo.svg';
	import { notifications } from '$lib/components/toasts/notifications';

	interface Props {
		data: PageData;
	}

	let { data }: Props = $props();
</script>

<div class="hero bg-base-100 h-screen">
	<div class="card md:card-side bg-base-200 shadow-secondary shadow-2xl">
		<figure class="bg-base-200">
			<img src={logo} alt="Logo" class="h-auto w-64 aspect-auto" />
		</figure>
		<div class="card-body w-80">
			<h2 class="card-title text-center text-2xl">Welcome to Bloom Control by Greenverse</h2>
			<p class="py-6 text-center">An easy to use control system for your Hydroponic System.</p>
			<a
				class="btn btn-primary"
				href="/control"
				onclick={() => notifications.success('You did it!', 1000)}>Start Control</a
			>
		</div>
	</div>
</div>
