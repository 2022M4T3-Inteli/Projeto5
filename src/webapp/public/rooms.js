let deletedRoomId = null; // referencia para a sala que foi deletada

// Obtém a lista de todas as salas
const roomsRef = database.ref("rooms");
roomsRef.on("child_added", (snapshot) => {
  const tr = document.createElement('tr');
  tr.id = snapshot.key;

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

  const x = document.createElement('td');
  x.textContent = snapshot.val().x;
  tr.appendChild(x);

  const y = document.createElement('td');
  y.textContent = snapshot.val().y;
  tr.appendChild(y);

  const tdBtn = document.createElement('td');
  tdBtn.style.textAlign = 'center';

  const editBtn = document.createElement('button');
  editBtn.innerHTML = "<i class=\"fas fa-edit\"></i>";
  editBtn.classList.add('btn', 'btn-warning', 'btn-sm');
  editBtn.addEventListener('click', () => {
    const result = confirm(`Deseja editar ${snapshot.key}?`);
    if (result) {
      document.getElementById('roomId').value = snapshot.key;
      document.getElementById('description').value = snapshot.val().description;
      document.getElementById('beacon1').value = snapshot.val().beacon1;
      document.getElementById('beacon2').value = snapshot.val().beacon2;
      document.getElementById('beacon3').value = snapshot.val().beacon3;
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
      deletedRoomId = snapshot.key;
      roomsRef.child(snapshot.key).remove();
    }

  });
  tdBtn.appendChild(deleteBtn);

  tr.appendChild(tdBtn);

  const roomsTable = document.getElementById('rooms-table');
  roomsTable.appendChild(tr);
});

const addRoomForm = document.getElementById('add-room-form');
addRoomForm.addEventListener('submit', (event) => {
  event.preventDefault();
  const id = addRoomForm.roomId.value;
  const description = addRoomForm.description.value;
  const beacon1 = addRoomForm.beacon1.value;
  const beacon2 = addRoomForm.beacon2.value;
  const beacon3 = addRoomForm.beacon3.value;
  const x = addRoomForm.x.value;
  const y = addRoomForm.y.value;
  roomsRef.child(id).set({
    description,
    beacon1,
    beacon2,
    beacon3,
    x,
    y
  });
  addRoomForm.reset();
});

roomsRef.on("child_removed", (snapshot) => {
  const tr = document.getElementById(deletedRoomId);
  tr.remove();
});

roomsRef.on("child_changed", (snapshot) => {
  const tr = document.getElementById(snapshot.key);
  tr.childNodes[1].textContent = snapshot.val().description;
  tr.childNodes[2].textContent = snapshot.val().beacon1;
  tr.childNodes[3].textContent = snapshot.val().beacon2;
  tr.childNodes[4].textContent = snapshot.val().beacon3;
  tr.childNodes[5].textContent = snapshot.val().x;
  tr.childNodes[6].textContent = snapshot.val().y;
});
