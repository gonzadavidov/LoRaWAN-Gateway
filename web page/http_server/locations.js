// Load safe zones 'database'
const safe_zones = require('./safe_zones.json');

function isValidZone(zoneName)
{
    return safe_zones[zoneName] !== undefined;
}

function getSafeZone(zoneName)
{
    return safe_zones[zoneName];
}

function isInsideZone(zoneName, loc)
{
    zone = safe_zones[zoneName];
    // Agregar API Google Maps para ver si loc esta dentro de la zona.
    return true;
}

module.exports = { getSafeZone, isValidZone, isInsideZone };