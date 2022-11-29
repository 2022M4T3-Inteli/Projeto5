// Obtém a lista de todas as salas
const roomsRef = database.ref("rooms");
roomsRef.on("child_added", (snapshot) => {
  const tr = document.createElement('tr');
  const tdId = document.createElement('td');
  tdId.textContent = snapshot.key;
  tr.appendChild(tdId);

  const tdDescription = document.createElement('td');
  tdDescription.textContent = snapshot.val().description;
  tr.appendChild(tdDescription);

  const tdBeacon1 = document.createElement('td');
  tdBeacon1.textContent = snapshot.val().beacon1;
  tr.appendChild(tdBeacon1);

  const tdBeacon2 = document.createElement('td');
  tdBeacon2.textContent = snapshot.val().beacon2;
  tr.appendChild(tdBeacon2);

  const tdBeacon3 = document.createElement('td');
  tdBeacon3.textContent = snapshot.val().beacon3;
  tr.appendChild(tdBeacon3);

  const roomsTable = document.getElementById('rooms-table');
  roomsTable.appendChild(tr);
});
