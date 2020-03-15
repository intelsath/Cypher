// Dependencies
const bip39 = require('bip39');
const bch = require('bitcoincashjs');

// coin
var wallet_id = process.argv[2];
var coin_lib = "BitcoinCash";

// Function to generate keys
module.exports.generate_key = function(mnemonic,passphrase,path){
    if(bip39.validateMnemonic(mnemonic)){
        var seed = bip39.mnemonicToSeed(mnemonic, passphrase);
        var root = new bch.HDPrivateKey.fromSeed(seed);
        var privateKey = root.derive("m/0/" + path.toString()).privateKey;
        var out = {
            coin: coin_lib,
            //address: privateKey.toAddress().toString(),
			address: privateKey.toAddress(bch.Networks.testnet).toString(),
            publicKey: new bch.PublicKey(privateKey).toString(),
            privateKey: privateKey.toWIF()
        };

        fk_select = "(SELECT coin_id FROM coin WHERE coin_name = '" + out['coin'] + "')"; // Foreign Key
		sqlquery = "INSERT INTO addresses (address_path, wallet_id, coin_id, public_key, address, private_key) VALUES(" + path.toString() + "," + wallet_id + "," + fk_select + ",'" + out['publicKey'] + "'," + "'" + out['address'] + "'," + "'" + out['privateKey'] + "'" + ")"; // Save hex string 

		db.run(sqlquery); // Execute query, add new address to db
		console.log(out);
	}else{
		console.log("error");
	}
}

// Open DB
const sqlite3 = require('sqlite3').verbose();
let db = new sqlite3.Database('../cypher.db', (err) => {
  if (err) {
    console.error(err.message);
  }
  console.log('Connected to the cypher database.');
});

// Get mnemonic from SQLlite database
let sqlquery1 = 'SELECT mnemonic FROM wallets WHERE ID = ' + wallet_id + ';';
db.each(sqlquery1, (err, row) => {
	if (err) {
		throw err;
	}
	mnemonic = row.mnemonic; // Get mnemonic number (hex string)
	console.log(mnemonic);

	// Convert seed to mnemonic
	const passphrase = "password";
	// Print mnemonic
	console.log(mnemonic);

	// Addressnum (for path)
	fk_select = "(SELECT coin_id FROM coin WHERE coin_name = '" + coin_lib + "')"; // Foreign Key
	let sqlquery2 = "SELECT address_path FROM addresses WHERE coin_id = " + fk_select + " AND wallet_id = " + wallet_id + " ORDER BY ID DESC LIMIT 1;";

	// Get last id (path derivation) 
	db.each(sqlquery2, (err, row) => {
		if (err) {
			throw err;
		}
		path_id = row.address_path + 1; // Get path (last id) and adds 1
		console.log("path: " + path_id);

		// Generate keys!
		module.exports.generate_key(mnemonic,passphrase,path_id); 
	}, function(err, rows) {
		if (rows == 0) { // Have we not created any address for this coin yet?
			// Now rows!
			console.log("empty!");
			module.exports.generate_key(mnemonic,passphrase,0); 
		}
	});

});

// Return status (0 -- exit was succesful)
return 0;