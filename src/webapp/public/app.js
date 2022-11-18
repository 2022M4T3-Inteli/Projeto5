// FIREBASE REAL TIME DATABASE

// Id única da tag (endereço MAC)
let macAddress = "F4:12:FA:E2:43:10";

// Caminhos do banco de dados
let distance1Path = `tags/${macAddress}/distance1`;
let distance2Path = `tags/${macAddress}/distance2`;
let distance3Path = `tags/${macAddress}/distance3`;

// Obter referências do banco de dados
const distance1Ref = database.ref(distance1Path);
const distance2Ref = database.ref(distance2Path);
const distance3Ref = database.ref(distance3Path);

// Obtem as coordenadas da tag
const getCoordinates = async () => {
  let r1, r2, r3;
  // Faz a leitura dos dados no banco
  await Promise.all([
    distance1Ref.once("value", (snapshot) => {
      r1 = snapshot.val();
      document.getElementById("r1").innerHTML = Math.round(r1 * 100) / 100;
    }),
    distance2Ref.once("value", (snapshot) => {
      r2 = snapshot.val();
      document.getElementById("r2").innerHTML = Math.round(r2 * 100) / 100;
    }),
    distance3Ref.once("value", (snapshot) => {
      r3 = snapshot.val();
      document.getElementById("r3").innerHTML = Math.round(r3 * 100) / 100;
    }),
  ]);

  // Algoritmo de trilateração (ALTERAR VALORES CONFORME BEACONS)
  const x1 = 0; // Coordenada do eixo X do beacon 1 (em metros)
  const y1 = 0; // Coordenada do eixo Y do beacon 1 (em metros)
  const x2 = 0; // Coordenada do eixo X do beacon 2 (em metros)
  const y2 = 2; // Coordenada do eixo Y do beacon 2 (em metros)
  const x3 = 2; // Coordenada do eixo X do beacon 3 (em metros)
  const y3 = 1; // Coordenada do eixo Y do beacon 3 (em metros)
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

  // Round para 2 casas decimais
  x = Math.round(x * 100) / 100;
  y = Math.round(y * 100) / 100;
  document.getElementById("x").innerHTML = x;
  document.getElementById("y").innerHTML = y;
  return [x, y];
};

// MAPA DO LOCAL

// Background do canvas (mapa do local))
document.getElementById("map").style.background =
  "url('mapa.jpeg') no-repeat center center";

// Desenha um ponto vermelho no mapa
const canvas = document.getElementById("map");
const ctx = canvas.getContext("2d");

let roomSizeX = 5; // Tamanho da sala (eixo X) em metros (mundo real)
let roomSizeY = 5; // Tamanho da sala (eixo Y) em metros (mundo real)

let canvasSizeX = 500; // Tamanho da sala (eixo X) em pixels (canvas)
let canvasSizeY = 500; // Tamanho da sala (eixo Y) em pixels (canvas)

const updateCanvas = async () => {
  // Conversão de coordenadas do mundo real para coordenadas do canvas
  const [xReal, yReal] = await getCoordinates();
  // Previnir que coordenadas sejam negativas
  const xCanvas = Math.floor(
    Math.max(0, Math.min(canvasSizeX, (xReal / roomSizeX) * canvasSizeX))
  );
  const yCanvas = Math.floor(
    Math.max(0, Math.min(canvasSizeY, (yReal / roomSizeY) * canvasSizeY))
  );

  // Apaga o canvas
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  ctx.beginPath(); // Inicia o desenho
  // Mude o valor de x e y para a posição do local
  ctx.arc(xCanvas, yCanvas, 10, 0, 2 * Math.PI); // Desenha um círculo
  ctx.fillStyle = "red"; // Cor do ponto
  ctx.fill(); // Desenha o ponto
  ctx.closePath(); // Fecha o desenho
};

updateCanvas(); // Desenha o ponto no canvas
