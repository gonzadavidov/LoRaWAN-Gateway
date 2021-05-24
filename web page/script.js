const createMap = ({ lat, lng }) => {
  return new google.maps.Map(document.getElementById('map'), {
    center: { lat, lng},
    zoom: 15,
    Size: 30
  });
};

const createMarker = ({ map, position, image }) => {
  return new google.maps.Marker({ map, position, icon: image });
};

const createSafeZone = (safeZone, map) => {
  return new google.maps.Polygon({
    paths: safeZone,
    strokeColor: 'white',
    strokeOpacity: 0.8,
    strokeWeight: 2,
    fillColor: 'green',
    fillOpacity: 0.35,
  });
}

function toggleHeatmap() {
  heatmap.setMap(heatmap.getMap() ? null : map);
}

function changeGradient() {
  const gradient = [
    "rgba(0, 255, 255, 0)",
    "rgba(0, 255, 255, 1)",
    "rgba(0, 191, 255, 1)",
    "rgba(0, 127, 255, 1)",
    "rgba(0, 63, 255, 1)",
    "rgba(0, 0, 255, 1)",
    "rgba(0, 0, 223, 1)",
    "rgba(0, 0, 191, 1)",
    "rgba(0, 0, 159, 1)",
    "rgba(0, 0, 127, 1)",
    "rgba(63, 0, 91, 1)",
    "rgba(127, 0, 63, 1)",
    "rgba(191, 0, 31, 1)",
    "rgba(255, 0, 0, 1)",
  ];
  heatmap.set("gradient", heatmap.get("gradient") ? null : gradient);
}

function changeRadius() {
  heatmap.set("radius", heatmap.get("radius") ? null : 20);
}

function changeOpacity() {
  heatmap.set("opacity", heatmap.get("opacity") ? null : 0.2);
}

async function init() {
  const bicycle = {
                    url: "https://icon-library.com/images/bicycle-icon-png/bicycle-icon-png-20.jpg",
                    scaledSize: new google.maps.Size(50, 50)
                  }
  var bicyclePositions = [];
  var response = await fetch('https://api.thingspeak.com/channels/1396775/feeds.json?api_key=V4QNC2WLJQPOJJ65&location=true');
  var data = await response.json();
  data["feeds"].forEach( (el) => {
    bicyclePositions.push({
      lat: Number(el["latitude"]),
      lng: Number(el["longitude"])
    })
  });
  const lastPosition = bicyclePositions[bicyclePositions.length - 1];
  const safeZone = [
    { lat: 44.505688, lng: 11.339667 },
    { lat: 44.504701, lng: 11.345555 }, 
    { lat: 44.504066, lng: 11.348087 },
    { lat: 44.500906, lng: 11.356101 },
    { lat: 44.485728, lng: 11.357894 },
    { lat: 44.484496, lng: 11.356231 },
    { lat: 44.486479, lng: 11.339515 },
    { lat: 44.490245, lng: 11.329655 },
    { lat: 44.499135, lng: 11.327006 },
  ];
  const map = createMap(lastPosition);
  const marker = createMarker({ map, position: lastPosition, image: bicycle });
  const bolognaSafeZone = createSafeZone(safeZone, map);
  //Set safe zone on the actual map
  bolognaSafeZone.setMap(map);
  const heatmap = new google.maps.visualization.HeatmapLayer({
    data: bicyclePositions,
    map: map,
  });
  heatmap.setMap(map);
}