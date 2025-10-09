// === Configuración de Firebase ===
const firebaseConfig = {
  apiKey: "AIzaSyB8tP9FFzGB3-j3uv5GW3oZCIk-sImZ-3M",
  authDomain: "espcar32.firebaseapp.com",
  databaseURL: "https://espcar32-default-rtdb.firebaseio.com",
  projectId: "espcar32",
  storageBucket: "espcar32.appspot.com",
  messagingSenderId: "749230310167",
  appId: "1:749230310167:web:3cc80f065253438d8b6d52",
  measurementId: "G-NCSQJ3NQ2T"
};

firebase.initializeApp(firebaseConfig);
const db = firebase.database();

let ruta = [];

function enviarComando(cmd) {
  db.ref("modo").set("manual");
  db.ref("comando").set(cmd);

  // Agregar también a la ruta
  ruta.push(cmd);
  mostrarRutaEnPantalla();

  // Guardar en Firebase la nueva ruta actualizada
  db.ref("ruta").set(ruta);

  console.log("📤 Comando enviado y guardado en ruta:", cmd);
}


function agregarARuta(paso) {
  ruta.push(paso);
  mostrarRutaEnPantalla();
  console.log("📝 Ruta:", ruta);
}

function mostrarRutaEnPantalla() {
  const contenedor = document.getElementById("rutaLista");
  contenedor.textContent = "Ruta actual: " + (ruta.length ? ruta.join(" → ") : "(vacía)");
}

function guardarRuta() {
  if (!ruta.length) return alert("⚠️ No hay pasos en la ruta.");
  db.ref("ruta").set(ruta).then(() => {
    alert("✅ Ruta guardada.");
  });
}

function ejecutarRuta() {
  db.ref("ejecutarRuta").set(true).then(() => {
    alert("🚀 Ruta en ejecución.");
  });
}

function borrarRuta() {
  ruta = [];
  mostrarRutaEnPantalla();
  db.ref("ruta").remove();
  db.ref("ejecutarRuta").set(false);
  alert("🗑️ Ruta eliminada.");
}

function borrarUltimoPaso() {
  if (ruta.length === 0) {
    alert("❌ No hay pasos para borrar.");
    return;
  }

  const ultimo = ruta.pop(); // Elimina el último paso

  // Revertir la acción en el carro
  let accionReversa = "";

  switch (ultimo) {
    case "adelante": accionReversa = "atras"; break;
    case "atras": accionReversa = "adelante"; break;
    case "izquierda": accionReversa = "derecha"; break;
    case "derecha": accionReversa = "izquierda"; break;
    case "detener": accionReversa = "detener"; break; // O nada
    default: accionReversa = "detener"; break;
  }

  // Enviar comando inverso al carro
  db.ref("modo").set("manual");
  db.ref("comando").set(accionReversa);
  db.ref("ruta").set(ruta);

  mostrarRutaEnPantalla();
  console.log("🧹 Paso eliminado:", ultimo, " | Acción revertida:", accionReversa);
}
