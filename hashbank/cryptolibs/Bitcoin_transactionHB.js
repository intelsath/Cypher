const bitcoin = require('bitcoinjs-lib');

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
    module.exports.sign_transaction(inputs, outputs);
  
})


module.exports.sign_transaction = async function(inputs, outputs){
 
    var txb = new bitcoin.TransactionBuilder(bitcoin.networks.testnet);
    var count = 0;

	for(var i=0; i<inputs.length; i++){
		txb.addInput(inputs[i].txId, inputs[i].vout);
	}
	// Output
	txb.addOutput(outputs[0].address_to, outputs[0].value_send);
	txb.addOutput(outputs[0].address_change, outputs[0].value_change);

    for(const input of inputs){
           await getPrivateKey(input.address).then(function(rows){

            txb.sign(count, bitcoin.ECPair.fromWIF(rows[0].private_key, bitcoin.networks.testnet));
            count++;
        });
    }

	var txsigned_hex = (txb.build().toHex()).toString();
    // console.log(txsigned_hex);

	// Add null terminated character
	txsigned_hex += "\x00";

    // Write to named pipe
    const wstream = fs.createWriteStream(tmpfileout)
    wstream.write(txsigned_hex);
}

function getPrivateKey(address){
    return new Promise(function(resolve, reject){
        db.all("SELECT private_key FROM addresses WHERE address = ? AND coin_id = 1;", [address], (error, rows)=>{
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
  console.log('Connected to the cypher database.');
});
