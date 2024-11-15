const express = require('express');
const bodyParser = require('body-parser');
const mongoose = require('mongoose');

const app = express();
const PORT = 3000;

// MongoDB connection
mongoose.connect('mongodb+srv://paulman2:H3BF1bgtQgbKiERF@cluster0.yaaap.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0');

// Define the schema and model
const trashSchema = new mongoose.Schema({
  distance: Number,
  flux: Number,
  temperature: Number,
  timestamp: { type: Date, default: Date.now },
});

const TrashData = mongoose.model('TrashData', trashSchema);

app.use(bodyParser.json());

// API endpoint to save data
app.post('/api/trash-data', async (req, res) => {
  const data = new TrashData(req.body);
  try {
    await data.save();
    res.status(201).send('Data saved successfully');
  } catch (error) {
    console.error(error);
    res.status(500).send('Error saving data');
  }
});

// API endpoint to get all data
app.get('/api/trash-data', async (req, res) => {
  try {
    const data = await TrashData.find({});
    res.json(data);
  } catch (error) {
    console.error(error);
    res.status(500).send('Error retrieving data');
  }
});

app.listen(PORT, () => console.log(`Server running on http://localhost:${PORT}`));
