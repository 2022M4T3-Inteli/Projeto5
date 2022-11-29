// FIREBASE REAL TIME DATABASE (RTDB)

const devices = {};
const rooms = {};

// Obtém a lista de todas as salas
const roomsRef = database.ref("rooms");
roomsRef.on("child_added", (snapshot) => {
  const id = snapshot.key;
  const room = snapshot.val();
  rooms[id] = room;

  // Adiciona o nome da sala no select
  const option = document.createElement("option");
  option.value = id;
  option.textContent = room.description;

  const roomSelector = document.getElementById("room");
  roomSelector.appendChild(option);

  // Background do canvas (mapa do local))
  document.getElementById("map").style.background =
    `url('${document.getElementById('room').value}.jpeg') no-repeat center center`;
});

// Obtém a lista de todos os beacons
const beaconsRef = database.ref("beacons");
beaconsRef.on("child_added", (snapshot) => {
  const id = snapshot.key;
  const beaconRef = database.ref("beacons/" + id); // Referência do beacon
  beaconRef.on(
    "value",
    async (snapshot) => {
      const beacon = snapshot.val();
      const { x, y, color, room } = beacon;
      devices[snapshot.key] = { x, y, color, room, type: "beacon" };
      updateCanvas();
    },
    (error) => {
      console.log(error);
    }
  );
});

function getRoomTags() {
  // Obtém a lista de todas as tags
  const tagsRef = database.ref("tags");
  tagsRef.on("child_added", (snapshot) => {
    const id = snapshot.key;
    // Para cada tag, atualiza as distâncias automaticamente e calcula as coordenadas
    const tagRef = database.ref("tags/" + id); // Referência da tag
    tagRef.on(
      "value",
      async (snapshot) => {
        const tag = snapshot.val();

        const {
          distance1: r1,
          distance2: r2,
          distance3: r3,
          room,
          color,
        } = tag;

        const [x, y] = computeTagCoordinates(r1, r2, r3);
        devices[id] = { x, y, color, room, type: "tag" };

        updateCanvas();
      },
      (error) => {
        console.log(error);
      }
    );
  });
}

// Obtem as coordenadas da tag, a partir das distâncias
const computeTagCoordinates = (r1, r2, r3) => {
  const room = document.getElementById("room").value;
  const beacon1 = devices[rooms[room].beacon1];
  const beacon2 = devices[rooms[room].beacon2];
  const beacon3 = devices[rooms[room].beacon3];

  // Algoritmo de trilateração
  const x1 = beacon1.x; // Coordenada do eixo X do beacon 1 (em metros)
  const y1 = beacon1.y; // Coordenada do eixo Y do beacon 1 (em metros)
  const x2 = beacon2.x; // Coordenada do eixo X do beacon 2 (em metros)
  const y2 = beacon2.y; // Coordenada do eixo Y do beacon 2 (em metros)
  const x3 = beacon3.x; // Coordenada do eixo X do beacon 3 (em metros)
  const y3 = beacon3.y; // Coordenada do eixo Y do beacon 3 (em metros)

  let x; // Coordenada do eixo X da tag (em metros)
  let y; // Coordenada do eixo Y da tag (em metros)

  const A = 2 * x2 - 2 * x1;
  const B = 2 * y2 - 2 * y1;
  const C = r1 ** 2 - r2 ** 2 - x1 ** 2 + x2 ** 2 - y1 ** 2 + y2 ** 2;
  const D = 2 * x3 - 2 * x2;
  const E = 2 * y3 - 2 * y2;
  const F = r2 ** 2 - r3 ** 2 - x2 ** 2 + x3 ** 2 - y2 ** 2 + y3 ** 2;
  x = (C * E - F * B) / (E * A - B * D);
  y = (C * D - A * F) / (B * D - A * E);

  return [x, y];
};

// MAPA DO LOCAL

// Desenha um ponto vermelho no mapa
const canvas = document.getElementById("map");
const ctx = canvas.getContext("2d");

const updateCanvas = () => {
  // Apaga o canvas
  const canvasSizeX = canvas.width;
  const canvasSizeY = canvas.height;

  const room = document.getElementById("room").value
  if (!rooms[room]) return; // Retorna se a sala não existir no banco de dados
  let roomSizeX = rooms[room].x; // Tamanho da sala (eixo X) em metros (mundo real)
  let roomSizeY = rooms[room].y; // Tamanho da sala (eixo Y) em metros (mundo real)

  ctx.clearRect(0, 0, canvasSizeX, canvasSizeY);

  // Filtra os beacons e tags por sala
  filterByRoom();

  // Loop para desenhar os beacons e tags
  for (const [key, value] of Object.entries(devices)) {
    if (devices[key].show) {
      // Conversão de coordenadas do mundo real para coordenadas do canvas
      const xCanvas = Math.floor(
        Math.max(0, Math.min(canvasSizeX, (value.x / roomSizeX) * canvasSizeX))
      );
      const yCanvas = Math.floor(
        Math.max(0, Math.min(canvasSizeY, (value.y / roomSizeY) * canvasSizeY))
      );
      ctx.beginPath(); // Inicia o desenho
      // Mude o valor de x e y para a posição do local
      ctx.arc(xCanvas, yCanvas, 10, 0, 2 * Math.PI); // Desenha um círculo
      ctx.fillStyle = value.color || "red"; // Cor do ponto
      ctx.fill(); // Desenha o ponto
      ctx.closePath(); // Fecha o desenho
    }
  }
};

// Filtra os beacons e tags por sala
const filterByRoom = () => {
  const room = document.getElementById("room").value;
  for (const [key, value] of Object.entries(devices)) {
    if (value.room === room) {
      devices[key].show = true;
    } else {
      devices[key].show = false;
    }
  }
};

// Dispara filterByRoom quando o usuário seleciona uma sala
document.getElementById("room").addEventListener("change", (event) => {
  getRoomTags();
  document.getElementById("map").style.background =
    `url('${document.getElementById('room').value}.jpeg') no-repeat center center`;
});

getRoomTags();
