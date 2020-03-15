const RippleAPI = require('ripple-lib').RippleAPI;

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

    var all_balances = []; // BALANCES
    var fee;

    // Separate BALANCES from FEE
    for( var i=0; i<Json_data.length; i++ ){
        // Inputs
        if (i != Json_data.length-1) {
            all_balances.push(Json_data[i]);
            
        }
        // Outputs (always last element)
        else {
            fee = Json_data[i].fee;
        }
    }

    // Sign transaction
    module.exports.sign_transaction(all_balances, fee);
  
})

module.exports.sign_transaction = async function(balances, fee){
    var api = new RippleAPI();
    var payment, instructions, tx_hashes = [];
    for(var balance of balances){
        payment = {
            "source": {
                "address": balance.address,
                "maxAmount": {
                    "value": (balance.amount_sending + (fee / balances.length)).toString(),
                    "currency": "XRP"
                }
            },
            "destination": {
                "address": balance.address_to,
                "amount": {
                    "value": (balance.amount_sending - (fee / balances.length)).toString(),
                    "currency": "XRP"
                }
            }
        };
        instructions = {
                "fee": (fee / balances.length).toString(),
                // sequence of all transactions starting in 1
                "sequence": balance.sequence,
                "maxLedgerVersion": null
        };
        await getPrivateKey(balance.address, 5).then(async function(rows){
            await api.preparePayment(balance.address, payment, instructions).then(prepared =>{
                //  signed transaction
                var signedPreparedTransaction = api.sign(prepared.txJSON, rows[0].private_key);
                tx_hashes.push(signedPreparedTransaction.signedTransaction);
            }).catch(err => console.error(err));
        });
    }

    var signatures;

    signatures = JSON.stringify(tx_hashes);

    // Add null terminated character
    signatures += "\x00";

    // Write to named pipe
    const wstream = fs.createWriteStream(tmpfileout)
    wstream.write(signatures);
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
