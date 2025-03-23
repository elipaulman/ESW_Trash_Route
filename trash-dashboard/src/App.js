import React, { useEffect, useState } from "react";
import axios from "axios";
import { Line } from "react-chartjs-2";
import Select from "react-select";
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from "chart.js";
import "./App.css";

// Register the components
ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

const App = () => {
  const [data, setData] = useState([]);
  const [selectedDumpster, setSelectedDumpster] = useState(null);
  const [dumpsterOptions, setDumpsterOptions] = useState([]);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [darkMode, setDarkMode] = useState(false);

  useEffect(() => {
    // Fetch data from backend
    const fetchData = async () => {
      setLoading(true);
      setError(null);
      try {
        const response = await axios.get(
          "https://esw-trash-route.onrender.com/api/trash-data"
        );
        console.log("Fetched data:", response.data); // Log the fetched data
        setData(response.data);

        // Extract unique dumpster names
        const uniqueNames = [
          ...new Set(response.data.map((item) => item.name)),
        ];
        const options = uniqueNames.map((name) => ({
          value: name,
          label: name,
        }));
        setDumpsterOptions(options);

        // Set the first dumpster as the selected one by default
        if (options.length > 0) {
          setSelectedDumpster(options[0]);
        }
      } catch (error) {
        console.error("Error fetching data:", error);
        setError("Failed to fetch data. Please try again later.");
      }
      setLoading(false);
    };
    fetchData();
  }, []);

  useEffect(() => {
    if (selectedDumpster) {
      const fetchData = async () => {
        setLoading(true);
        setError(null);
        const timeoutId = setTimeout(() => {
          setError("Fetching data is taking longer than expected...");
        }, 60000); // 60 seconds timeout

        try {
          const response = await axios.get(
            `https://esw-trash-route.onrender.com/api/trash-data?name=${selectedDumpster.value}`
          );
          clearTimeout(timeoutId);
          console.log("Fetched data:", response.data);
          setData(response.data);
        } catch (error) {
          clearTimeout(timeoutId);
          console.error("Error fetching data:", error);
          setError("Failed to fetch data. Please try again later.");
        }
        setLoading(false);
      };
      fetchData();
    }
  }, [selectedDumpster]);

  // Filter data for the selected dumpster
  const filteredData = data.filter(
    (item) => item.name === selectedDumpster?.value
  );

  // Get the last 5 measurements
  const lastFiveMeasurements = filteredData.slice(-5);

  // Dumpster dimensions
  const dumpsterHeightFeet = 6; // Height of the dumpster in feet
  const dumpsterWidthFeet = 5; // Width of the dumpster in feet
  const dumpsterDiagonalFeet = Math.sqrt(
    dumpsterHeightFeet ** 2 + dumpsterWidthFeet ** 2
  ); // Diagonal in feet
  const dumpsterDiagonalCm = dumpsterDiagonalFeet * 30.48; // Convert feet to cm
  const currentDistanceCm =
    lastFiveMeasurements.length > 0
      ? lastFiveMeasurements[lastFiveMeasurements.length - 1].distance
      : dumpsterDiagonalCm; // Default to full height if no data

  // Calculate fullness level
  const fullnessLevel = Math.max(
    0,
    Math.min(100, (1 - currentDistanceCm / dumpsterDiagonalCm) * 100)
  ); // Ensure 0-100%

  // Convert distance to feet and inches
  const currentDistanceFeet = currentDistanceCm / 30.48;
  const feet = Math.floor(currentDistanceFeet);
  const inches = Math.round((currentDistanceFeet - feet) * 12);

  // Temperature conversion
  const currentTemperatureRaw =
    lastFiveMeasurements.length > 0
      ? lastFiveMeasurements[lastFiveMeasurements.length - 1].temperature
      : "N/A";
  const currentTemperatureCelsius =
    currentTemperatureRaw !== "N/A" ? currentTemperatureRaw / 10 : "N/A";
  const currentTemperatureFahrenheit =
    currentTemperatureCelsius !== "N/A"
      ? (currentTemperatureCelsius * 9) / 5 + 32
      : "N/A";

  // Function to get the day of the week
  const getDayOfWeek = (date) => {
    const days = ["Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat"];
    return days[date.getDay()];
  };

  // Calculate fullness level for each data point
  const fullnessLevels = lastFiveMeasurements.map((entry) => {
    // Calculate unclamped fullness level (can be negative)
    const rawFullness = (1 - entry.distance / dumpsterDiagonalCm) * 100;
    
    // For display in the chart, clamp to 0-100%
    return Math.max(0, Math.min(100, rawFullness));
  });

  // Chart data
  const chartData = {
    labels: lastFiveMeasurements.map((entry) => {
      const date = new Date(entry.timestamp);
      return `${date.toLocaleString()} (${getDayOfWeek(date)})`;
    }),
    datasets: [
      {
        label: "Fullness Level (%)",
        data: fullnessLevels,
        borderColor: "#bb0000", // Ohio State Scarlet
        backgroundColor: "rgba(187, 0, 0, 0.2)", // Light Scarlet background
        fill: true,
        tension: 0, // Set to 0 for straight lines (no curve)
        pointRadius: 6, // Increase point size for better visibility
        pointHoverRadius: 8,
        spanGaps: false,
        pointBackgroundColor: "#bb0000", // All points are red
        borderWidth: 2,
      },
    ],
  };

  // Chart options
  const chartOptions = {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      y: {
        beginAtZero: true, // Start the y-axis at 0
        min: 0,
        max: 100, // Fullness level percentage
        ticks: {
          stepSize: 20, // Create ticks at 0, 20, 40, 60, 80, 100
          callback: function (value) {
            return `${value}%`;
          },
        },
        grid: {
          drawBorder: true,
          color: function(context) {
            if (context.tick.value === 0) {
              return 'rgba(0, 0, 0, 0.3)'; // Make the zero line more visible
            }
            return 'rgba(0, 0, 0, 0.1)';
          },
        },
      },
      x: {
        grid: {
          display: true,
        },
        ticks: {
          maxRotation: 45,
          minRotation: 45,
        },
      },
    },
    elements: {
      point: {
        radius: 6, // Make all points visible with consistent size
        hitRadius: 12,
        hoverRadius: 8,
        borderWidth: 2,
        backgroundColor: "#bb0000", // All points are red
      },
      line: {
        tension: 0, // Ensure straight lines between points
        borderWidth: 2,
      },
    },
    plugins: {
      legend: {
        display: true,
        position: 'top',
      },
      tooltip: {
        callbacks: {
          label: function(context) {
            const index = context.dataIndex;
            const entry = lastFiveMeasurements[index];
            const rawFullness = (1 - entry.distance / dumpsterDiagonalCm) * 100;
            const displayFullness = Math.max(0, rawFullness).toFixed(2);
            const distanceFeet = (entry.distance / 30.48).toFixed(2);
            
            return [
              `Fullness: ${displayFullness}%`,
              `Distance: ${distanceFeet} ft`
            ];
          }
        }
      }
    },
    animation: {
      duration: 0 // Disable animations for clearer rendering
    },
    layout: {
      padding: {
        left: 10,
        right: 30,
        top: 20,
        bottom: 10
      }
    }
  };

  // Get the last 5 days worth of data
  const lastFiveDaysData = filteredData
    .filter((entry) => {
      const entryDate = new Date(entry.timestamp);
      const currentDate = new Date();
      const timeDiff = currentDate - entryDate;
      return timeDiff <= 5 * 24 * 60 * 60 * 1000; // 5 days in milliseconds
    })
    .sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp)); // Sort by timestamp in descending order

  // Export data to CSV
  const exportToCSV = () => {
    const csvData = [
      ["Time", "Fullness (%)", "Distance (ft)"],
      ...lastFiveDaysData.map((entry) => {
        const entryDistanceCm = entry.distance;
        const entryDistanceFeet = entryDistanceCm / 30.48;
        const entryFullnessLevel = Math.max(
          0,
          Math.min(100, (1 - entryDistanceCm / dumpsterDiagonalCm) * 100)
        );
        return [
          new Date(entry.timestamp).toLocaleString(),
          entryFullnessLevel.toFixed(2),
          entryDistanceFeet.toFixed(2),
        ];
      }),
    ];
    const csvContent = `data:text/csv;charset=utf-8,${csvData
      .map((e) => e.join(","))
      .join("\n")}`;
    const encodedUri = encodeURI(csvContent);
    const currentDate = new Date().toISOString().split("T")[0]; // Get only the date part
    const fileName = `${
      selectedDumpster?.value || "dumpster"
    }_${currentDate}.csv`;
    const link = document.createElement("a");
    link.setAttribute("href", encodedUri);
    link.setAttribute("download", fileName);
    document.body.appendChild(link);
    link.click();
  };

  return (
    <div className={`dashboard ${darkMode ? "dark-mode" : ""}`}>
      <h1>Trash Level Dashboard</h1>
      <button onClick={() => setDarkMode(!darkMode)}>
        {darkMode ? "Light Mode" : "Dark Mode"}
      </button>
      <Select
        value={selectedDumpster}
        onChange={setSelectedDumpster}
        options={dumpsterOptions}
        className="dumpster-select"
        placeholder="Select Dumpster"
      />
      {loading ? (
        <div className="spinner">Loading...</div>
      ) : error ? (
        <p className="error">{error}</p>
      ) : filteredData.length > 0 ? (
        <>
          <div className="chart-container" style={{ height: "400px" }}>
            <Line data={chartData} options={chartOptions} />
          </div>
          <div className="info">
            <p>Current Fullness Level: {fullnessLevel.toFixed(2)}%</p>
            <p>
              Current Distance: {feet} feet {inches} inches
            </p>
            {currentTemperatureCelsius !== "N/A" && (
              <p>
                Temperature: {currentTemperatureCelsius.toFixed(1)}°C ({currentTemperatureFahrenheit.toFixed(1)}°F)
              </p>
            )}
          </div>
          <div className="table-container">
            <h2>Last 5 Days Data</h2>
            <button onClick={exportToCSV}>Export to CSV</button>
            <table className="data-table">
              <thead>
                <tr>
                  <th>Time</th>
                  <th>Fullness (%)</th>
                  <th>Distance (ft)</th>
                </tr>
              </thead>
              <tbody>
                {lastFiveDaysData.map((entry) => {
                  const entryDistanceCm = entry.distance;
                  const entryDistanceFeet = entryDistanceCm / 30.48;
                  const entryFullnessLevel = Math.max(
                    0,
                    Math.min(
                      100,
                      (1 - entryDistanceCm / dumpsterDiagonalCm) * 100
                    )
                  );
                  return (
                    <tr key={entry.timestamp}>
                      <td>
                        {new Date(entry.timestamp).toLocaleString()} (
                        {getDayOfWeek(new Date(entry.timestamp))})
                      </td>
                      <td>{entryFullnessLevel.toFixed(2)}%</td>
                      <td>{entryDistanceFeet.toFixed(2)} ft</td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </div>
        </>
      ) : (
        <p>No data available.</p>
      )}
    </div>
  );
};

export default App;