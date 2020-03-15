const bch = require('bitcoincashjs');

// tmp file named pipe for input
var tmpfilein = process.argv[2];
var tmpfileout = process.argv[3];

var Json_data = "";
var fs = require('fs')
const fd = fs.openSync(tmpfilein, 'r')
const stream = fs.createReadStream(null, {fd})
// Get data from named pipe
stream.on('data', (d) => {

    if (d.toString().trim() === '[stdin end]') {
        return process.nextTick(() => {
            console.log(process.argv.slice(2))
        })
    }
    process.argv.push(d.toString());
    var string_pipe = d.toString();
    string_pipe = string_pipe.slice(0, -1); // Slice last character (null terminated char)

    // Convert data to JSON
    Json_data = JSON.parse(string_pipe);
    // console.log(Json_data);

    var inputs = []; // INPUTS
    var outputs = []; // OUTPUTS

    // Separate inputs from outputs
    for( var i=0; i<Json_data.length; i++ ){
        // Inputs
        if (i != Json_data.length-1) {
            inputs.push(Json_data[i]);
            
        }
        // Outputs (always last element)
        else {
            outputs.push(Json_data[i]);
        }
    }

    // Sign transaction
    module.exports.sign_transaction(inputs, outputs, outputs[0].fee);
    
});

module.exports.sign_transaction = async function(inputs, outputs, fee){    
    var utxos = [];
    for(const input of inputs){
        utxos.push({
            "txId" : input.txId,
            //index de donde se encuentra el address
            "outputIndex" : input.vout,
            "address" : input.address,
            "script" : bch.Script.buildPublicKeyHashOut(input.address).toString(),
            //amount has to be in bch
            "satoshis" : input.value 
        });
    }
    var tx = new bch.Transaction();
    tx.from(utxos);
    tx.fee(fee);
    tx.to(outputs[0].address_to, outputs[0].value_send);
    tx.change(outputs[0].address_change);
    for(const utxo of utxos){
        await getPrivateKey(utxo.address, 2).then(function(rows){
            tx.sign(new bch.PrivateKey(rows[0].private_key));
        });
    }

	var txsigned_hex = tx.toString();
    // console.log(txsigned_hex);

	// Add null terminated character
	txsigned_hex += "\x00";

    // Write to named pipe
    const wstream = fs.createWriteStream(tmpfileout)
    wstream.write(txsigned_hex);

}

function getPrivateKey(address, coin_id){
    return new Promise(function(resolve, reject){
        db.all("SELECT private_key FROM addresses WHERE address = ? AND coin_id = ?;", [address, coin_id], (error, rows)=>{
            if(error)
                reject(error);
            else 
                resolve(rows);
        })
    });
}

// Open DB
const sqlite3 = require('sqlite3').verbose();
let db = new sqlite3.Database('../cypher.db', (err) => {
    if (err) {
        console.error(err.message);
    }
    //console.log('Connected to the cypher database.');
});
