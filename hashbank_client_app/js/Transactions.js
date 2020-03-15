const https = require('https');
const http = require('http');
const RippleAPI = require('ripple-lib').RippleAPI;
const api = new RippleAPI({server: 'wss://s.altnet.rippletest.net:51233'});

class Transactions{

    constructor(){
    }

    async main(){
      console.log("entrando a transacciones");
        return new Promise( (resolve, reject) => {
          verifyTxState();
          var temp;
          var db_transactions_array = [];
          db.all("SELECT a.address, b.coin_prefix, b.coin_id, a.balance FROM Addresses a INNER JOIN coin b ON a.coin_id = b.coin_id ORDER BY b.coin_id asc", [],async (err, db_addresses) => {
            if(err) throw err;
            db.all("SELECT DISTINCT transaction_hash FROM transactions;", [],async (err, db_transactions) => {
                if(err) throw err;
                db_transactions_array = [];
                if (db_transactions.length != 0) {
                  for (let i = 0; i < db_transactions.length; i++) {
                    db_transactions_array.push(db_transactions[i]);
                  }
                }
                for(let i = 0; i < db_addresses.length; i++){
                  await getTransactions(db_addresses[i]).then( async transactions => {
                    if(transactions){
                      temp = [];
                      temp.push.apply(temp, transactions);
                      for (let j = 0; j < temp.length; j++) {
                        if(db_addresses[i].coin_id == 4 || db_addresses[i].coin_id == 5){
                          if(db_transactions_array.indexOf(temp[j]) == -1){
                            //console.log(temp[j].transaction_hash +" NO SE HA VERIFICADO");
                            if(db_addresses[i].coin_id == 5){
                              await validateInputs(temp[j].transaction_hash, {address: temp[j].address_from, amount: temp[j].input_amount}, db_addresses[i].coin_id).then(async validated_address =>{
                                if(validated_address){
                                  await registerTransaction({transaction_hash: validated_address[0].input_hash, address: validated_address[0].address, amount: validated_address[0].amount, time: temp[j].time, status_id: 2, type_id: 2}, db_addresses[i].coin_id, j);
                                }
                              }).catch(err => console.error(JSON.stringify(err)));
                              await validateOutputs(temp[j].transaction_hash, {address: temp[j].address_to, amount: temp[j].output_amount}, db_addresses[i].coin_id).then(async validated_address => {
                                if(validated_address){
                                  await registerTransaction({transaction_hash: validated_address[0].input_hash, address: validated_address[0].address, amount: validated_address[0].amount, time: temp[j].time, status_id: 2, type_id: 1}, db_addresses[i].coin_id, j);
                                }
                              });
                            }else{
                              if(temp[j].confirmations > 6){
                                //INPUTS ETHEREUM COMPLETED
                                await validateInputs(temp[j].transaction_hash, {address: temp[j].address_from, amount: temp[j].value / 1000000000000000000}, db_addresses[i].coin_id).then(async validated_address => {
                                  if(validated_address){
                                    await registerTransaction({transaction_hash: validated_address[0].input_hash, address: validated_address[0].address, amount: validated_address[0].amount, time: temp[j].time, status_id: 2, type_id: 2}, db_addresses[i].coin_id, j);
                                  }
                                });
                                //OUTPUTS ETHEREUM COMPLETED
                                await validateOutputs(temp[j].transaction_hash, {address: temp[j].address_to, amount: temp[j].value / 1000000000000000000}, db_addresses[i].coin_id).then(async validated_address => {
                                  if(validated_address){
                                    await registerTransaction({transaction_hash: validated_address[0].input_hash, address: validated_address[0].address, amount: validated_address[0].amount, time: temp[j].time, status_id: 2, type_id: 1}, db_addresses[i].coin_id, j);
                                  }
                                });
                              }else{
                                //INPUTS ETHEREUM PENDING
                                await validateInputs(temp[j].transaction_hash, {address: temp[j].address_from, amount: temp[j].value / 1000000000000000000}, db_addresses[i].coin_id).then(async validated_address => {
                                  if(validated_address){
                                    await registerTransaction({transaction_hash: validated_address[0].input_hash, address: validated_address[0].address, amount: validated_address[0].amount, time: temp[j].time, status_id: 1, type_id: 2}, db_addresses[i].coin_id, j);
                                  }
                                });
                                //OUTPUTS ETHEREUM PENDING
                                await validateOutputs(temp[j].transaction_hash, {address: temp[j].address_to, amount: temp[j].value / 1000000000000000000}, db_addresses[i].coin_id).then(async validated_address => {
                                  if(validated_address){
                                    await registerTransaction({transaction_hash: validated_address[0].input_hash, address: validated_address[0].address, amount: validated_address[0].amount, time: temp[j].time, status_id: 1, type_id: 1}, db_addresses[i].coin_id, j);
                                  }
                                });
                              }
                            }
                          }
                        }else{
                          //console.log("RESULT: "+ db_transactions_array.indexOf("5cdbcb04a3e8ce8e31489159cd54603b2327fc8983a1ecbb6bcee1045735a979"));
                          if(db_transactions_array.indexOf(temp[j]) == -1){
                            //console.log(temp[j] +" NO SE HA VERIFICADO")
                            await getTransactionInfo(temp[j], db_addresses[i].address, db_addresses[i].coin_id)
                              .then(async transaction_result => {
                                if(transaction_result[0]){
                                  if(transaction_result[0].confirmations > 6){
                                    //INPUTS BTC, BCH, LTC COMPLETED
                                    await validateInputs(transaction_result[0].transaction_hash, transaction_result[0].inputs, db_addresses[i].coin_id)
                                      .then ( async validated_address => {
                                        if (validated_address) {
                                          await registerTransaction({transaction_hash: validated_address[0].input_hash, address: validated_address[0].address, amount: validated_address[0].amount, time: transaction_result[0].time, status_id: 2, type_id: 2}, db_addresses[i].coin_id, j);
                                        }
                                      });
                                    //OUTPUTS BTC, BCH, LTC COMPLETED  
                                    await validateOutputs(transaction_result[0].transaction_hash, transaction_result[0].outputs, db_addresses[i].coin_id)
                                      .then ( async validated_address => {
                                        if (validated_address) {
                                          await registerTransaction({transaction_hash: validated_address[0].input_hash, address: validated_address[0].address, amount: validated_address[0].amount, time: transaction_result[0].time, status_id: 2, type_id: 1}, db_addresses[i].coin_id, j);
                                        }
                                      });
                                  }else{
                                    //INPUTS BTC, BCH, LTC PENDING
                                    await validateInputs(transaction_result[0].transaction_hash, transaction_result[0].inputs, db_addresses[i].coin_id)
                                      .then( async validated_address => {
                                        if(validated_address){
                                          await registerTransaction({transaction_hash: validated_address[0].input_hash, address: validated_address[0].address, amount: validated_address[0].amount, time: transaction_result[0].time, status_id: 1, type_id: 2}, db_addresses[i].coin_id, j);
                                        }
                                    });
                                    //OUTPUTS BTC, BCH, LTC PENDING
                                    await validateOutputs(transaction_result[0].transaction_hash, transaction_result[0].outputs, db_addresses[i].coin_id)
                                      .then( async validated_address => {
                                        if(validated_address){
                                          await registerTransaction({transaction_hash: validated_address[0].input_hash, address: validated_address[0].address, amount: validated_address[0].amount, time: transaction_result[0].time, status_id: 1, type_id: 1}, db_addresses[i].coin_id, j);
                                        }
                                    });
                                  }
                                }
                              });
                          }
                        }
                      }
                    }
                  });
                }
                resolve(true);
            })
          });
        });
    }
    
    disconnectRippleServer(){
      api.disconnect().then( () => {
        console.log("Ripple server disconnected");
      });
    }
}


async function getTransactions(db_address){
    var result = [];
    var rippleArray = [];
    return new Promise( (resolve, reject) => {
      switch(db_address.coin_id){
        case 1:
          console.log("BITCOIN ADDRESS: " + db_address.address);
          https.get('https://testnet.blockexplorer.com/api/addr/'+ db_address.address, (response) => {
              let data = '';
              response.on('data', (chunk) => {
                  data += chunk;
              });
              response.on('end', () => {
                try{
                  resolve(JSON.parse(data).transactions);
                }catch(error){
                  resolve(false);
                  console.error(error);
                }
              });
          });
          break;
        case 2:
          console.log("BITCOIN CASH ADDRESS: " + db_address.address);
          https.get('https://test-bch-insight.bitpay.com/api/addr/'+ db_address.address, (response) => {
              let data = '';
              response.on('data', (chunk) => {
                  data += chunk;
              });
              response.on('end', () => {
                try{
                  resolve(JSON.parse(data).transactions);
                }catch(error){
                  resolve(false);
                  console.error(error);
                }
              });
          });
          break;
        case 3: 
          console.log("LITECOIN ADDRESS: " + db_address.address);
          https.get('https://chain.so/api/v2/address/LTCTEST/'+ db_address.address, (response) => {
              let data = '';
              response.on('data', (chunk) => {
                  data += chunk;
              });
              response.on('end', () => {
                try{
                  for (let i = 0; i < JSON.parse(data).data.txs.length; i++) {
                      result.push(JSON.parse(data).data.txs[i].txid);
                  }
                  resolve(result);
                }catch(error){
                  resolve(false);
                  console.error(error);
                }
              });
          });
          break;
        case 4:
          console.log("ETHEREUM ADDRESS: " + db_address.address);
          http.get('http://api-ropsten.etherscan.io/api?module=account&action=txlist&address='+ db_address.address+ "&startblock=0&endblock=99999999&sort=asc&apikey=YourApiKeyToken", (response) => {
              let data = '';
              response.on('data', (chunk) => {
                  data += chunk;
              });
              response.on('end', () => {
                try{
                  for (let i = 0; i < JSON.parse(data).result.length; i++) {
                      result.push({transaction_hash: JSON.parse(data).result[i].hash, address_from: JSON.parse(data).result[i].from, address_to: JSON.parse(data).result[i].to, confirmations: JSON.parse(data).result[i].confirmations, value: JSON.parse(data).result[i].value, time: JSON.parse(data).result[i].timeStamp});
                  }
                  resolve(result);
                }catch(error){
                  resolve(false);
                  console.error(error);
                }
              });
          });
          break;
        case 5:
          console.log("RIPPLE ADDRESS: " + db_address.address);
          //const api = new RippleAPI({server: 'wss://s.altnet.rippletest.net:51233'});
          api.connect().then( () => {
              api.getServerInfo().then( server => {
              var ledgerRange = server.completeLedgers.split("-");
              return api.getTransactions(db_address.address, {minLedgerVersion: parseInt(ledgerRange[0]), maxLedgerVersion: parseInt(ledgerRange[1])}).catch(err => {console.error(err)});
              }).then( async transactions => {
              result.push.apply(result, transactions);
              rippleArray = [];
              for (let i = 0; i < result.length; i++) {
                  rippleArray.push({transaction_hash: result[i].id, address_from: result[i].specification.source.address, address_to: result[i].specification.destination.address, input_amount: result[i].specification.source.maxAmount.value, output_amount: result[i].specification.destination.amount.value, time: result[i].outcome.timestamp});
              }
              resolve(rippleArray);
              }).catch(error => {
              console.error(error)
              });
          });
          break;    
      }
    });
}

async function getTransactionInfo(transaction_hash, address, coin_id){
    var result = [];
    return new Promise( (resolve, reject) => {
      switch(coin_id){
        case 1:
          https.get("https://testnet.blockexplorer.com/api/tx/" + transaction_hash, (response) => {
              let data = '';

              response.on('data', (chunk) => {
                  data += chunk;
              });

              response.on('end', () => {
                try{
                  result.push({
                    transaction_hash: JSON.parse(data).txid,
                    inputs: JSON.parse(data).vin,
                    outputs: JSON.parse(data).vout,
                    confirmations: JSON.parse(data).confirmations,
                    time: JSON.parse(data).time      
                  });
                  resolve(result);
                }catch(error){
                  resolve(false);
                  console.error(error.message)
                }  
              });
          });
          break;
        case 2:
          //console.log(transaction_hash);
          https.get("https://test-bch-insight.bitpay.com/api/tx/" + transaction_hash, (response) => {
            let data = '';

            response.on('data', (chunk) => {
                data += chunk;
            });

            response.on('end', () => {
              try{
                result.push({
                    transaction_hash: JSON.parse(data).txid,
                    inputs: JSON.parse(data).vin,
                    outputs: JSON.parse(data).vout,
                    confirmations: JSON.parse(data).confirmations,
                    time: JSON.parse(data).time      
                });
                resolve(result);
              }catch(error){
                resolve(false);
                console.error(error.message);
              }
            });
          }).on('error', (err) => {
            console.log(err);
          });
          break;  
        case 3:
        https.get("https://chain.so/api/v2/get_tx/LTCTEST/" + transaction_hash, (response) => {
          let data = '';
          response.on('data', (chunk) => {
            data += chunk;
          });
          
          response.on('end', () => {
            try{
              result.push({
                transaction_hash: JSON.parse(data).data.txid,
                inputs: JSON.parse(data).data.inputs,
                outputs: JSON.parse(data).data.outputs,
                confirmations: JSON.parse(data).data.confirmations,
                time: JSON.parse(data).data.time      
              });
              resolve(result);
            }catch(error){
              resolve(false);
              console.log(error);
            }
          });
        });
        break;  
      }
    });
}

async function validateInputs(transaction_hash, inputs, coin_id){
  var result = [];
  return new Promise( async (resolve, reject) => {
    switch(coin_id){
      case 1:
        for (let i = 0; i < inputs.length; i++) {
          await validateAddress(inputs[i].addr, coin_id).then( belongs => {
            if (belongs) { 
              result.push({address: inputs[i].addr, amount: inputs[i].value, input_hash: inputs[i].txid}); 
              resolve(result) 
            } else {
              resolve(false);
            } 
          }).catch(err => console.error(err));
        }
        break;
      case 2: 
        for (let i = 0; i < inputs.length; i++) {
          await validateAddress(inputs[i].addr, coin_id).then( belongs => {
            if (belongs) { 
              result.push({address: inputs[i].addr, amount: inputs[i].value, input_hash: inputs[i].txid}) 
              resolve(result) 
            } else {
              resolve(false);
            }
          }).catch(err => console.error(err));
        }
        break;
      case 3:
        for (let i = 0; i < inputs.length; i++) {
          await validateAddress(inputs[i].address, coin_id).then( belongs => {
            if (belongs) { 
              result.push({address: inputs[i].address, amount: inputs[i].value, input_hash: inputs[i].from_output.txid}) 
              resolve(result) 
            } else {
              resolve(false);
            }
          }).catch(err => console.error(err));
        }
        break;
        case 4:
          await validateAddress(inputs.address, coin_id).then( belongs => {
            if (belongs) { 
              result.push({address: inputs.address, amount: inputs.amount, input_hash: transaction_hash}) 
              resolve(result) 
            } else {
              resolve(false);
            }
          }).catch(err => console.error(err));
          break;
        case 5:
          await validateAddress(inputs.address, coin_id).then( belongs => {
            if (belongs) { 
              result.push({address: inputs.address, amount: inputs.amount, input_hash: transaction_hash}) 
              resolve(result) 
            } else {
              resolve(false);
            }
          }).catch(err => console.error(JSON.stringify(err)));
          break;    
    }
  })
}

async function validateOutputs(transaction_hash, outputs, coin_id){
  var result = [];
  return new Promise( async (resolve, reject) => {
    switch(coin_id){
      case 1:
        for (let i = 0; i < outputs.length; i++) {
          if(outputs[i].scriptPubKey.addresses){
            await validateAddress(outputs[i].scriptPubKey.addresses[0], coin_id).then( belongs => {
              if (belongs) { 
                result.push({address: outputs[i].scriptPubKey.addresses[0], amount: outputs[i].value, input_hash: transaction_hash}); 
                resolve(result) 
              } else {
                resolve(false);
              } 
            }).catch(err => console.error(err));
          }
        }
        break;
      case 2: 
        for (let i = 0; i < outputs.length; i++) {
          await validateAddress(outputs[i].scriptPubKey.addresses[0], coin_id).then( belongs => {
            if (belongs) { 
              result.push({address: outputs[i].scriptPubKey.addresses[0], amount: outputs[i].value, input_hash: transaction_hash}) 
              resolve(result) 
            } else {
              resolve(false);
            }
          }).catch(err => console.error(err));
        }
        break;
      case 3:
        for (let i = 0; i < outputs.length; i++) {
          await validateAddress(outputs[i].address, coin_id).then( belongs => {
            if (belongs) { 
              result.push({address: outputs[i].address, amount: outputs[i].value, input_hash: transaction_hash}) 
              resolve(result); 
            } else {
              resolve(false);
            }
          }).catch(err => console.error(err));
        }
        break;
        case 4:
          await validateAddress(outputs.address, coin_id).then( belongs => {
            if (belongs) { 
              result.push({address: outputs.address, amount: outputs.amount, input_hash: transaction_hash}); 
              resolve(result); 
            } else {
              resolve(false);
            }
          }).catch(err => console.error(err));
          break;
        case 5:
          await validateAddress(outputs.address, coin_id).then( belongs => {
            if (belongs) { 
              result.push({address: outputs.address, amount: outputs.amount, input_hash: transaction_hash}) 
              resolve(result) 
            } else {
              resolve(false);
            }
          }).catch(err => console.error(JSON.stringify(err)));
          break;    
    }
  })
}

async function validateAddress(address, coin_id){
  return new Promise( (resolve, reject) => {
    db.all("SELECT * FROM addresses WHERE address = ? and coin_id = ?;", [address, coin_id], (err, rows) => {
      if(err){
        throw err;
      }
      (rows.length != 0) ? resolve(true) : resolve(false);
    });
  });
}

async function registerTransaction(transaction, coin_id, index){
  if(coin_id == 5){
    const date = new Date(transaction.time);
    var month = "0"+date.getMonth(), day = "0"+date.getDate(), hour = "0"+date.getHours(), minutes = "0"+date.getMinutes(), seconds = "0"+date.getSeconds(), fullday;
    fullday = date.getFullYear() + "-" + month.substr(-2) + "-" + day.substr(-2) + " " + hour.substr(-2) + ":" + minutes.substr(-2) + ":" + seconds.substr(-2);
  }else{
    const date = new Date(transaction.time * 1000);
    var month = "0"+date.getMonth(), day = "0"+date.getDate(), hour = "0"+date.getHours(), minutes = "0"+date.getMinutes(), seconds = "0"+date.getSeconds(), fullday;
    fullday = date.getFullYear() + "-" + month.substr(-2) + "-" + day.substr(-2) + " " + hour.substr(-2) + ":" + minutes.substr(-2) + ":" + seconds.substr(-2);
  }
  
  await this.validateTransaction(transaction.transaction_hash, transaction.address, transaction.type_id).then( async exist => {
    if(!exist){
      db.run("INSERT INTO transactions(transaction_hash, address, amount, transaction_date, coin_id, status_id, type_id, wallet_id) VALUES(?, ?, ?, ?, ?, ?, ?, (SELECT wallet_id FROM addresses WHERE address = ? and coin_id = ?))", [transaction.transaction_hash, transaction.address, transaction.amount, fullday, coin_id, transaction.status_id, transaction.type_id, transaction.address, coin_id], (err) => {
        if(err){
          throw err
        }
        console.log("TRANSACTION: "+ transaction.transaction_hash +" AND ADDRESS: "+ transaction.address + " REGISTERED!");
      });
    }
  });
}

async function validateTransaction(transaction_hash, address, type_id){
  return new Promise( (resolve, reject) => {
    db.all("SELECT * FROM transactions WHERE transaction_hash = ? and address = ? and type_id = ?;", [transaction_hash, address, type_id], (err, rows) => {
      if(err){
        throw err;
      }
      (rows.length != 0) ? resolve(true) : resolve(false)
    });
  });
}

  /************************************************************* 
  *                        Verify status                       *
  **************************************************************/
  
async function verifyTxState(){
  db.all("SELECT transaction_hash, coin_id, address FROM transactions WHERE status_id = 1;", [], async (err, rows) => {
    if (err) {
      throw err;
    }
    for (let i = 0; i < rows.length; i++) {
      await getTransactionInfo(rows[i].transaction_hash, rows[i].address, rows[i].coin_id).then( transaction_info => {
        if(transaction_info[0].confirmations > 6){
          this.database.updatePendingtx(rows[i].transaction_hash, rows[i].address).then( () => {
            console.log("TRANSACCION :"+ rows[i].transaction_hash +" ACTUALIZADA");
          });
        }
      });
    }
  });
}

module.exports = Transactions;