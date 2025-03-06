# Getting Started

## Prerequisites

This project has quite a complicated build chain to prepare the frontend code for the ESP32. You will need to install some tools to make this all work, starting with a powerful code editor.

### Required Software

Please install the following software, if you haven't already:

- **VSCode** - IDE for development
- **Node.js** - For building the interface with npm

### Required VSCode Plugins

Please install the following mandatory VSCode Plugins:

- **PlatformIO** - Embedded development platform
- **Prettier** - Automated code formatter
- **Svelte for VS Code** - Makes working with Svelte much easier
- **Svelte Intellisense** - Another Svelte tool
- **Tailwind CSS Intellisense** - Makes working with Tailwind CSS much easier
- **Prettier plugin for Tailwind CSS** - Automatically sorts Tailwind classes

### Documentation Engine (Optional)

To use Materials for MkDocs as your documentation engine:

```bash
pip install mkdocs-material
```

> **Tip**: You might need to run this as administrator if you get an error message.

## Project Structure

| Resource         | Description                                     |
| ---------------- | ----------------------------------------------- |
| `.github/`       | Github CI pipeline to deploy MkDocs to gh-pages |
| `docs/`          | MkDocs documentation files                      |
| `interface/`     | SvelteKit based front end                       |
| `lib/framework/` | C++ back end for the ESP32 device               |
| `src/`           | The main.cpp and demo project                   |
| `scripts/`       | Scripts that build the interface                |
| `platformio.ini` | PlatformIO project configuration                |
| `mkdocs.yaml`    | MkDocs project configuration                    |

## Setting up PlatformIO

### Setup Build Target

> **Important**: Do not use the PlatformIO UI for editing platformio.ini

It is tempting to use the PlatformIO user interface to add dependencies or parameters to platformio.ini. However, doing so will remove all "irrelevant" information like comments from the file. Please edit the file directly in the editor.

The `platformio.ini` file controls the entire build process. It comes pre-configured with several ESP32 boards:

```ini
[platformio]
default_envs = esp32-s3-devkitc-1

[env:adafruit_feather_esp32_v2]
board = adafruit_feather_esp32_v2
board_build.mcu = esp32

[env:lolin_c3_mini]
board = lolin_c3_mini
board_build.mcu = esp32c3

[env:esp32-s3-devkitc-1]
board = esp32-s3-devkitc-1
board_build.mcu = esp32s3
```

If your board isn't listed in the platformio.ini, you can look in the official board list for supported boards and add their information accordingly. Either delete the obsolete [env:...] sections or change your board using `default_envs = ...`.

> **Note**: The default setup is for an ESP32-S3-DevKitC/M board by Espressif connected to the UART USB port. If you use another board and experience undesired behavior, it's likely due to mismatched pin definitions.

### Build & Upload Process

1. Configure platformio.ini for your board
2. Upload the sample code to your board
3. This will download ESP32 libraries and execute node install
4. Select your board's environment under the PlatformIO tab
5. Hit Upload and Monitor

> **Note**: The first build process will take a while. After a few minutes, you'll see ESP32 outputting information on the terminal. Some Python scripts might need additional packages - if the first build fails, just run it again.

### Development Tips

VSCode allows multiple terminals running simultaneously:

- Use one terminal for the serial monitor
- Use another terminal for the development server

## Setting up SvelteKit

### Setup Proxy for Development

To ease frontend development, deploy the backend code on an ESP32 board and pass the websocket and REST API calls through the development server's proxy. Configure the proxy in `vite.config.ts`:

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

> **Tip**: You must restart the development server after changing the proxy location.

### Development Server

The interface uses Vite as a development server, providing hot module reloading that reflects code changes instantly in your browser. To start:

```bash
cd interface
npm run dev
```

Follow the link in the terminal to access the frontend in your browser.

## Setting up MkDocs

Material for MkDocs allows you to create great technical documentation pages from markup. You have two options:

1. If you don't want to use it:

   - Delete the `.github` and `docs` folders
   - Delete `mkdocs.yaml`

2. To use it:
   - Commit and push to your repository to initiate the github CI pipeline
   - This triggers the automatic build
   - After a few minutes, a new branch `gh-pages` will appear with your documentation
   - Go to your github repository settings to deploy from gh-pages

To use the built-in development server with hot reload:

```bash
mkdocs serve
```
