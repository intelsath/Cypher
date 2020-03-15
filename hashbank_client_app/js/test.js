const RippleAPI = require('ripple-lib').RippleAPI;
var sqlite3 = require('sqlite3').verbose();
const db = new sqlite3.Database('./cypher.db');

const api = new RippleAPI({server: 'wss://s.altnet.rippletest.net:51233'});
api.connect().then( () => {
    db.each("SELECT * FROM addresses WHERE coin_id = 5", [], (err, rows) => {
        //console.log(rows.address);
        api.getServerInfo().then( async server => {
            var ledgerRange = server.completeLedgers.split("-");
            return await api.getTransactions(rows.address, {minLedgerVersion: parseInt(ledgerRange[0]), maxLedgerVersion: parseInt(ledgerRange[1])}).catch(err => {console.error(err)});
        }).then( async transactions => {
            for (const transaction of transactions) {
                await api.getTransaction(transaction.id).then( result => {
                    validateTransaction(result, rows.address, 5);
                }).catch(error => {
                    console.error(error);
                });
            }
        }).then( () => api.disconnect() ).catch(error => {
            console.error(error)
        });
    })
});

function validateTransaction(transaction, address, coin_id){
    db.all("SELECT * FROM transactions WHERE transaction_hash = ?", [transaction.id], (err, rows) => {
        if(err){
            throw err;
        }
        if(rows.length == 0){
            validateAddress(address, coin_id).then( belongs => {
                if(belongs){
                    console.log(transaction);
                }
            });
        }
    });
}

async function validateAddress(address, coin_id){
    return new Promise( (resolve, reject) => {
        db.all("SELECT * FROM addresses WHERE address = ? and coin_id = ?", [address, coin_id], (err, rows) => {
            if(err){
                throw err;
            }
            (rows.length != 0) ? resolve(true) : resolve(false); 
        });
    });
}