let deletedBeaconId = null; // referencia para o beacon que foi deletado

// Obtém a lista de todos os beacons
const beaconsRef = database.ref("beacons");
beaconsRef.on("child_added", (snapshot) => {
  const tr = document.createElement('tr');
  tr.id = snapshot.key;

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

  const tdBtn = document.createElement('td');
  tdBtn.style.textAlign = 'center';

  const editBtn = document.createElement('button');
  editBtn.innerHTML = "<i class=\"fas fa-edit\"></i>";
  editBtn.classList.add('btn', 'btn-warning', 'btn-sm');
  editBtn.addEventListener('click', () => {
    const result = confirm(`Deseja editar ${snapshot.key}?`);
    if (result) {
      document.getElementById('beaconID').value = snapshot.key;
      document.getElementById('ssid').value = snapshot.val().ssid;
      document.getElementById('password').value = snapshot.val().password;
      document.getElementById('color').value = snapshot.val().color;
      document.getElementById('room').value = snapshot.val().room;
      document.getElementById('x').value = snapshot.val().x;
      document.getElementById('y').value = snapshot.val().y;
      window.scrollTo(0, 0); // scroll para o topo da pagina
    }
  });
  tdBtn.appendChild(editBtn);

  const deleteBtn = document.createElement('button');
  deleteBtn.innerHTML = "<i class=\"fas fa-trash\"></i>";
  deleteBtn.classList.add('btn', 'btn-danger', 'btn-sm', 'ml-2');
  deleteBtn.addEventListener('click', () => {
    const result = confirm(`Deseja deletar ${snapshot.key}?`);
    if (result) {
      deletedBeaconId = snapshot.key;
      beaconsRef.child(snapshot.key).remove();
    }
  });
  tdBtn.appendChild(deleteBtn);

  tr.appendChild(tdBtn);

  const beaconsTable = document.getElementById('beacons-table');
  beaconsTable.appendChild(tr);
});

const addBeaconForm = document.getElementById('add-beacon-form');
addBeaconForm.addEventListener('submit', (event) => {
  event.preventDefault();
  const id = addBeaconForm.beaconID.value;
  const ssid = addBeaconForm.ssid.value;
  const password = addBeaconForm.password.value;
  const color = addBeaconForm.color.value;
  const room = addBeaconForm.room.value;
  const x = addBeaconForm.x.value;
  const y = addBeaconForm.y.value;
  beaconsRef.child(id).set({
    ssid,
    password,
    color,
    room,
    x,
    y
  });
  addBeaconForm.reset();
});

beaconsRef.on("child_removed", (snapshot) => {
  const tr = document.getElementById(deletedBeaconId);
  tr.remove();
});

beaconsRef.on("child_changed", (snapshot) => {
  const tr = document.getElementById(snapshot.key);
  tr.childNodes[1].textContent = snapshot.val().ssid;
  tr.childNodes[2].textContent = snapshot.val().password;
  tr.childNodes[3].textContent = snapshot.val().color;
  tr.childNodes[4].textContent = snapshot.val().room;
  tr.childNodes[5].textContent = snapshot.val().x;
  tr.childNodes[6].textContent = snapshot.val().y;
});
