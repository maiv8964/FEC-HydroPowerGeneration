# FEC-HydroPowerGeneration

<p align="center">
  <img src="https://github.com/user-attachments/assets/ac893e2a-7d1a-405e-95d2-84acbe499238" />
</p>

The code in the repository contains all the files needed to operate the monitoring system for the FEC Hydroelectric Power Generation System. Below contains the preliminary setup guide for Node RED.

# Initial Setup
To start setting up the Node RED interface, the hosting computer must have Node-RED installed.  
[Node-RED Local Installation Guide](https://nodered.org/docs/getting-started/local)

Once installed, the command below can initialize the interface:
```
node-red start
```
The Node-RED interface builder can be accessed in the browser with the URL:
```
localhost:1880
```

Once inside the interface, click on the top right 3 bars, and select "Import"

<p align="center">
  <img height="100"src="https://github.com/user-attachments/assets/a2e66b7c-ca09-42fa-a472-e61f9cf1f6a6" />
</p>

The json code in the file [HydroPowerGenerationFlow.json](HydroPowerGenerationFlow.json) can be copied into the window to import the interface.
