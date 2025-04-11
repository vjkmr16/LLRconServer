# RconServer

## Overview

RconServer is a powerful plugin that introduces RCON (Remote Console) support to the LeviLamina. This allows for enhanced remote management of your server, enabling you to execute commands from a distance.

## Special Thanks

A special thanks to [Jaskowicz1](https://github.com/Jaskowicz1/rconpp) for his project. It served as the basic foundation of this plugin.

## Configuration

The RCON server can be easily configured by using the following JSON format. Below is an example configuration:

```json
{
    "version": 1,
    "rconSettings": {
        "port": 19130,
        "maxConnections": 5,
        "password": "Change this to your password",
        "logOnNewConnection": true,
        "logOnClientAuth": true,
        "logOnClientDisconnect": true,
        "logOnCommand": true,
        "logOnDebugInfo": false
    }
}