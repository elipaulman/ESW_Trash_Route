const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const mongoose = require('mongoose');
require('dotenv').config(); // Add this line

const app = express();
const PORT = 3000;

// Enable CORS
app.use(cors());

mongoose.connect(process.env.MONGO_URI, {
  useNewUrlParser: true,
  useUnifiedTopology: true,
});

const trashSchema = new mongoose.Schema({
  distance: Number,
  flux: Number,
  temperature: Number,
  timestamp: { type: Date, default: Date.now },
});

const TrashData = mongoose.model('TrashData', trashSchema);

app.use(bodyParser.json());

app.post('/api/trash-data', async (req, res) => {
  const data = new TrashData(req.body);
  try {
    await data.save();
    res.status(201).send('Data saved successfully');
  } catch (error) {
    res.status(500).send('Error saving data');
  }
});

app.get('/api/trash-data', async (req, res) => {
  try {
    const data = await TrashData.find({});
    res.json(data);
  } catch (error) {
    res.status(500).send('Error retrieving data');
  }
});

app.listen(PORT, () => console.log(`Server running on http://localhost:${PORT}`));