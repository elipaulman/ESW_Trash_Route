import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { Line } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js';
import './App.css';

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

  useEffect(() => {
    // Fetch data from backend
    const fetchData = async () => {
      try {
        const response = await axios.get('http://localhost:3000/api/trash-data');
        console.log('Fetched data:', response.data); // Log the fetched data
        setData(response.data);
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };
    fetchData();
  }, []);

  // Get the last 3 measurements
  const lastThreeMeasurements = data.slice(-3);

  // Dumpster dimensions
  const dumpsterHeightFeet = 3.5; // Height of the dumpster in feet
  const dumpsterHeightCm = dumpsterHeightFeet * 30.48; // Convert feet to cm
  const currentDistanceCm =
    lastThreeMeasurements.length > 0
      ? lastThreeMeasurements[lastThreeMeasurements.length - 1].distance
      : dumpsterHeightCm; // Default to full height if no data

  // Calculate fullness level
  const fullnessLevel = Math.max(
    0,
    Math.min(100, ((1 - currentDistanceCm / dumpsterHeightCm) * 100))
  ); // Ensure 0-100%

  // Convert distance to feet and inches
  const currentDistanceFeet = currentDistanceCm / 30.48;
  const feet = Math.floor(currentDistanceFeet);
  const inches = Math.round((currentDistanceFeet - feet) * 12);

  // Temperature conversion
  const currentTemperatureRaw =
    lastThreeMeasurements.length > 0
      ? lastThreeMeasurements[lastThreeMeasurements.length - 1].temperature
      : 'N/A';
  const currentTemperatureCelsius =
    currentTemperatureRaw !== 'N/A' ? currentTemperatureRaw / 10 : 'N/A';
  const currentTemperatureFahrenheit =
    currentTemperatureCelsius !== 'N/A'
      ? (currentTemperatureCelsius * 9) / 5 + 32
      : 'N/A';

  // Chart data
  const chartData = {
    labels: lastThreeMeasurements.map((entry) =>
      new Date(entry.timestamp).toLocaleString()
    ),
    datasets: [
      {
        label: 'Distance (cm)',
        data: lastThreeMeasurements.map((entry) => entry.distance),
        borderColor: 'blue',
        fill: false,
      },
    ],
  };

  // Get the last 5 days worth of data
  const lastFiveDaysData = data.filter((entry) => {
    const entryDate = new Date(entry.timestamp);
    const currentDate = new Date();
    const timeDiff = currentDate - entryDate;
    return timeDiff <= 5 * 24 * 60 * 60 * 1000; // 5 days in milliseconds
  });

  return (
    <div className="dashboard">
      <h1>Trash Level Dashboard</h1>
      {data.length > 0 ? (
        <>
          <div className="chart-container">
            <Line data={chartData} />
          </div>
          <div className="info">
            <p>Current Fullness Level: {fullnessLevel.toFixed(2)}%</p>
            <p>Current Distance: {feet} feet {inches} inches</p>
          </div>
          <div className="table-container">
            <h2>Last 5 Days Data</h2>
            <table className="data-table">
              <thead>
                <tr>
                  <th>Time</th>
                  <th>Fullness (%)</th>
                  <th>Distance (cm)</th>
                </tr>
              </thead>
              <tbody>
                {lastFiveDaysData.map((entry) => {
                  const entryDistanceCm = entry.distance;
                  const entryFullnessLevel = Math.max(
                    0,
                    Math.min(100, ((1 - entryDistanceCm / dumpsterHeightCm) * 100))
                  );
                  return (
                    <tr key={entry.timestamp}>
                      <td>{new Date(entry.timestamp).toLocaleString()}</td>
                      <td>{entryFullnessLevel.toFixed(2)}</td>
                      <td>{entry.distance}</td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </div>
        </>
      ) : (
        <p>Loading data...</p>
      )}
    </div>
  );
};

export default App;