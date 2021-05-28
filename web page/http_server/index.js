// Importing required node modules
const express = require('express');
const path = require('path');

// Importing server configuration
const config = require('./config');

// Importing locations
const { getSafeZone, isValidZone, isInsideZone } = require('./locations');

// Global variables
let app;

// Initializing the server
(async() => {

    // Creates the express app
    app = express();

    // Logging requests
    app.use((req, res, next) => {
        console.log(`[Server] HTTP - ${req.method} - ${req.path}`);
        next();
    });

    // Get Safe Zone Endpoint
    app.get('/api/safe_zone', (req, res) => {
        let zone = req.query.zone; 
        if ((zone !== undefined) && isValidZone(zone))
        {
            res.send(getSafeZone(zone));
        }
        else
        {
            res.status(400).send({
                message: "Provide a valid safe zone"
            });
        }
    });

    // See if location is inside zone endpoint
    app.get('/api/is_inside', (req, res) => {
        let zone = req.query.zone;
        let lat = Number(req.query.latitude), lng = Number(req.query.longitude);
        console.log(`Location: [${lat}, ${lng}]. Type: ${typeof(lat)}`);
        if ( (typeof zone === 'string') && (typeof lat === 'number') && (typeof lng === 'number') && isValidZone(zone))
        {
            res.send(isInsideZone(zone, {lat, lng}));
        }
        else
        {
            res.status(400).send({
                message: "Please specify both a safe zone and a location"
            });
        }
    });
    
    // Serve index.html for default route
    app.get('*', (req, res) => {
        res.sendFile(path.join(__dirname, '/index.html'));
    });

    // Serve index.html for default route
    // app.get('*', (req, res) => {
    //     res.sendFile(process.cwd() + "/app/dist/app/index.html");
    // });

    app.listen(config.server.port, () => {
        // Console message
        console.log(`[Server] The server is listening to port ${config.server.port}`);
    });

})();
