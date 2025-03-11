/** @type {import('tailwindcss').Config} */
module.exports = {
	content: ['./src/**/*.{html,js,svelte,ts}'],
	theme: {
		extend: {}
	},
	plugins: [require('daisyui')],
	daisyui: {
		themes: ['emerald', 'forest'],
		darkTheme: 'forest',
		lightTheme: 'emerald'
	}
};
