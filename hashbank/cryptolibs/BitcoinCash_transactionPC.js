const request = require('request');
const coinSelect = require('coinselect');
const apiUrl = "https://test-bch-insight.bitpay.com/api/addrs/";

var address_to = process.argv[2];
var amount_sending = parseFloat(process.argv[3]) * 100000000; // Expect arg in bitcoin cash, convert it to satoshis
var fee = parseFloat(process.argv[4])* 100000000;             // Expect arg in bitcoin cash, convert it to satoshis
var address_change = ""; // This will be taken from the db

// Tmp file to store output
var tmpfile = process.argv[5];

// faucet: https://testnet.manu.backend.hamburg/bitcoin-cash-faucet

module.exports.prepare_transaction = async function(){
    const wallet_id = "1";
    var addresses = "";
    await getAddresses(wallet_id, 2).then(function(rows){
        //console.log(rows);
        for(var i=0; i<rows.length; i++){
            if(rows.length-1 == i){
                addresses += rows[i].address;
            }else{
                addresses += rows[i].address + ",";
            }

            // Use last created address as address_change
			if( i == rows.length-1 )
                address_change = rows[i].address;
        }
    });
    // UTXO request the addresses
    /*request.get(apiUrl + addresses + '/utxo', { json: true }, (err, req, body) => {
        console.log(apiUrl + addresses + '/utxo');
        /*for(var i=0; i<body.length; i++){
            //utxos.push({script: body[i].scriptPubKey, address: body[i].address, txId: body[i].txid, vout: body[i].vout, value: body[i].satoshis});
        }*/
        var utxos=[
            {   
                address:"mwa7QwsDnLbtdQHUvh1a8NvjAX3Chrt517",
                txId:"166cfa221e3268b3dfdfb212e1b03f8624e30a589045fdd9b19a07b657ee8f04",
                vout:1,
                script:"76a914b01adf58ed369f85ec00ebb277596ab182162aa588ac",
                value:67840000
            },
            {   
                address:"mwa7QwsDnLbtdQHUvh1a8NvjAX3Chrt517",
                txId:"87c291991c5d04203bc172cf19b2504b410d1944defcc590f0555f9d3a3d0d7a",
                vout:1,
                script:"76a914b01adf58ed369f85ec00ebb277596ab182162aa588ac",
                value:220000000
            },
            { 
                address:"n3erLV88x3RG658wW4uvYsKinXU8z3g8iN",
                txId:"2da4d833d3a00ff4fbbcc48eb9e46def06ce0c970a341375bacc090ea0b154e3",
                vout:1,
                script:"76a914f2d11f133c1d192b46dbeab671388321b894bcc488ac",
                value:67840000
            },
            { 
                address:"n3erLV88x3RG658wW4uvYsKinXU8z3g8iN",
                txId:"e073955e8692d584cd4e8556a772b5d781919e4a30f04e5442367d340edd7f0d",
                vout:0,
                script:"76a914f2d11f133c1d192b46dbeab671388321b894bcc488ac",
                value:400000000
            },
            { 
                address:"mfkhLv2o9PJQ1AtrjDbRLGNtoZLq6iXzuH",
                txId:"69b50ead1db42c29fffefa1416e1d1a2cae2a0ff2898155d4638d96b8639ee22",
                vout:0,
                script:"76a914029957e0cdb56f638fcf3721a55a1eaae53937d388ac",
                value:13450000
            },
            {
                address:"mfkhLv2o9PJQ1AtrjDbRLGNtoZLq6iXzuH",
                txId:"e073955e8692d584cd4e8556a772b5d781919e4a30f04e5442367d340edd7f0d",
                vout:1,
                script:"76a914029957e0cdb56f638fcf3721a55a1eaae53937d388ac",
                value:119900000,
            }
        ];

        let targets = [{
                address: address_to,
                value: amount_sending
        }];

        let { inputs, outputs } = coinSelect(utxos, targets, 0);

        if (!inputs || !outputs){
            return;
        }else{
            //console.log(outputs);

			// Add outputs to JSON obj (Inputs + Outputs)
			inputs.push({address_to: outputs[0].address, value_send:outputs[0].value, address_change: address_change, value_change:outputs[1].value, fee: fee});

			// Return all inputs in json format
			// console.log(inputs);

			// Write to file
			var fs = require('fs');
			fs.writeFile(tmpfile, JSON.stringify(inputs), function(err) {
				if(err) {
					return console.log(err);
				}

				//console.log("The file was saved!");
			});

        }
    //});
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
  //console.log('Connected to the cypher database.');
});

module.exports.prepare_transaction();