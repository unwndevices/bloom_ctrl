# Getting Started with SvelteKit

## Overview

SvelteKit's unique approach makes it perfect for constrained servers. It builds very small files shipping only the minimum required amount of JavaScript. This keeps the memory footprint low so that rich applications can be built with just the 4MB flash of many ESP32 chips.

However, since SvelteKit is designed with server-side rendering first, there are some catches and pitfalls to avoid. This is especially important as nearly all SvelteKit tutorials heavily make use of the combined front and back end features.

## Limitations of adapter-static

To build a website that can be served from an ESP32, adapter-static is used. This means no server functions can be used. The front end is built as a Single-Page Application (SPA) instead. However, SvelteKit will pre-render sites at build time, even if SSR and pre-rendering are disabled. This leads to some restrictions:

1. You can't use any server-side logic like `+page.server.ts`, `+layout.server.ts` or `+server.ts` files in your project.
2. The load function in `+page.ts` gets executed on both server and client. If you try to access browser resources in the load function this will fail. Instead, use a more traditional approach like fetching data in the `+page.svelte` with the `onMount(() => {})` callback.

## Customizing and Theming

### Changing the App Name

`+layout.ts` bundles globally customizable properties like github repository, app name and copyright notice:

```typescript
export const load = (async () => {
  const result = await fetch("/rest/features");
  const item = await result.json();
  return {
    features: item,
    title: "ESP32-SvelteKit",
    github: "theelims/ESP32-sveltekit",
    copyright: "2024 theelims",
    appName: "ESP32 SvelteKit",
  };
}) satisfies LayoutLoad;
```

In `menu.svelte` you can add a discord invite (disabled by default):

```javascript
const discord = { href: ".", active: false };
```

The manifest file (`interface/static/manifest.json`) contains the app name for mobile devices:

```json
{
  "name": "ESP32 SvelteKit",
  "icons": [
    {
      "src": "/favicon.png",
      "sizes": "48x48 72x72 96x96 128x128 256x256"
    }
  ],
  "start_url": "/",
  "display": "fullscreen",
  "orientation": "any"
}
```

### Changing the App Icon and Favicon

- Replace favicon at `interface/static/favicon.png` (256x256 PNG recommended)
- Replace Svelte Logo at `interface/src/lib/assets/logo.png`

### DaisyUI Themes

The overall theme is defined by DaisyUI and can be easily changed. Either select one of DaisyUI's standard themes or create your own. Default themes in `tailwind.config.cjs`:

```javascript
daisyui: {
    themes: ['corporate', 'business'],
    darkTheme: 'business'
}
```

### Opinionated use of Shadows

The frontend uses colored shadows with `shadow-primary` and `shadow-secondary` DaisyUI classes. Use search and replace to change these to a more neutral look if you find the colors too strong.

### Color Scheme Helper

For JS modules that don't accept DaisyUI/TailwindCSS color class names, use the helper function to convert DaisyUI color variables to HSLA:

```javascript
import { daisyColor } from "$lib/DaisyUiHelper";

borderColor: daisyColor('--p'),
backgroundColor: daisyColor('--p', 50)
```

This allows modules like Charts.js to be styled in the current color scheme responsively.

### TS Types Definition

All types are exported from `models.ts`. This is the convenient location to add your custom types when expanding the frontend.

## Development Server Setup

### Proxy Configuration

To ease frontend development, configure the development server's proxy in `vite.config.ts`:

```javascript
proxy: {
    // Proxying REST: http://localhost:5173/rest/bar -> http://192.168.1.83/rest/bar
    '/rest': {
        target: 'http://192.168.1.83',
        changeOrigin: true,
    },
    // Proxying websockets ws://localhost:5173/ws -> ws://192.168.1.83/ws
    '/ws': {
        target: 'ws://192.168.1.83',
        changeOrigin: true,
        ws: true,
    },
}
```

> **Note**: Restart the development server after changing proxy location.

### Starting Development Server

```bash
cd interface
npm run dev
```

## SvelteKit Limitations

Since we're using adapter-static for ESP32 deployment, there are some restrictions:

1. No server-side logic (no `+page.server.ts`, `+layout.server.ts`, or `+server.ts`)
2. Load functions execute on both server and client
3. Use `onMount(() => {})` for browser resource access

## Customization

### App Configuration

Modify `+layout.ts` for global properties:

```typescript
export const load = (async () => {
  return {
    features: item,
    title: "ESP32-SvelteKit",
    github: "theelims/ESP32-sveltekit",
    copyright: "2024 theelims",
    appName: "ESP32 SvelteKit",
  };
}) satisfies LayoutLoad;
```

### Theme Customization

#### DaisyUI Themes

Configure themes in `tailwind.config.cjs`:

```javascript
daisyui: {
    themes: ['corporate', 'business'],
    darkTheme: 'business'
}
```

#### App Icons

- Replace favicon: `interface/static/favicon.png` (256x256 PNG recommended)
- Replace logo: `interface/src/lib/assets/logo.png`

### Color Scheme Helper

For modules not accepting DaisyUI/TailwindCSS colors:

```javascript
import { daisyColor } from "$lib/DaisyUiHelper";

borderColor: daisyColor('--p'),
backgroundColor: daisyColor('--p', 50)
```

## Type Definitions

Add custom types in `models.ts` when expanding the frontend.
