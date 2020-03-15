const litecore = require('litecore-lib');
const request = require('request');
const coinSelect = require('coinselect');
const apiUrl = "https://chain.so/api/v2/get_tx_unspent/LTCTEST/";

// faucet: http://testnet.litecointools.com

var address_to = process.argv[2];
var amount_sending = parseFloat(process.argv[3]) * 100000000; // Expect arg in litecoins, convert it to satoshis
var fee = parseFloat(process.argv[4])* 100000000;             // Expect arg in litecoins, convert it to satoshis
var address_change = ""; // This will be taken from the db

// Tmp file to store output
var tmpfile = process.argv[5];

module.exports.prepare_transaction = async function(){
    const wallet_id = "1";
    var addresses = [];
    var utxos_tmp = [];
    await getAddresses(wallet_id, 3).then(function(rows){
        addresses = rows;
        // Use last created address as address_change
        address_change = rows[rows.length-1].address;
    });
    for(const address of addresses){
        await getUTXO(address).then(function(utxos_address){
            utxos_tmp.push.apply(utxos_tmp, utxos_address);
        });
    }

    var inputs = [], value_change = 0;
    var total = amount_sending + fee;
    for(var i=0; i<utxos_tmp.length; i++){
        if(total <= utxos_tmp[i].value){
            value_change = utxos_tmp[i].value - total;
            total -= total;
            inputs.push(utxos_tmp[i]);
            break;
        }else{
            inputs.push(utxos_tmp[i]);
            total -= utxos_tmp[i].value;
        }
    }

    let outputs = [{
            address: address_to,
            value: amount_sending
    }];

    if (!inputs || !outputs){
        return;
    }else{
        var utxos_sign = [];
        for(const utxo of inputs){
            utxos_sign.push({txId: utxo.txId, outputIndex: utxo.vout, address: utxo.address, script: utxo.script, amount: utxo.value / 100000000});
        }
            
        // Add outputs to JSON obj (Inputs + Outputs)
        utxos_sign.push({address_to: outputs[0].address, value_send:outputs[0].value, address_change: address_change, value_change:value_change, fee: fee});

        // Return all inputs in json format
        // console.log(utxos_sign);

        // Write to file
        var fs = require('fs');
        fs.writeFile(tmpfile, JSON.stringify(utxos_sign), function(err) {
            if(err) {
                return console.log(err);
            }

            //console.log("The file was saved!");
        });

    }
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

function getUTXO(address){
    return new Promise(function(resolve, reject){
        address = address.address;
        request.get(apiUrl + address, { json: true, rejectUnauthorized: false }, (err, req, body) => {
            if(err){
                reject(err);
            }else{
                var utxos = [];
                for(var i=0; i<body.data.txs.length; i++){
                    utxos.push({txId: body.data.txs[i].txid, vout: body.data.txs[i].output_no, address: body.data.address, script: litecore.Script.buildPublicKeyHashOut(body.data.address).toString(), value: body.data.txs[i].value * 100000000});
                }
                resolve(utxos);
            }
        });
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