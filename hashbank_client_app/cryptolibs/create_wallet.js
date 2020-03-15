// Create a new wallet (new mnemonic) and save it to db
var wltname = process.argv[2];

const bip39 = require('bip39');
// Open DB
const sqlite3 = require('sqlite3').verbose();
let db = new sqlite3.Database('../cypher.db', (err) => {
  if (err) {
    console.error(err.message);
  }
  console.log('Connected to the cypher database.');
});

// Come up with mnemonic 
// NOTE: This function needs to be modified as Cypher will obtain its entropy from /dev/random + user interaction (from accelerometer)
var mnemonic = bip39.generateMnemonic(256)
// Get hex number
//var hex_num = bip39.mnemonicToEntropy(mnemonic);

// Print to console
console.log(mnemonic);
//console.log(hex_num);

// Save to db
sqlquery = "INSERT INTO wallets (mnemonic, wallet_name) VALUES('" + mnemonic + "','" + wltname + "')"; // Save hex string 
db.run(sqlquery);


// Return status (0 -- exit was succesful)
return 0;