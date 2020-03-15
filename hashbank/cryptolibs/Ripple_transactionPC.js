const RippleAPI = require('ripple-lib').RippleAPI;

var address_to = process.argv[2];
var amount_sending = parseFloat(process.argv[3]); // Expect arg in xrp
var fee = parseFloat(process.argv[4]); // Expect arg in xrp
var api;

// Tmp file to store output
var tmpfile = process.argv[5];

module.exports.prepare_transaction = async function(){
    amount_sending += fee;
    const wallet_id = "1";
    var addresses = [];
    await getAddresses(wallet_id, 5).then(function(rows){
        addresses = rows;
    });
    const all_balances = [];
    api = new RippleAPI({server: 'wss://s.altnet.rippletest.net:51233'});
    await api.connect().then( async ()=>{
        for(const address of addresses){
            if(amount_sending==0){
                break;
            }else{
                await getBalance(address.address).then((response)=>{
                    if(response.value != 0){
                        if(response.value < amount_sending){
                            response.amount_sending = response.value;
                            amount_sending -= response.value;
                        }else{
                            response.amount_sending = amount_sending;
                            amount_sending -= amount_sending;
                        }
                        all_balances.push(response);
                    }
                });
            }
        }
    }).then(() => {
        api.disconnect();
    }).catch((error)=>{
        console.error(error);
    });
    
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

function getBalance(address){
    return new Promise(async function(resolve, reject){ 
        api.getBalances(address).then((balances)=>{
            if(balances[0].value >= 20){
                balances[0].value -= 20;
            }
            api.getAccountInfo(address).then((account)=>{
                balances[0].address = address;
                balances[0].sequence = account.sequence;
                balances[0].address_to = address_to;
                resolve(balances[0]);   
            }).catch((error)=>{
                console.error(error);
            });
        }).catch((error)=>{
            reject(error);
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
