const request = require('request');

const apiToken = "JZDEW855XEZFDZ3H3QWXXDG8F9A3YGTVMT";
const apiUrl = "https://api-ropsten.etherscan.io/api?module=account&action=balance&address=";

var address_to = process.argv[2];
var amount_sending = parseFloat(process.argv[3]) * 1000000000000000000; // Expect arg in ether, convert it to wei
var fee = parseFloat(process.argv[4])* 1000000000000000000;             // Expect arg in ether, convert it to wei

// Tmp file to store output
var tmpfile = process.argv[5];

// faucet: https://faucet.ropsten.be

module.exports.prepare_transaction = async function(){
    const wallet_id = "1";
    var addresses = [];
    await getAddresses(wallet_id, 4).then(function(rows){
        addresses = rows;
    });
    const all_balances = [];
    for(const address of addresses){
        if(amount_sending==0){
            break;
        }else{
            await getBalance(address).then((response)=>{
                if(response.amount > 0){
                    if(response.amount < amount_sending){
                        response.amount_sending = response.amount;
                        amount_sending -= response.amount;
                    }else{
                        response.amount_sending = amount_sending;
                        amount_sending -= amount_sending;
                    }
                    all_balances.push(response);
                }
            });
        }
    }
    all_balances.push({fee: fee});
    
    // Write to file
    var fs = require('fs');
    fs.writeFile(tmpfile, JSON.stringify(all_balances), function(err) {
        if(err) {
            return console.log(err);
        }

        //console.log("The file was saved!");
    });
}

async function getBalance(address){
    return new Promise( function(resolve, reject){ 
        request.get(apiUrl + address.address + '&tag=latest&apikey=' + apiToken, { json: true }, (err, req, body) => {
            if(err){
                reject(err);
            }else{
                /*var Web3 = require('web3');
                // create an instance of web3 using the HTTP provider.
                // NOTE in mist web3 is already available, so check first if it's available before instantiating
                var web3 = new Web3(new Web3.providers.HttpProvider("http://localhost:8545"));
                //var number = web3.eth.getTransactionCount(address_to);
                console.log("entra");        
                await web3.eth.getTransactionCount(address_to).then(result=>{
                    console.log(result);
                });*/
                resolve({amount: body.result - 21000, address: address.address, address_to: address_to, nonce: 5});
            }
        });
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
  //console.log('Connected to the cypher database.');
});

module.exports.prepare_transaction();