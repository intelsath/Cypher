
const request = require('request');
const coinSelect = require('coinselect');
const apiUrl = "https://testnet.blockexplorer.com/api/addrs/";

/*
const address_to = "mnkmgZcaFctk5HQEKb661yvUt9vSVz84GL";
const amount_sending = 200000000;
var fee = 0.02;
const address_change = "myfA194ui22hm91ERvQiGnXy7uQzPQTF7P";
faucet: https://testnet.manu.backend.hamburg/faucet
push transaction to: https://live.blockcypher.com/btc-testnet/pushtx/
*/

var address_to = process.argv[2];
var amount_sending = parseFloat(process.argv[3]) * 100000000; // Expect arg in bitcoins, convert it to satoshis
var fee = parseFloat(process.argv[4])* 100000000;             // Expect arg in bitcoins, convert it to satoshis
var address_change = ""; // This will be taken from the db

// Tmp file to store output
var tmpfile = process.argv[5];

module.exports.prepare_transaction = async function(){
    const wallet_id = "1";
    var addresses = "";
    await getAddresses(wallet_id, 1).then(function(rows){
        for(var i=0; i<rows.length; i++){
            if(rows.length-1 == i){
                addresses += rows[i].address;
            }else{
                addresses += rows[i].address + ",";
            }			
			// // Use last created address as address_change
			// if( i == rows.length-1 )
			// 	address_change = rows[i].address;
        }
    });

    // UTXO request the addresses
    request.get(apiUrl + addresses + '/utxo', { json: true, rejectUnauthorized: false }, (err, req, body) => {
        var utxos=[];

        for(var i=0; i<body.length; i++){
            utxos.push({address: body[i].address, txId: body[i].txid, vout: body[i].vout, value: body[i].satoshis});
        }
        let targets = [{
                address: address_to,
                value: amount_sending + fee
        }];
        let { inputs, outputs } = coinSelect(utxos, targets, 0);

        outputs[0].value -= fee;

        // Use last utxo address as address_change
        address_change = inputs[inputs.length - 1].address;

        if (!inputs || !outputs){
            return;
        }else{
            if(fee != 0){
                outputs[outputs.length-1].value = outputs[outputs.length-1].value - (fee);
            }

            if(outputs[1]){
                // Add outputs to JSON obj (Inputs + Outputs)
			    inputs.push({address_to: outputs[0].address, value_send:outputs[0].value, address_change: address_change, value_change:outputs[1].value});
            }else{
                // Add outputs to JSON obj (Inputs + Outputs)
			    inputs.push({address_to: outputs[0].address, value_send:outputs[0].value, address_change: address_change, value_change:0});
            }

			// Return all inputs in json format
			// console.log(inputs);

			// Write to file
			var fs = require('fs');
			fs.writeFile(tmpfile, JSON.stringify(inputs), function(err) {
				if(err) {
					return console.log(err);
				}

				// console.log("The file was saved!");
			});

        }
    });
}

function getAddresses(wallet_id, coin_id){
    return new Promise(function(resolve, reject){
        db.all("SELECT address FROM addresses WHERE wallet_id = ? AND coin_id = ? AND address NOT LIKE ?;", [wallet_id, coin_id, address_to], (error, rows)=>{
            if(error)
                reject(error);
            else 
                resolve(rows);
        })
    });
}

// Open DB
const sqlite3 = require('sqlite3').verbose();
let db = new sqlite3.Database('./cypher.db', (err) => {
  if (err) {
    console.error(err.message);
  }
  // console.log('Connected to the cypher database.');
});

module.exports.prepare_transaction();