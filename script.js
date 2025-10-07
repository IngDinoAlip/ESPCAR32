// 🔧 Configuración de Firebase (tu proyecto)
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

// 🔌 Inicializar Firebase
firebase.initializeApp(firebaseConfig);
const db = firebase.database();

// 📈 Crear gráficas con Chart.js
const co2Chart = new Chart(document.getElementById("co2Chart"), {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      label: "CO₂ (ppm)",
      data: [],
      borderColor: "red",
      fill: false
    }]
  }
});

const tempChart = new Chart(document.getElementById("tempChart"), {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      label: "Temp (°C)",
      data: [],
      borderColor: "orange",
      fill: false
    }]
  }
});

const humChart = new Chart(document.getElementById("humChart"), {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      label: "Humedad (%)",
      data: [],
      borderColor: "blue",
      fill: false
    }]
  }
});

// 🧾 Tabla de datos
const dataTable = document.querySelector("#dataTable tbody");

// 🔄 Leer datos en tiempo real desde Firebase
db.ref("datos").limitToLast(20).on("value", (snapshot) => {
  const data = snapshot.val();
  if (!data) return;

  // Limpiar gráficas y tabla
  co2Chart.data.labels = [];
  co2Chart.data.datasets[0].data = [];

  tempChart.data.labels = [];
  tempChart.data.datasets[0].data = [];

  humChart.data.labels = [];
  humChart.data.datasets[0].data = [];

  dataTable.innerHTML = "";

  // Recorrer los datos
  Object.values(data).forEach((entry) => {
    const time = entry.timestamp || "—";

    // Agregar a gráficas
    co2Chart.data.labels.push(time);
    co2Chart.data.datasets[0].data.push(entry.co2);

    tempChart.data.labels.push(time);
    tempChart.data.datasets[0].data.push(entry.temperatura);

    humChart.data.labels.push(time);
    humChart.data.datasets[0].data.push(entry.humedad);

    // Agregar a tabla
    const row = `<tr>
      <td>${time}</td>
      <td>${entry.co2}</td>
      <td>${entry.temperatura}</td>
      <td>${entry.humedad}</td>
    </tr>`;
    dataTable.innerHTML += row;
  });

  // Actualizar las gráficas
  co2Chart.update();
  tempChart.update();
  humChart.update();
});
