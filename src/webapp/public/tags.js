let deletedTagId = null; // referencia para a tag que foi deletada

// Obtém a lista de todas as tags
const tagsRef = database.ref("tags");
tagsRef.on("child_added", (snapshot) => {
  const tr = document.createElement('tr');
  tr.id = snapshot.key;

  const tdName = document.createElement('td');
  tdName.textContent = snapshot.val().name;
  tr.appendChild(tdName);

  const tdId = document.createElement('td');
  tdId.textContent = snapshot.key;
  tr.appendChild(tdId);

  const tdColor = document.createElement('td');
  tdColor.textContent = snapshot.val().color;
  tr.appendChild(tdColor);

  const tdRoom = document.createElement('td');
  tdRoom.textContent = snapshot.val().room;
  tr.appendChild(tdRoom);

  const tdBtn = document.createElement('td');
  tdBtn.style.textAlign = 'center';

  const editBtn = document.createElement('button');
  editBtn.innerHTML = "<i class=\"fas fa-edit\"></i>";
  editBtn.classList.add('btn', 'btn-warning', 'btn-sm');
  editBtn.addEventListener('click', () => {
    const result = confirm(`Deseja editar ${snapshot.key}?`);
    if (result) {
      document.getElementById('tagId').value = snapshot.key;
      document.getElementById('name').value = snapshot.val().name;
      document.getElementById('color').value = snapshot.val().color;
      document.getElementById('room').value = snapshot.val().room;
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
      deletedTagId = snapshot.key;
      tagsRef.child(snapshot.key).remove();
    }
  });
  tdBtn.appendChild(deleteBtn);

  tr.appendChild(tdBtn);

  const tagsTable = document.getElementById('tags-table');
  tagsTable.appendChild(tr);
});

const addTagForm = document.getElementById('add-tag-form');
addTagForm.addEventListener('submit', (event) => {
  event.preventDefault();
  const id = addTagForm.tagId.value;
  const name = addTagForm.name.value;
  const color = addTagForm.color.value;
  const room = addTagForm.room.value;

  tagsRef.child(id).set({
    name,
    id,
    color,
    room
  });
  addTagForm.reset();
});

tagsRef.on("child_removed", (snapshot) => {
  const tr = document.getElementById(deletedTagId);
  tr.remove();
});

tagsRef.on("child_changed", (snapshot) => {
  const tr = document.getElementById(snapshot.key);
  tr.children[0].textContent = snapshot.val().name;
  tr.children[1].textContent = snapshot.key;
  tr.children[2].textContent = snapshot.val().color;
  tr.children[3].textContent = snapshot.val().room;
});
