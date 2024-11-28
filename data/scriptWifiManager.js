// const ws = new WebSocket(`ws://${window.location.host}/ws`);
// ws.onmessage = function(event) {
//   const options = JSON.parse(event.data);
//   const dropdown = document.getElementById('ssidDropdown');
//   options.forEach(option => {
//     const opt = document.createElement('option');
//     opt.value = option;
//     opt.textContent = option;
//     dropdown.appendChild(opt);
//   });
// };

  // Fetch dropdown options from the server
  window.onload = function() {
    const dropdown = document.getElementById('ssid');
    fetch('/getOptions')
      .then(response => response.json())
      .then(options => {
        options.forEach(option => {
          const opt = document.createElement('option');
          opt.value = option;
          opt.textContent = option;
          dropdown.appendChild(opt);
        });
      })
      .catch(error => console.error('Error fetching dropdown options:', error));
  };