import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { Line } from 'react-chartjs-2';

const App = () => {
  const [data, setData] = useState([]);

  useEffect(() => {
    // Fetch data from backend
    const fetchData = async () => {
      try {
        const response = await axios.get('http://localhost:3000/api/trash-data');
        setData(response.data);
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    };
    fetchData();
  }, []);

  const chartData = {
    labels: data.map((entry) => new Date(entry.timestamp).toLocaleString()),
    datasets: [
      {
        label: 'Distance (cm)',
        data: data.map((entry) => entry.distance),
        borderColor: 'blue',
        fill: false,
      },
    ],
  };

  return (
    <div style={{ padding: '20px' }}>
      <h1>Trash Level Dashboard</h1>
      {data.length > 0 ? (
        <Line data={chartData} />
      ) : (
        <p>Loading data...</p>
      )}
    </div>
  );
};

export default App;
