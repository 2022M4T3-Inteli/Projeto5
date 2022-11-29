// Obtém a lista de todas as tags
const tagsRef = database.ref("tags");

tagsRef.on("child_added", (snapshot) => {
  const tr = document.createElement('tr');

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

  const tagsTable = document.getElementById('tags-table');
  tagsTable.appendChild(tr);
});
