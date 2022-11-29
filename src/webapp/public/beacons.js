// Obtém a lista de todos os beacons
const beaconsRef = database.ref("beacons");

beaconsRef.on("child_added", (snapshot) => {
  const tr = document.createElement('tr');

  const tdId = document.createElement('td');
  tdId.textContent = snapshot.key;
  tr.appendChild(tdId);

  const tdSSID = document.createElement('td');
  tdSSID.textContent = snapshot.val().ssid;
  tr.appendChild(tdSSID);

  const tdPassword = document.createElement('td');
  tdPassword.textContent = snapshot.val().password;
  tr.appendChild(tdPassword);

  const tdColor = document.createElement('td');
  tdColor.textContent = snapshot.val().color;
  tr.appendChild(tdColor);

  const tdRoom = document.createElement('td');
  tdRoom.textContent = snapshot.val().room;
  tr.appendChild(tdRoom);

  const tdX = document.createElement('td');
  tdX.textContent = snapshot.val().x;
  tr.appendChild(tdX);

  const tdY = document.createElement('td');
  tdY.textContent = snapshot.val().y;
  tr.appendChild(tdY);

  const beaconsTable = document.getElementById('beacons-table');
  beaconsTable.appendChild(tr);
});
