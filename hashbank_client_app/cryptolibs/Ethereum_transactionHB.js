const ethUtil = require('ethereumjs-util');
const EthereumTx = require('ethereumjs-tx');

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

module.exports.sign_transaction = async function(all_balances, fee){
    var tx_hashes = [];
    for(const balance of all_balances){
        var balance_tmp = balance;
        await getPrivateKey(balance_tmp.address, 4).then(function(rows){
            var rawTx = {
                //The total cost of a transaction (the "TX fee") is the Gas Limit * Gas Price.
                //*21000 is the gas limit for standard transactions
                nonce: 3,
                gasLimit: 21000,
                
                to: balance_tmp.address_to,
                //transfer 1 wei
                // 1 ETH = 1000000000000000000 wei
                value: balance_tmp.amount_sending - (fee / all_balances.length),
            };
            var tx = new EthereumTx(rawTx);
            var privBuffer = ethUtil.toBuffer(rows[0].private_key);
            tx.sign(privBuffer);
            tx_hashes.push(tx.serialize().toString('hex'));
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
  console.log('Connected to the cypher database.');
});