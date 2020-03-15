function copyClipboard(currency){
    document.getElementById("receive-address-input-" + currency).select();
    document.execCommand("Copy");
}

var coin_array=[];
var coins;

function sortAmount(){

}

document.getElementById("transactions-side_nav").addEventListener("click", function(){
    document.getElementById("advanced-filter-modal-year-input").value = "";
    document.getElementById("advanced-filter-modal-month-input").value = "";
    document.getElementById("advanced-filter-modal-day-input").value = "";
    document.getElementById("advanced-filter-modal-coinType-select").value = "none";
});

function loadCoins(){
    var db = new sqlite3.Database('./cypher.db');
    var sql = "select * from coin;"
    db.all(sql, [], ( err, rows ) => {
        coins = rows;
        console.log(coins);
        for(let i = 0; i < coins.length; i++){
            coin_array.push(coins[i].coin_name);
            $('select[name="cmbCoins"]').append('<option value="'+coins[i].coin_id+'">'+coins[i].coin_name+'</option>')
        }
    });
}

function changeWallet(){
    $("#pages .page:not('.hide')").stop().fadeOut('fast', function(){
        $(this).addClass('hide');
        $('#change-wallet').fadeIn('slow').removeClass('hide');
    });
    document.getElementById("side-nav").style.display = "none";
    document.getElementById("change-wallet").classList.remove("hide");
}

function verifyBalance(option){
    loadBalanceAddress().then( change => {
        loadBalance().then( total => {
            if(change){
                loadChart(total);
            }
        });
        /*setTimeout(() => {
            verifyBalance();
        }, 60000);*/
    });
}

function loadPortfolioBalance(wallet_id){
    selected_wallet = wallet_id;
    db.all("SELECT SUM(balance) as balance FROM addresses WHERE wallet_id = ? GROUP BY coin_id", [wallet_id], (err, rows) => {
        var total = 0;
        if(err){
            throw err;
        }
        document.getElementById('change-wallet_menu').classList.remove('active');
        document.getElementById('portfolio-side_nav').classList.add('active');
        $("#pages .page:not('.hide')").stop().fadeOut('fast', function(){
            $(this).addClass('hide');
            $('#portfolio').fadeIn('slow').removeClass('hide');
        });
        document.getElementById("side-nav").style.display = "";
        for (let i = 0; i < coins.length; i++) {
            if(rows[i]){
                total_array[i].coin_price = coins[i].coin_price;
                total_array[i].total = rows[i].balance;
                total += total_array[i].total * coins[i].coin_price;
            }else{
                total_array[i].coin_price = coins[i].coin_price;
                total_array[i].total = 0;
                total += total_array[i].total * coins[i].coin_price;
            }
        }
        loadChart(total);
        verifyBalance();
    });
}

function updateAddressBalance(balance, address, coin_id){
    db.run("UPDATE addresses SET balance = ? WHERE address = ? AND coin_id = ?", [balance, address, coin_id], (err, rows) => {
        if(err){
            console.error(err);
        }
    });
}

async function loadBalanceAddress(){
 //   const transaction = new Transactions();
    return new Promise( (resolve, reject) => {
        db.all("SELECT address, coin_id, balance FROM addresses;", [], async (err, rows) => {
            if(err){
                throw err;
            }
            var flag = false;
            var addresses = rows;
            for (const address of addresses) {
                switch(address.coin_id){
                    case 1:
                        var ajax = await $.ajax({
                            url: 'https://testnet.blockexplorer.com/api/addr/' + address.address,
                            dataType: 'json',
                            success: result => {
                                if(address.balance != result.balance){
                                    console.log("UPDATING! BITCOIN");
                                    updateAddressBalance(result.balance, address.address, address.coin_id);
                                    flag = true;
                                }
                                //transaction.verifyTransactions(result.transactions, address.address, address.coin_id);
                            }
                        });
                        break;
                    case 2:
                        var ajax = await $.ajax({
                            url: 'https://test-bch-insight.bitpay.com/api/addr/' + address.address,
                            dataType: 'json',
                            success: result => {
                                if(address.balance != result.balance){
                                    console.log("UPDATING! BITCOIN CASH");
                                    updateAddressBalance(result.balance, address.address, address.coin_id);
                                    flag = true;
                                }
                                //transaction.verifyTransactions(result.transactions, address.address, address.coin_id);
                            }
                        });
                        break;
                    case 3:
                        var ajax = await $.ajax({
                            url: 'https://chain.so/api/v2/address/LTCTEST/' + address.address,
                            dataType: 'json',
                            success: result => {
                                var hashes = [];
                                for (let i = 0; i < result.data.txs.length; i++) {
                                    hashes.push(result.data.txs[i].txid);
                                }
                                if(address.balance != result.data.balance){
                                    console.log("UPDATING! LITECOIN");
                                    updateAddressBalance(result.data.balance, address.address, address.coin_id);
                                    flag = true;
                                }
                                //transaction.verifyTransactions(hashes, address.address, address.coin_id);
                            }
                        });
                        break;
                    case 4:
                        var ajax = await $.ajax({
                            url: 'https://ropsten.etherscan.io/api?module=account&action=balance&address='+ address.address +'&tag=latest&apikey=YourApiKeyToken',
                            dataType: 'json',
                            success: result => {
                                if(address.balance != (result.result)/ 1000000000000000000){
                                    console.log("UPDATING! ETHEREUM");
                                    updateAddressBalance(parseFloat(result.result / 1000000000000000000), address.address, address.coin_id);
                                    flag = true;
                                }
                                //transaction.verifyTransactions([""], address.address, address.coin_id);
                            }
                        });
                        break; 
                    case 5:
                        var rippleApi = new RippleAPI({server: 'wss://s.altnet.rippletest.net:51233'});   
                        await rippleApi.connect().then( async ( ) => {
                            await rippleApi.getServerInfo().then( server_info =>{
                                rippleApi.getBalances(address.address, {ledgerVersion: server_info.validatedLedger.ledgerVersion}).then( result => {
                                    if(address.balance != result[0].value){
                                        console.log("UPDATING! RIPPLE");
                                        updateAddressBalance(result[0].value, address.address, address.coin_id);         
                                        flag = true;
                                    }
                                    //transaction.verifyTransactions([""], address.address, address.coin_id);
                                });
                            });
                        });
                        break;
                }
            }
            resolve(flag);
        });
    });
}

async function loadBalance(){
    let db = new sqlite3.Database('./cypher.db');
    var total = 0;
    return new Promise( (resolve, reject) => {
        db.all('select sum(balance) as balance, coin_id from addresses group by coin_id order by coin_id;', [], (err, rows) => {
            for (const balance of rows) {
                switch(balance.coin_id){
                    case 1:
                        total_array[0].coin_price = coins[0].coin_price;
                        total_array[0].total = balance.balance;
                        total += total_array[0].total * coins[0].coin_price;
                        break;
                    case 2:
                        total_array[1].coin_price = coins[1].coin_price;
                        total_array[1].total = balance.balance;
                        total += total_array[1].total * coins[1].coin_price;
                        break;
                    case 3:
                        total_array[2].coin_price = coins[2].coin_price;
                        total_array[2].total = balance.balance;
                        total += total_array[2].total * coins[2].coin_price;
                        break;
                    case 4:
                        total_array[3].coin_price = coins[3].coin_price;
                        total_array[3].total = balance.balance;
                        total += total_array[3].total * coins[3].coin_price;
                        break;
                    case 5:
                        total_array[4].coin_price = coins[4].coin_price;
                        total_array[4].total = balance.balance;
                        total += total_array[4].total * coins[4].coin_price;
                        break;            
                }
            }
            resolve(total);
        });
        db.close();
    });
}

function loadChart(total){
    const chart = new myChart();
    for (let i = 0; i < coins.length; i++) {
        if(total_array[i].total == 0){
            total_array[i].percentage = 0;   
            document.getElementById(coins[i].coin_name.toLowerCase().replace(" ", "_")+"-percent").innerHTML = total_array[i].percentage.toFixed(2)+"%"
            document.getElementById(coins[i].coin_name.toLowerCase().replace(" ", "_")+"-price").innerHTML = comma_splitter(Math.floor(total_array[i].total * 100000)/100000);
        }else{
            total_array[i].percentage = (((coins[i].coin_price * total_array[i].total) / total)*100);
            document.getElementById(coins[i].coin_name.toLowerCase().replace(" ", "_")+"-percent").innerHTML = total_array[i].percentage.toFixed(2)+"%"
            document.getElementById(coins[i].coin_name.toLowerCase().replace(" ", "_")+"-price").innerHTML = comma_splitter(Math.floor(total_array[i].total * 100000)/100000);
        }
    }

    document.getElementById("mycanvas").remove();
    var canvas = document.createElement('canvas');
    canvas.id = "mycanvas";
    canvas.style.width = "350px";
    canvas.style.height = "235px";
    document.getElementById("currencyChart").appendChild(canvas);
    chart.addElement("Bitcoin", "#f79920", "rgba(248,155,36,0.5)", total_array[0].percentage);
    chart.addElement("Bitcoin Cash", "#00c900", "rgba(0,201,0,0.5)", total_array[1].percentage);
    chart.addElement("Litecoin", "#8c8c8c", "rgba(140,140,140,0.4)", total_array[2].percentage);
    chart.addElement("Ethereum", "#313131", "rgba(49,49,49,0.5)", total_array[3].percentage);
    chart.addElement("Ripple", "#007ab2", "rgba(0,122,178,0.5)", total_array[4].percentage);
    chart.createTable();
}



function advancedFilters(table, option){

    advancedFiltersTutorial();
    var table_element, tr;
    table_element = document.getElementById(table);
    tr = table_element.getElementsByTagName("tr");

    for(i = 1; i < tr.length; i++){
        tr[i].classList.add("showing");
    }

    switch(option){
        case "transactions":
            document.querySelector('#advanced-filter-modal-coinType-select').setAttribute("onchange", "transactionFilter(1, 4, 'transaction-table', 'Transaction-pagination-list', 'transaction-div-table', 4, 'Transaction-pagination-lquo-button', 'Transaction-pagination-rquo-button', 'transaction-pagination-current-page-number', 'transaction-pagination-last-page-number', 'pagination-night-theme', 'pagination-day-theme', 'transaction-pagination-info')");
            document.querySelector('#advanced-filter-modal-year-input').setAttribute("onblur", "transactionFilter(1, 4, 'transaction-table', 'Transaction-pagination-list', 'transaction-div-table', 4, 'Transaction-pagination-lquo-button', 'Transaction-pagination-rquo-button', 'transaction-pagination-current-page-number', 'transaction-pagination-last-page-number', 'pagination-night-theme', 'pagination-day-theme', 'transaction-pagination-info')");
            document.querySelector('#advanced-filter-modal-month-input').setAttribute("onchange", "transactionFilter(1, 4, 'transaction-table', 'Transaction-pagination-list', 'transaction-div-table', 4, 'Transaction-pagination-lquo-button', 'Transaction-pagination-rquo-button', 'transaction-pagination-current-page-number', 'transaction-pagination-last-page-number', 'pagination-night-theme', 'pagination-day-theme', 'transaction-pagination-info')");
            document.querySelector('#advanced-filter-modal-day-input').setAttribute("onblur", "transactionFilter(1, 4, 'transaction-table', 'Transaction-pagination-list', 'transaction-div-table', 4, 'Transaction-pagination-lquo-button', 'Transaction-pagination-rquo-button', 'transaction-pagination-current-page-number', 'transaction-pagination-last-page-number', 'pagination-night-theme', 'pagination-day-theme', 'transaction-pagination-info')");
            document.querySelector('#advanced-filter-modal-clear-btn').setAttribute("onclick", "clearSearch('transaction-table', 'transactions')");
            break;
    }
}

function clearSearch(table, option){
    var table_element = document.getElementById(table);
    var tr = table_element.getElementsByTagName("tr");

    switch(option){
      case "transactions":
        loadTransactions();
        break;  
    }
    
    for(i = 1; i < tr.length; i++){
        tr[i].classList.add("showing");
    }
    
    $("#advanced-filter-modal-coinType-select option:first").attr('selected','selected');    
    document.getElementById("advanced-filter-modal-year-input").value = "";
    document.getElementById("advanced-filter-modal-month-input").value = "";
    document.getElementById("advanced-filter-modal-day-input").value = "";
}

function transactionFilter(coinCol, dateCol, table, pagination_list, table_div, rows, lquo, rquo, currentPage, lastPage, night, day, footer){
    var year = document.getElementById("advanced-filter-modal-year-input").value, month = document.getElementById("advanced-filter-modal-month-input").value, date = document.getElementById("advanced-filter-modal-day-input").value; 
    loadTransactions().then( transactions => {
        var filtered_txs = transactions.filter( (value, index, array) => {
            if(document.getElementById("advanced-filter-modal-coinType-select").selectedIndex != 0){
                if(date == ""){
                    return (value.coin_name === document.getElementById("advanced-filter-modal-coinType-select")[document.getElementById("advanced-filter-modal-coinType-select").selectedIndex].innerHTML && value.transaction_date.split(" ")[0].split("-")[1].indexOf(month.split("-")[1] || "" ) > -1 && value.transaction_date.split(" ")[0].split("-")[0].indexOf(year || "") > -1);
                }else{
                    return (value.coin_name === document.getElementById("advanced-filter-modal-coinType-select")[document.getElementById("advanced-filter-modal-coinType-select").selectedIndex].innerHTML && value.transaction_date.split(" ")[0].indexOf(date) > -1);
                }
            }else{
                if(date == ""){
                    return (value.transaction_date.split(" ")[0].split("-")[0].indexOf(year) > -1 && value.transaction_date.split(" ")[0].split("-")[1].indexOf(month.split("-")[1] || "") > -1);
                }else{
                    return (value.transaction_date.split(" ")[0].indexOf(date) > -1);
                }
            }
        });
        $('#transaction-table').find("tr:gt(0)").remove();
        for (const tx of filtered_txs) {
            $('#transaction-table tbody').append(
                '<tr>'+
                    '<td style="width:40%">'+ tx.address +'</td>'+
                    '<td style="width:20%">'+ tx.coin_name +'</td>'+
                    '<td style="width:10%">'+ tx.amount +'</td>'+
                    '<td style="width:10%">'+ tx.type_name +'</td>'+
                    '<td style="width:5%; white-space:nowrap;">'+ tx.transaction_date.split(" ")[0] +'</td>'+
                    '<td style="width:15%">'+ tx.transaction_date.split(" ")[1] +'</td>'+
                '</tr>'
            );
        }
        pagination('Transaction-pagination-list', 'transaction-table', 'transaction-div-table', 'Transaction-pagination-lquo-button', 'Transaction-pagination-rquo-button', 'transaction-pagination-current-page-number', 'transaction-pagination-last-page-number', (Math.ceil(screen.height / 100) - 2), 'pagination-night-theme', 'pagination-day-theme', 'transaction-pagination-info');
    });
}

function sortTable(n, tableId, lquo, pagination_list, rowsShown) {
    var table, rows, switching, i, x, y, shouldSwitch, dir, switchcount = 0;
    console.log(tableId)
    switch(tableId){
      case 'transaction-table':
        pagination('Transaction-pagination-list', 'transaction-table', 'transaction-div-table', 'Transaction-pagination-lquo-button', 'Transaction-pagination-rquo-button', 'transaction-pagination-current-page-number', 'transaction-pagination-last-page-number', (Math.ceil(screen.height / 100) - 2), 'pagination-night-theme', 'pagination-day-theme', 'transaction-pagination-info');
        if($('#transactionTable tbody tr').length == 0){
          document.getElementById("transaction-pagination-current-page-number").innerHTML = "0 ";  
        }else{
          document.getElementById("transaction-pagination-current-page-number").innerHTML = "1 ";
        }
      break;
      case 'wallet-modal-contact-table':
      pagination('wallet-pagination-list', 'wallet-modal-contact-table', 'wallet-div-table-scroll', 'wallet-pagination-lquo-button', 'wallet-pagination-rquo-button', 'wallet-pagination-current-page-number', 'wallet-pagination-last-page-number', (Math.ceil((screen.height / 100)/2) - 1), 'pagination-night-theme', 'pagination-day-theme', 'wallet-pagination-info');        if($('#wallet-modal-contact-table tbody tr').length == 0){
          document.getElementById("wallet-pagination-current-page-number").innerHTML = "0 "; 
        }else{
          document.getElementById("wallet-pagination-current-page-number").innerHTML = "1 "; 
        }
      break;
      case 'contact-table':
        pagination('Contacts-pagination-list', 'contact-table', 'contact-div-table-scroll', 'Contacts-pagination-lquo-button', 'Contacts-pagination-rquo-button', 'contacts-pagination-current-page-number', 'contacts-pagination-last-page-number', (Math.ceil(screen.height / 100) - 2), 'pagination-night-theme', 'pagination-day-theme', 'contacts-pagination-info');
        if($('#contact-table tbody tr').length == 0){
          document.getElementById("contacts-pagination-current-page-number").innerHTML = "0 "; 
        }else{
          document.getElementById("contacts-pagination-current-page-number").innerHTML = "1 "; 
        }
      break;
    }
    
    table = document.getElementById(tableId);
    switching = true;
    dir = "asc";
    while (switching) 
    {
        switching = false;
        rows = table.getElementsByTagName("TR");
        for (i = 1; i < (rows.length - 1); i++) 
        {
            shouldSwitch = false;
            x = rows[i].getElementsByTagName("TD")[n];
            y = rows[i + 1].getElementsByTagName("TD")[n];
            if (dir == "asc") 
            {
                if (x.innerHTML.toLowerCase() > y.innerHTML.toLowerCase()) 
                {
                    shouldSwitch= true;
                    break;
                }
            } else if (dir == "desc") 
            {
                if (x.innerHTML.toLowerCase() < y.innerHTML.toLowerCase()) 
                {
                    shouldSwitch= true;
                    break;
                }
            }
        }
        if (shouldSwitch) {
            rows[i].parentNode.insertBefore(rows[i + 1], rows[i]);
            switching = true;
            switchcount ++; 

            $('#'+tableId+' tbody tr').hide();
            $('#'+tableId+' tbody tr').slice(0, rowsShown).show();
            $('#'+pagination_list+' a').removeClass('disabled');
            $('#btnPagination-'+tableId+'-0').addClass('disabled');
            $('#'+lquo).addClass('disabled');
            for(i = 5; i <= pagination_endpoints.page_number; i++){
                $('#btnPagination-'+tableId+'-'+i).hide();
            }
            for(i = 0; i < 5; i++){
                $('#btnPagination-'+tableId+'-'+i).show();
            }
        } else {
            if (switchcount == 0 && dir == "asc") 
            {
                dir = "desc";
                switching = true;
            }
        }
    }
    if(document.getElementById("select-mode").selectedIndex == 0){
      $('#'+tableId+' tbody tr:odd').css({'background-color':'#2e3438'});
      $('#'+tableId+' tbody tr:even').css({'background-color':'#3E464C'});
    }else{
      $('#'+tableId+' tbody tr:odd').css({'background-color':'#cccccc'});
      $('#'+tableId+' tbody tr:even').css({'background-color':'#efefef'});
    }
    $('#'+tableId+' tbody tr').css('opacity','0.0').hide().slice(0,rowsShown).css('display','table-row').animate({opacity:1}, 300);
}

function pagination(pagination_list, table, table_div, lquo, rquo, currentPage, lastPage, rows, night, day, footer){

    $('#'+table+' tbody tr').css('opacity','0.0').hide().slice(0,rows).css('display','table-row').animate({opacity:1}, 300);
    if(document.getElementById(pagination_list+'-div')){
      document.getElementById(pagination_list+'-div').remove();
    }
    
    $('#'+footer).after('<div class="col-4" style="display:flex;" id="'+pagination_list+'-div"><ul id="'+pagination_list+'" class="pagination" style="margin:auto auto"></ul></div>');
    var rowsShown = rows;
    var rowsTotal = $('#'+table+' tbody tr').length;
    var numPages = rowsTotal/rowsShown;
    pagination_endpoints.page_number = numPages;
    
    if(rowsTotal == 0) {
      document.getElementById(currentPage).innerHTML = "0 ";
    }else{
      document.getElementById(currentPage).innerHTML = "1 ";
    }
        for(var i = 0; i < numPages; i++){
            var pageNum = i + 1;
            if(document.getElementById("select-mode").selectedIndex == 0){
                $('#'+pagination_list).append('<li class="'+pagination_list+'"><a id="btnPagination-'+table+'-'+i+'" class="page-link btnPagination activeBtnPagination '+night+'" rel="'+i+'">'+pageNum+'</a></li>');
            }else{
                $('#'+pagination_list).append('<li class="'+pagination_list+'"><a id="btnPagination-'+table+'-'+i+'" class="page-link btnPagination activeBtnPagination '+day+'" rel="'+i+'">'+pageNum+'</a></li>');
            }
        }
        for(var i = 5; i < numPages; i++){
            $('#btnPagination-'+table+'-'+i).hide();
        }

    $('#btnPagination-'+table+'-0').addClass('disabled');
    if(document.getElementById("select-mode").selectedIndex == 0){
        $('#'+pagination_list).prepend('<li class="page-item"><a id="'+lquo+'" class="page-link '+night+' disabled" rel="'+0+'" aria-label="Previous"><span aria-hidden="true">&laquo;</span><span class="sr-only">Previous</span></a></li>');
        $('#'+pagination_list).append('<li class="page-item"><a id="'+rquo+'" class="page-link '+night+'" rel="'+(Math.ceil(numPages)-1)+'" aria-label="Next"><span aria-hidden="true">&raquo;</span><span class="sr-only">Next</span></a></li>');
    }else{
        $('#'+pagination_list).prepend('<li class="page-item"><a id="'+lquo+'" class="page-link '+day+' disabled" rel="'+0+'" aria-label="Previous"><span aria-hidden="true">&laquo;</span><span class="sr-only">Previous</span></a></li>');
        $('#'+pagination_list).append('<li class="page-item"><a id="'+rquo+'" class="page-link '+day+'" rel="'+(Math.ceil(numPages)-1)+'" aria-label="Next"><span aria-hidden="true">&raquo;</span><span class="sr-only">Next</span></a></li>');                
    }


    $('#'+table+' tbody tr').hide();
    if(document.getElementById("select-mode").selectedIndex == 0){
        $('#'+table+' tbody tr:odd').css({'background-color':'#2e3438'});
        $('#'+table+' tbody tr:even').css({'background-color':'#3E464C'});
    }else{
        $('#'+table+' tbody tr:odd').css({'background-color':'#cccccc'});
        $('#'+table+' tbody tr:even').css({'background-color':'#efefef'});
    }
    $('#'+table+' tbody tr').slice(0, rowsShown).show();
    document.getElementById(lastPage).innerHTML = Math.ceil(numPages);
    if($('#'+table+' tbody tr').length <= rows){
      document.getElementById(rquo).classList.add("disabled");
    }

    $('#'+pagination_list+' a').unbind().on('click', function(e){

        $('#'+pagination_list+' a').removeClass('disabled');
        var currPage = $(this).attr('rel');
        var startItem = currPage*rowsShown;
        var endItem = startItem+rowsShown;
        var currPageInt = (parseInt(currPage));
        var row = document.getElementById(table).getElementsByTagName("tr");
        
        pagination_endpoints.current_page = currPage;
        pagination_endpoints.start_index = startItem;
        pagination_endpoints.end_index = endItem;

        document.getElementById(currentPage).innerHTML = (currPageInt+1)+" ";
        $('#'+table+' tbody tr').css('opacity','0.0').hide().slice(startItem,endItem).css('display','table-row').animate({opacity:1}, 300);
       

        $('#btnPagination-'+table+'-'+currPage).addClass('disabled');
        $('#btnPagination-'+table+'-'+(currPageInt-4)).hide();
        $('#btnPagination-'+table+'-'+(currPageInt+1)).show();
        $('#btnPagination-'+table+'-'+(currPageInt-1)).show();
        $('#btnPagination-'+table+'-'+(currPageInt+4)).hide();

        if(currPage == 0){
            $('#'+lquo).addClass('disabled');
            for(i = 5; i <= numPages; i++){
                $('#btnPagination-'+table+'-'+i).hide();
            }
            for(i = 0; i < 5; i++){
                $('#btnPagination-'+table+'-'+i).show();
            }
        }else{
            $('#'+lquo).removeClass('disabled');
        }
        if(currPage == Math.ceil(numPages-1)){

            $('#'+rquo).addClass('disabled');
            for(i = 0; i <= currPageInt - 5; i++){
                $('#btnPagination-'+table+'-'+i).hide();
            }
            for(i = Math.ceil(numPages-5); i < Math.ceil(numPages); i++){
                $('#btnPagination-'+table+'-'+i).show();
            }
        }else{
            $('#'+rquo).removeClass('disabled');
        }
    });
}

function newPagination(table, pagination_list, table_div, rows, lquo, rquo, currentPage, lastPage, night, day, footer){
    var rowsShown = rows;
    var newRows = $('#'+table+' .showing').length;
    var newPages = Math.ceil(newRows/rowsShown);

    if(document.getElementById(pagination_list+'-div')){
        document.getElementById(pagination_list+'-div').remove();
    }

    if(newRows == 0) {
      document.getElementById(currentPage).innerHTML = "0 ";
    }else{
      document.getElementById(currentPage).innerHTML = "1 ";
    }

    $('#'+footer).after('<div id="'+pagination_list+'-div" class="col-4" style="display:flex"><ul id="'+pagination_list+'" class="pagination" style="margin: auto "></ul></div>');

    for(var i = 0; i < newPages; i++){
        var pageNum = i + 1;
        if(document.getElementById("select-mode").selectedIndex == 0){
            $('#'+pagination_list).append('<li class="Transaction-list-pagination"><a id="btnPagination-'+table+'-'+i+'" class="page-link btnPagination activeBtnPagination '+night+'" rel="'+i+'">'+pageNum+'</a></li>');
        }else{
            $('#'+pagination_list).append('<li class="Transaction-list-pagination"><a id="btnPagination-'+table+'-'+i+'" class="page-link btnPagination activeBtnPagination '+day+'" rel="'+i+'">'+pageNum+'</a></li>');
        }
    }
    for(var i = 5; i < newPages; i++){
        $('#btnPagination-'+table+'-'+i).hide();
    }

    $('#btnPagination-'+table+'-0').addClass('disabled');	
    if(document.getElementById("select-mode").selectedIndex == 0){
        $('#'+pagination_list).prepend('<li class="page-item"><a id="'+lquo+'" class="page-link pagination-night-theme disabled" rel="'+0+'" aria-label="Previous"><span aria-hidden="true">&laquo;</span><span class="sr-only">Previous</span></a></li>');
        $('#'+pagination_list).append('<li class="page-item"><a id="'+rquo+'" class="page-link pagination-night-theme" rel="'+(newPages-1)+'" aria-label="Next"><span aria-hidden="true">&raquo;</span><span class="sr-only">Next</span></a></li>');
    }else{
        $('#'+pagination_list).prepend('<li class="page-item"><a id="'+lquo+'" class="page-link pagination-day-theme disabled" rel="'+0+'" aria-label="Previous"><span aria-hidden="true">&laquo;</span><span class="sr-only">Previous</span></a></li>');
        $('#'+pagination_list).append('<li class="page-item"><a id="'+rquo+'" class="page-link pagination-day-theme" rel="'+(newPages-1)+'" aria-label="Next"><span aria-hidden="true">&raquo;</span><span class="sr-only">Next</span></a></li>');                
    }
    if(document.getElementById("select-mode").selectedIndex == 0){
        $('#'+table+' .showing:odd').css({'background-color':'#2e3438'});
        $('#'+table+' .showing:even').css({'background-color':'#3E464C'});
    }else{
        $('#'+table+' .showing:odd').css({'background-color':'#cccccc'});
        $('#'+table+' .showing:even').css({'background-color':'#efefef'});
    }
    $('#'+table+' tbody tr .showing').slice(0, rowsShown).show();

    document.getElementById(lastPage).innerHTML = Math.ceil(newPages);
    $('#'+pagination_list+' a').unbind().on('click', function(e){
        e.preventDefault();
        $('#'+pagination_list+' a').removeClass('disabled');
        var currPage = $(this).attr('rel');
        var startItem = currPage*rowsShown;
        var endItem = startItem+rowsShown;
        var currPageInt = (parseInt(currPage));
        
        pagination_endpoints.current_page = currPage;
        pagination_endpoints.start_index = startItem;
        pagination_endpoints.end_index = endItem;

        document.getElementById(currentPage).innerHTML = (currPageInt+1)+" ";
        $('#'+table+' .showing').css('opacity','0.0').hide().slice(startItem,endItem).css('display','table-row').animate({opacity:1}, 300);
        $('#btnPagination-'+table+'-'+currPage).addClass('disabled');
        $('#btnPagination-'+table+'-'+(currPageInt-4)).hide();
        $('#btnPagination-'+table+'-'+(currPageInt+1)).show();
        $('#btnPagination-'+table+'-'+(currPageInt-1)).show();
        $('#btnPagination-'+table+'-'+(currPageInt+4)).hide();

        if(currPage == 0){
            $('#'+lquo).addClass('disabled');
            for(i = 5; i <= newPages; i++){
                $('#btnPagination-'+table+'-'+i).hide();
            }
            for(i = 0; i < 5; i++){
                $('#btnPagination-'+table+'-'+i).show();
            }
        }else{
            $('#'+lquo).removeClass('disabled');
        }
        if(currPage == Math.ceil(newPages-1)){
            $('#'+rquo).addClass('disabled');
            for(i = 0; i <= currPageInt - 5; i++){
                $('#btnPagination-'+table+'-'+i).hide();
            }
            for(i = Math.ceil(newPages-5); i < Math.ceil(newPages); i++){
                $('#btnPagination-'+table+'-'+i).show();
            }
        }else{
            $('#'+rquo).removeClass('disabled');
        }
    });
}

function comma_splitter(numero){
    var numero, x, x1, x2;
    numero += '';
    x = numero.split('.');
    x1 = x[0];
    x2 = x.length > 1 ? '.' + x[1] : '';
    var rgx = /(\d+)(\d{3})/;

    while (rgx.test(x1)) {
      x1 = x1.replace(rgx, '$1' + ',' + '$2');
    }

    return x1 + x2;
}



/******************************************************************************************************
 *                                    Contacts functions                                               *
 ******************************************************************************************************/
var sqlite3 = require('sqlite3').verbose();
var total_array, contacts, index;


function addContact(){

    var id;
    if(contacts.length == 0){
        id = 0;
    }else{
        id = contacts[contacts.length-1].contact_id + 1;
    }
    var name = document.getElementById('add-contacts-modal-name-input').value;
    var address = document.getElementById('add-contacts-modal-address-input').value;  
    var coin = document.getElementById("add-contacts-modal-coin-select").value;
    var type;
    let db = new sqlite3.Database('./cypher.db');

    document.querySelectorAll("#add-contact-modal-content input").forEach( input => {
        if(input.value.length == 0){
            input.style.boxShadow = "red 0px 0px 2px 2px";
            document.getElementById("add-contacts-modal-add-btn").removeAttribute("data-dismiss", "modal");
        }else{
            input.style.boxShadow = "none";
        }
    });

    if(document.getElementById("add-contacts-modal-name-input").value != "" && document.getElementById("add-contacts-modal-address-input").value != ""){
        document.getElementById("add-contacts-modal-add-btn").setAttribute("data-dismiss", "modal");
        // insert contact
        db.run(`INSERT INTO Contact(contact_name, coin_address, coin_id) VALUES(?,?,?)`, [name, address, coin], function(err) {
            if (err) {
                return console.log(err.message);
            }
            $('#contact-table').append(
                '<tr>'+
                    '<td style="width: 28%">'+name+'</td>'+
                    '<td style="width: 28%">'+address+'</td>'+
                    '<td style="width: 24%">'+coins[document.getElementById("add-contacts-modal-coin-select").value-1].coin_name+'</td>'+
                    '<td style="width: 16%">'+
                        '<button class="btn btn-warning" onclick="loadEditModal(this,'+id+')" data-toggle="modal" data-target="#edit-contact-modal" style="margin-right: 5px;"><span style="color: white;">'+
                            '<i class="fa fa-pencil"></i>'+
                        '</span></button>'+
                        '<button class="btn btn-danger" onclick="deleteContact(this, '+id+')"><span style="color: white;">'+
                            '<i class="fa fa-trash"></i>'+
                        '</span></button>'+
                    '</td>'+                            
                '</tr>'
            ); 
            document.getElementById('add-contacts-modal-name-input').value = "";
            document.getElementById('add-contacts-modal-address-input').value = "";
            document.getElementById("add-contacts-modal-coin-select").selectedIndex = 0;
            pagination('Contacts-pagination-list', 'contact-table', 'contact-div-table-scroll', 'Contacts-pagination-lquo-button', 'Contacts-pagination-rquo-button', 'contacts-pagination-current-page-number', 'contacts-pagination-last-page-number', (Math.ceil(screen.height / 100) - 2), 'pagination-night-theme', 'pagination-day-theme', 'contacts-pagination-info');
            contacts.push({coin_address: address, contact_id: id, contact_name: name, crypto_id: coin});
            
            document.getElementById("add-header-info").innerHTML = default_language.success+"!";
            document.getElementById("contact-modal-added-message").innerHTML = default_language.contact_added;
            $('#contact-modal-added').modal('show');
            setTimeout( () => {
                $('#contact-modal-added').modal('hide');
            }, 1500);
        });
        
        // close the database connection
        db.close();
    }else{
        document.getElementById("add-contacts-modal-add-btn").removeAttribute("data-dismiss", "modal");
    }
}

function editContact(row){

    var name = document.getElementById('edit-contacts-modal-name-input').value;
    var address = document.getElementById('edit-contacts-modal-address-input').value;  
    var coin = document.getElementById("edit-contacts-modal-coin-select").value;
    let db = new sqlite3.Database('./cypher.db');

    document.querySelectorAll("#edit-contact-modal-content input").forEach( input => {
        if(input.value.length == 0){
            input.style.boxShadow = "red 0px 0px 2px 2px";
            document.getElementById("edit-contacts-modal-edit-btn").removeAttribute("data-dismiss", "modal");
        }else{
            input.style.boxShadow = "none";
        }
    });

    if(document.getElementById("edit-contacts-modal-name-input").value != "" && document.getElementById("edit-contacts-modal-address-input").value != ""){
        document.getElementById("edit-contacts-modal-edit-btn").setAttribute("data-dismiss", "modal");
        document.getElementById("modal-btn-warning-yes").innerHTML = default_language.yes;
        document.getElementById("modal-btn-warning-no").innerHTML = default_language.no;
        document.getElementById("modal-warning-header-info").innerHTML = default_language.warning;
        document.getElementById("modal-warning-body-info").innerHTML = default_language.edit_contact_confirm;
        $('#modal-message-warning').modal('show');
        $('#modal-btn-warning-yes').unbind().click( (e) => {
            // edit contact
            db.run(`UPDATE Contact SET contact_name = ?, coin_address = ?, coin_id = ? WHERE contact_id = ?`, [name, address, coin, contact_id], function(err) {
                if (err) {
                    return console.log(err.message);
                }else{
                    document.getElementById("contact-table").getElementsByTagName("tr")[row_index].getElementsByTagName("td")[0].innerHTML = name;
                    document.getElementById("contact-table").getElementsByTagName("tr")[row_index].getElementsByTagName("td")[1].innerHTML = address;
                    document.getElementById("contact-table").getElementsByTagName("tr")[row_index].getElementsByTagName("td")[2].innerHTML = coins[document.getElementById("edit-contacts-modal-coin-select").value - 1].coin_name;
                    document.getElementById("add-header-info").innerHTML = default_language.success+"!";
                    document.getElementById("contact-modal-added-message").innerHTML = default_language.contact_edited;
                    $('#contact-modal-added').modal('show');
                    setTimeout( () => {
                        $('#contact-modal-added').modal('hide');
                    }, 1500);
                }
            });  
            // close the database connection
            db.close();
        });
    }else{
        document.getElementById("edit-contacts-modal-edit-btn").removeAttribute("data-dismiss", "modal");
    }
}

function deleteContact(row, id){
    
    document.getElementById("btn-delete-contact-confirm").innerHTML = default_language.yes;
    document.getElementById("btn-delete-contact-dismiss").innerHTML = default_language.no;
    document.getElementById("delete-header-info").innerHTML = default_language.delete;
    document.getElementById("contact-modal-delete-message").innerHTML = default_language.delete_contact;
    $('#contact-modal-deleted').modal('show');

    $('#btn-delete-contact-confirm').unbind().click( (event) => {
        let db = new sqlite3.Database('./cypher.db');
        var index_delete = row.parentNode.parentNode.rowIndex-1;
        db.run(`DELETE FROM Contact WHERE contact_id = ?`, [id], function(err) {
            if (err) {
                return console.log(err.message);
            }
        });
        db.close();
        document.getElementById('contact-table').deleteRow(row.parentNode.parentNode.rowIndex);
        pagination('Contacts-pagination-list', 'contact-table', 'contact-div-table-scroll', 'Contacts-pagination-lquo-button', 'Contacts-pagination-rquo-button', 'contacts-pagination-current-page-number', 'contacts-pagination-last-page-number', (Math.ceil(screen.height / 100) - 2), 'pagination-night-theme', 'pagination-day-theme', 'contacts-pagination-info');
        $('#btnPaginationContacts-'+pagination_endpoints.current_page).addClass('disabled');
    });        
}

function loadEditModal(row, id){
    row_index = row.parentNode.parentNode.rowIndex;
    contact_id = id;
    var name = document.getElementById('contact-table').rows.item(row_index).cells.item(0).innerHTML;
    var address = document.getElementById('contact-table').rows.item(row_index).cells.item(1).innerHTML;
    var coin = document.getElementById('contact-table').rows.item(row_index).cells.item(2).innerHTML;    
    document.getElementById('edit-contacts-modal-name-input').value = name;
    document.getElementById('edit-contacts-modal-address-input').value = address;
    document.getElementById('edit-contacts-modal-coin-select').selectedIndex = coin_array.indexOf(coin);
}

function searchName(table_id, input_id, disable_id, pagination_list, table_div, rows, lquo, rquo, current_page, last_page, footer){
    var input, filter, table, tr, td, i;
    input = document.getElementById(input_id);
    table = document.getElementById(table_id);
    tr = table.getElementsByTagName("tr");

    if(input.value == ""){
        for(i=1; i<tr.length; i++){
            tr[i].style.display = "";
            tr[i].classList.add("showing");
        }
        document.getElementById(disable_id).readOnly = false;
    }else{
        document.getElementById(disable_id).readOnly = true;
        filter = input.value.toUpperCase();
        for (i = 0; i < tr.length; i++) {
            td = tr[i].getElementsByTagName("td")[0];
            if (td) {
                if (td.innerHTML.toUpperCase().indexOf(filter) > -1) {
                    tr[i].style.display = "";
                    tr[i].classList.add("showing");
                } else {
                    tr[i].style.display = "none";
                    tr[i].classList.remove("showing");
                }
            }       
        }
    }

    if(table_id == "wallet-modal-contact-table"){
        $('#'+table_id+' .showing').css('opacity','0.0').hide().slice(0, (Math.ceil((screen.height / 100)/2) - 1)).css('display','table-row').animate({opacity:1}, 300);
        newPagination(table_id, pagination_list, table_div, (Math.ceil((screen.height / 100)/2) - 1), lquo, rquo, current_page, last_page, 'pagination-night-theme', 'pagination-day-theme', footer);
    }else{
        $('#'+table_id+' .showing').css('opacity','0.0').hide().slice(0, Math.ceil(screen.height / 100) - 2).css('display','table-row').animate({opacity:1}, 300);
        newPagination(table_id, pagination_list, table_div, Math.ceil(screen.height / 100) - 2, lquo, rquo, current_page, last_page, 'pagination-night-theme', 'pagination-day-theme', footer);
    }

    if(document.getElementById(table_id).getElementsByClassName("showing").length < Math.ceil(screen.height / 100) - 2){
        document.getElementById(rquo).classList.add("disabled");
    }else{
        document.getElementById(rquo).classList.remove("disabled");
    }
}

function searchWalletAddress(event, pagination_list, table_div, rows, lquo, rquo, current_page, last_page, footer){
    var val = event.target.value, table, tr, td, td2;
    table = document.getElementById("wallet-modal-receive-table");
    tr = table.getElementsByTagName("tr");

    if(val && val.trim() == ""){
        for(let i = 1; i < tr.length; i++){
            tr[i].style.display = "";
            tr[i].classList.add("showing");
        }
    }else{
        for(let i = 0; i < tr.length; i++){
            td = tr[i].getElementsByTagName("td")[0];
            td2 = tr[i].getElementsByTagName("td")[2];
            if(td){
                if(td.innerHTML.toLowerCase().indexOf(val.toLowerCase()) > -1 || td2.innerHTML.toLowerCase().indexOf(val.toLowerCase()) > -1 || td2.innerHTML.normalize('NFD').replace(/[\u0300-\u036f]/g, "").toLowerCase().indexOf(val.toLowerCase()) > -1){
                    tr[i].style.display = "";
                    tr[i].classList.add("showing");
                }else{
                    tr[i].style.display = "none";
                    tr[i].classList.remove("showing");
                }
            }
        }
    }

    $('#wallet-modal-receive-table .showing').css('opacity','0.0').hide().slice(0, (Math.ceil((screen.height / 100)/2) - 1)).css('display','table-row').animate({opacity:1}, 300);
    newPagination("wallet-modal-receive-table" ,'wallet-modal-receive-pagination-list' ,'wallet-modal-receive-div-table-scroll', (Math.ceil((screen.height / 100)/2) - 1), 'wallet-modal-receive-pagination-lquo-button', 'wallet-modal-receive-pagination-rquo-button', 'wallet-modal-receive-pagination-current-page-number', 'wallet-modal-receive-pagination-last-page-number', 'pagination-night-theme', 'pagination-day-theme', 'wallet-modal-receive-pagination-info');

    if(table.getElementsByClassName("showing").length < (Math.ceil((screen.height / 100)/2) - 1)){
        document.getElementById('wallet-pagination-rquo-button-'+currency).classList.add("disabled");
    }else{
        document.getElementById('wallet-pagination-rquo-button-'+currency).classList.remove("disabled");
    }
}
  
function searchAddress(table_id, input_id, disable_id, pagination_list, table_div, rows, lquo, rquo, current_page, last_page, footer, n){
    var input, filter, table, tr, td, i;
    input = document.getElementById(input_id);
    table = document.getElementById(table_id);
    tr = table.getElementsByTagName("tr");

    if(input.value == ""){
        for(i=1; i<tr.length; i++){
            tr[i].style.display = "";
            tr[i].classList.add("showing");
        }
        if(disable_id != ""){
            document.getElementById(disable_id).readOnly = false;
        }
    }else{
        if(disable_id != ""){
            document.getElementById(disable_id).readOnly = true;
        }
        filter = input.value.toUpperCase();
        for (i = 0; i < tr.length; i++) {
            td = tr[i].getElementsByTagName("td")[n];
            if (td) {
                if (td.innerHTML.toUpperCase().indexOf(filter) > -1) {
                    tr[i].style.display = "";
                    tr[i].classList.add("showing");
                } else {
                    tr[i].style.display = "none";
                    tr[i].classList.remove("showing");
                }
            }       
        }
    }

    if(table_id == "wallet-modal-contact-table"){
        $('#'+table_id+' .showing').css('opacity','0.0').hide().slice(0, (Math.ceil((screen.height / 100)/2) - 1)).css('display','table-row').animate({opacity:1}, 300);
        newPagination(table_id, pagination_list, table_div, (Math.ceil((screen.height / 100)/2) - 1), lquo, rquo, current_page, last_page, 'pagination-night-theme', 'pagination-day-theme', footer);
    }else{
        $('#'+table_id+' .showing').css('opacity','0.0').hide().slice(0, Math.ceil(screen.height / 100) - 2).css('display','table-row').animate({opacity:1}, 300);
        newPagination(table_id, pagination_list, table_div, Math.ceil(screen.height / 100) - 2, lquo, rquo, current_page, last_page, 'pagination-night-theme', 'pagination-day-theme', footer);
    }

    if(document.getElementById(table_id).getElementsByClassName("showing").length <= Math.ceil(screen.height / 100) - 2){
        document.getElementById(rquo).classList.add("disabled");
    }else{
        document.getElementById(rquo).classList.remove("disabled");
    }
}

function loadContacts() {
    contactsTutorial();
    var db = new sqlite3.Database('./cypher.db');
    var sql = 'SELECT * FROM contact a INNER JOIN coin b ON a.coin_id = b.coin_id;';
    var type;
    db.all(sql, [], (err, rows) => {
        if (err) {
          throw err;
        }
        contacts = rows;
        $('#contact-table').find("tr:gt(0)").remove();
        for(var i=0; i<contacts.length; i++){
            $('#contact-table').append(
                '<tr>'+
                    '<td style="width: 30%">'+contacts[i].contact_name+'</td>'+
                    '<td style="width: 30%">'+contacts[i].coin_address+'</td>'+
                    '<td style="width: 24%">'+contacts[i].coin_name+'</td>'+
                    '<td style="width: 16%;">'+
                        '<button id="contact-button-edit" class="btn btn-warning" onclick="loadEditModal(this, '+contacts[i].contact_id+')" data-toggle="modal" data-target="#edit-contact-modal" style="margin-right: 5px;"><span style="color: white;">'+
                            '<i class="fa fa-pencil"></i>'+
                        '</span></button>'+
                        '<button id="contact-button-delete" class="btn btn-danger" onclick="deleteContact(this, '+contacts[i].contact_id+')"><span>'+
                            '<i class="fa fa-trash"></i>'+
                        '</span></button>'+
                    '</td>'+
                '</tr>'
            );
        }
        pagination('Contacts-pagination-list', 'contact-table', 'contact-div-table-scroll', 'Contacts-pagination-lquo-button', 'Contacts-pagination-rquo-button', 'contacts-pagination-current-page-number', 'contacts-pagination-last-page-number', (Math.ceil(screen.height / 100) - 2), 'pagination-night-theme', 'pagination-day-theme', 'contacts-pagination-info');
    });
    db.close();
}

function loadContactsModal(coin){
    walletContactsTutorial();
    document.getElementById("wallet-modal-add-contact-image").setAttribute("src", "images/"+coin.image);
    document.getElementById("wallet-modal-add-contact-title").innerHTML = coin.label + " ("+ coin.prefix +") " + default_language.contacts;
    var db = new sqlite3.Database('./cypher.db');
    var sql = "SELECT * FROM contact a inner join coin b on a.coin_id = b.coin_id where b.coin_name = ?";
    db.all(sql, [coin.label], ( err, rows ) => {
        const contacts = rows;       
        $('#wallet-modal-contact-table').find("tr:gt(0)").remove();
        for(var i=0; i<contacts.length; i++){
            var send = contacts[i].coin_address;
            $('#wallet-modal-contact-table').append(
                '<tr>'+
                    '<td style="width: 35%">'+contacts[i].contact_name+'</td>'+
                    '<td style="width: 35%">'+contacts[i].coin_address+'</td>'+
                    '<td style="width: 24%">'+contacts[i].coin_name+'</td>'+
                    '<td style="width: 6%; text-align:center">'+
                        '<button id="button-send_address" type="button" class="btn btn-success" data-dismiss="modal" onclick=wallet_modal_add_contact(\''+ send +'\')>+</button>'+
                    '</td>'+
                '</tr>'
            );
        }
        $('#wallet-modal-contact-table tbody tr').css('opacity','0.0').hide().slice(0, (Math.ceil((screen.height / 100)/2) - 1)).css('display','table-row').animate({opacity:1}, 300);
        pagination('wallet-pagination-list', 'wallet-modal-contact-table', 'wallet-div-table-scroll', 'wallet-pagination-lquo-button', 'wallet-pagination-rquo-button', 'wallet-pagination-current-page-number', 'wallet-pagination-last-page-number', (Math.ceil((screen.height / 100)/2) - 1), 'pagination-night-theme', 'pagination-day-theme', 'wallet-pagination-info');
    });
}

function wallet_modal_add_contact(receive){
    document.getElementById("wallet-modal-input-address").value = receive
}

function loadAddresses(coin){
    receiveTutorial();
    var sql = "SELECT * FROM addresses a INNER JOIN wallet b ON a.wallet_id = b.wallet_id where b.wallet_id = ? and a.coin_id = ?";
    var type;
    
    document.getElementById("wallet-modal-receive-title").innerHTML = coin.label + " (" + coin.prefix + ")" + " " + default_language.addresses;
    document.getElementById("wallet-modal-receive-image").setAttribute("src", "images/"+coin.image+"");
    $('#wallet-modal-receive').modal("show");
    db.all(sql, [selected_wallet, coin_array.indexOf(coin.label) + 1], ( err, rows ) => {
        const addresses = rows;
        $('#wallet-modal-receive-table').find("tr:gt(0)").remove();
        for(var i=0; i<addresses.length; i++){
            $('#wallet-modal-receive-table').append(
                '<tr id="wallet-modal-receive-first_row">'+
                    '<td id="wallet-modal-receive-first-address" style="width: 40%">'+addresses[i].address+'</td>'+
                    '<td style="width: 40%">'+addresses[i].description+'</td>'+
                    '<td style="width: 20%;">'+
                        '<button class="btn btn-primary" onclick="loadQR(this)"><span style="color: white;">'+
                            '<i class="fa fa-qrcode"></i>'+
                        '</span></button>'+
                    '</td>'+
                '</tr>'
            );
        }
        $('#wallet-modal-receive-table tbody tr').css('opacity','0.0').hide().slice(0, (Math.ceil((screen.height / 100)/2) - 1)).css('display','table-row').animate({opacity:1}, 300);
        pagination('wallet-modal-receive-pagination-list', 'wallet-modal-receive-table', 'wallet-modal-receive-div-table-scroll', 'wallet-modal-receive-pagination-lquo-button', 'wallet-modal-receive-pagination-rquo-button', 'wallet-modal-receive-pagination-current-page-number', 'wallet-modal-receive-pagination-last-page-number', (Math.ceil((screen.height / 100)/2) - 1), 'pagination-night-theme', 'pagination-day-theme', 'wallet-modal-receive-pagination-info');
    });
}

/*
* MONEY 
*/

async function loadTransactions(){
    transactionTutorial();
    return new Promise( async (resolve, reject) => {
        var db = new sqlite3.Database('./cypher.db');
        var sql = "select address, coin_name, amount, type_name, status_name, transaction_date from transactions a inner join wallet b on a.wallet_id = b.wallet_id inner join coin c on a.coin_id = c.coin_id inner join types d on a.type_id = d.type_id inner join status e on a.status_id = e.status_id where b.wallet_id = ?";
        await db.all(sql, [selected_wallet], (err, rows) => {
            var transactions, date, hour;
            if(err){
                reject(err);
            }
            transactions = rows;
            $('#transaction-table').find("tr:gt(0)").remove();
            for(var i = 0; i < transactions.length; i++){
                date = transactions[i].transaction_date.split(" ")[0];
                hour = transactions[i].transaction_date.split(" ")[1];
            $('#transaction-table tbody').append(
                '<tr>'+
                '<td style="width:40%">'+ transactions[i].address +'</td>'+
                '<td style="width:20%">'+ transactions[i].coin_name +'</td>'+
                '<td style="width:10%">'+ transactions[i].amount +'</td>'+
                '<td style="width:10%">'+ transactions[i].type_name +'</td>'+
                '<td style="width:5%; white-space:nowrap;">'+ date +'</td>'+
                '<td style="width:15%">'+ hour +'</td>'+
                '</tr>'
                );
            }
            pagination('Transaction-pagination-list', 'transaction-table', 'transaction-div-table', 'Transaction-pagination-lquo-button', 'Transaction-pagination-rquo-button', 'transaction-pagination-current-page-number', 'transaction-pagination-last-page-number', (Math.ceil(screen.height / 100) - 2), 'pagination-night-theme', 'pagination-day-theme', 'transaction-pagination-info');
            resolve(transactions);
        });
    });
}

function loadQR(row){
    console.log(row.parentNode.parentNode);
    row = row.parentNode.parentNode;
    var row_index = row.rowIndex, qr;
    var tr = document.getElementById("wallet-modal-receive-table").getElementsByTagName("tr")[row_index];
    qr = qrcode(0, 'L');
    qr.addData(tr.getElementsByTagName("td")[0].innerHTML);
    qr.make();
    document.getElementById("QR-image").innerHTML = qr.createImgTag(7);
    //document.getElementById("qr-address-output").innerHTML = tr.getElementsByTagName("td")[0].innerHTML; 
    document.getElementById("modal-QR-header-info").innerHTML = single_coin_selected + " QR";
    $('#modal-QR').modal('show');
}

/******************************************************************************************************
*                                    Settings functions                                               *
******************************************************************************************************/

function changeLanguage() {
    var language_index = document.getElementById("select-language").selectedIndex;
    $.ajax({
        type: "GET",
        url: "language.json",
        data : "",
        dataType: "json",
        contentType : 'application/json',
        success: function (response) {
            try{
                default_language = response[language_index];
                changeLabels();
            }catch(Exception){
            }
        },
        error: function (jqXHR, exception) { CRDates
            alert("Error");
        }
    });
    let db = new sqlite3.Database('./cypher.db');
    // edit language
    db.run(`UPDATE settings SET language = ?`, [language_index], function(err) {
        if (err) {
            return console.log(err.message);
        }
    });
    db.close();
}

function changeMode() {
    var theme_index = document.getElementById("select-mode").selectedIndex;
    if(theme_index == 0){
        document.getElementsByClassName("logo")[0].style.color = "white";
        document.getElementsByClassName("logo")[0].style.backgroundColor = "#3E464C";
        document.getElementsByClassName("side-nav")[0].style.backgroundColor = "#3E464C";
        var array_a = document.getElementById("menu").getElementsByTagName("a");
        for (var i=0; i<array_a.length; i++){
            array_a[i].style.color = "white";
        }
        var array_b = document.getElementById("settings_menu").getElementsByTagName("a");
        for (var i=0; i<array_b.length; i++){
            array_b[i].style.color = "white";
        }
        var array_a = document.getElementById("menu").getElementsByTagName("i");
        for (var i=0; i<array_a.length; i++){
            array_a[i].style.color = "white";
        }
        document.getElementsByClassName("bodyClass")[0].style.backgroundImage = "linear-gradient(to bottom, #1e1e1e, #2b2b2b, #393939, #484848, #575757)";
        var portfolio_coins = document.getElementsByClassName("image-container");
        for(var i=0; i<portfolio_coins.length; i++){
            portfolio_coins[i].childNodes[0].style.color = "#f2f2f2";
            portfolio_coins[i].childNodes[1].style.color = "#f2f2f2";
            portfolio_coins[i].childNodes[0].childNodes[0].style.color = "#f2f2f2"
            portfolio_coins[i].childNodes[1].childNodes[0].style.color = "#f2f2f2";
            portfolio_coins[i].childNodes[1].childNodes[1].style.color = "#f2f2f2";
        }
        var wallet_coins = document.getElementsByClassName("wallet-images");
        for(var i=0; i<portfolio_coins.length; i++){
            wallet_coins[i].childNodes[1].style.color = "white";
        }
        document.getElementById("setting-label-language").style.color = "white";
        document.getElementById("setting-label-mode").style.color = "white";
      
        var cards = document.getElementsByClassName("card");
        for (let i = 0; i < cards.length; i++) {
            cards[i].childNodes[1].style.backgroundColor = "#3E464C";
        }

        var div = document.getElementById("accordion").getElementsByClassName("card");
        for(var i=0; i<div.length; i++){
            div[i].style.borderColor = "#3E464C";
        }

        var walletAccordion = document.getElementById("wallet_accordion").getElementsByClassName("wallet_card");
        for (let i = 0; i < walletAccordion.length; i++) {
            walletAccordion.style
        }

        var body = document.getElementsByClassName("card-body");
        for(var i=0; i<body.length; i++){
            body[i].style.backgroundColor = "#3E464C";
        }

        var text = document.getElementById("accordion").getElementsByTagName("p");
        for(var i=0; i<text.length; i++){
            text[i].style.color="white";
        }
        
        var walletAccordion = document.getElementById("wallet_accordion").getElementsByClassName("p");
        for (let i = 0; i < walletAccordion.length; i++) {
            walletAccordion.style.color = "white";
        }

        var h6 = document.getElementsByTagName("h6");
        for(var i=0; i<h6.length; i++){
            h6[i].style.color = "white";
            h6[i].style.textDecoration = "underline";
        }

        var walletCards = document.getElementById("wallet-container").getElementsByClassName("wallet-images");
        for(var i=0; i<walletCards.length; i++){
            walletCards[i].classList.remove("newWalletCards");
            walletCards[i].classList.add("theme_class");
        }

        var portfolioCards = document.getElementsByClassName("image-container");
        for(var i=0; i<portfolioCards.length; i++){
            portfolioCards[i].classList.remove("newPortfolioCards");
            portfolioCards[i].classList.add("theme_class2");
        }
        
        var div = document.getElementById("accordion").getElementsByClassName("card");
        for(var i=0; i<div.length; i++){
            div[i].style.borderColor = "default";
            div[i].classList.remove("newTheme");
            div[i].classList.add("theme");
        }
        
        document.getElementById("mailSender").classList.remove("newMailSender");
        document.getElementById("mailSender").classList.add("mail");

        var walletScroll = document.getElementById("wallet-container");
        walletScroll.classList.remove("newWalletScroll");
        walletScroll.classList.add("walletScroll");

        var helpScroll = document.getElementById("accordion");
        helpScroll.classList.remove("newHelp");
        helpScroll.classList.add("helpNow");

        var portfolioScroll = document.getElementById("cryptoContent");
        portfolioScroll.classList.remove("newPortfolioScroll");
        portfolioScroll.classList.add("portfolioScroll");

        var walletTable = document.getElementById("wallet-modal-contact-table");
        walletTable.classList.remove("table-day-style");
        walletTable.classList.add("table-night-style");

        var contactTable = document.getElementById("contact-table");
        contactTable.classList.remove("table-day-style");
        contactTable.classList.add("table-night-style");

        var contactTableDiv = document.getElementById("contact-table-div");
        contactTableDiv.classList.remove("contact-table-div-day");
        contactTableDiv.classList.add("contact-table-div-night");

        document.getElementById("contact-div-table-scroll").classList.add("contact-div-table-scroll-night");
        document.getElementById("contact-div-table-scroll").classList.remove("contact-div-table-scroll-day");

        document.getElementById("client_wallets_device").classList.add("client_wallets_style_night");
        document.getElementById("client_wallets_device").classList.remove("client_wallets_style_day");

        document.getElementById("client_wallets_nfc").classList.add("client_wallets_style_night");
        document.getElementById("client_wallets_nfc").classList.remove("client_wallets_style_day");
        
        var tables = document.querySelectorAll('#wallet-container table[name="wallet-address-types"]');
        for(var i=0; i<tables.length; i++){
          tables[i].classList.remove('table-day-style');
          tables[i].classList.add('table-night-style');
        }

        var transactionTable = document.getElementById("transaction-table");
        transactionTable.classList.remove("table-day-style");
        transactionTable.classList.add("table-night-style");

        document.getElementById("wallet-modal-contacts-search-name-label").style.color = "white";
        document.getElementById("wallet-modal-contacts-search-address-label").style.color = "white";
        document.getElementById("wallet-modal-add-contact-address").style.backgroundColor = "rgba(0,0,0,0.2)";
        document.getElementById("wallet-modal-add-contact-address-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("wallet-modal-receive-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("wallet-modal-generate-address-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("add-contact-modal-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("edit-contact-modal-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("mailSender").style.backgroundColor = "rgb(129, 120, 120)";
        document.getElementById("mailSender").style.color = "white";
        document.getElementById("collapseOne").style.backgroundColor = "#3E464C";
        document.getElementById("advanced-filter-modal-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("QR-modal-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("wallet-modal-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("add-wallet-modal-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("edit-wallet-modal-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("wallet-modal-confirm-fee-content").style.backgroundColor = "rgba(0,0,0,0.9)";
        document.getElementById("wallet-modal-confirm-fee").style.backgroundColor = "rgba(0,0,0, 0.9)";
        document.getElementById("contact-table").classList.remove("table-day");
        document.getElementById("contact-table").classList.add("table-night");
        document.getElementById("contacts-search-name-label").style.color = "white";
        document.getElementById("contacts-search-address-label").style.color = "white";
        document.getElementById("contacts-pagination-info").classList.remove("contacts-pagination-info-day");
        document.getElementById("contacts-pagination-info").classList.add("contacts-pagination-info-night");
        document.getElementById("transaction-pagination-info").classList.remove("transaction-pagination-info-day");
        document.getElementById("transaction-pagination-info").classList.add("transaction-pagination-info-night");
        document.getElementById("wallet-pagination-info").classList.remove("transaction-pagination-info-day");
        document.getElementById("wallet-pagination-info").classList.add("transaction-pagination-info-night");
        document.getElementById("wallet-modal-receive-pagination-info").style.color = "white";
        document.getElementById("transaction-div-table").classList.remove("transaction-div-table-day");
        document.getElementById("transaction-div-table").classList.add("transaction-div-table-night");
        document.getElementById("wallet-modal-receive-table").classList.remove("table-day-style");
        document.getElementById("wallet-modal-receive-table").classList.add("table-night-style");

        document.getElementById("add-contact-modal-alert-body").classList.add("modal-body-night");
        document.getElementById("add-contact-modal-alert-body").classList.remove("modal-body-day");
        document.getElementById("add-contact-modal-alert-footer").classList.add("modal-body-night");
        document.getElementById("add-contact-modal-alert-footer").classList.remove("modal-body-day");
        
        document.getElementById("delete-contact-modal-alert-body").classList.add("modal-body-night");
        document.getElementById("delete-contact-modal-alert-body").classList.remove("modal-body-day");
        document.getElementById("delete-contact-modal-alert-footer").classList.add("modal-footer-night");
        document.getElementById("delete-contact-modal-alert-footer").classList.remove("modal-footer-day");

        document.getElementById("modal-warning-body").classList.add("modal-body-night");
        document.getElementById("modal-warning-body").classList.remove("modal-body-day");
        document.getElementById("modal-warning-footer").classList.add("modal-body-night");
        document.getElementById("modal-warning-footer").classList.remove("modal-body-day");

        document.getElementById("modal-error-body").classList.add("modal-body-night");
        document.getElementById("modal-error-body").classList.remove("modal-body-day");
        document.getElementById("modal-error-footer").classList.add("modal-body-night");
        document.getElementById("modal-error-footer").classList.remove("modal-body-day");

        document.getElementById("home-title").style.color = "white";
        //document.getElementById("home-subtitle").style.color = "white";
        document.getElementById("add-nfc_wallet-btn").style.color = "white";
        document.getElementById("add-device_wallet-btn").style.color = "white";
        for(let i = 0; i < document.getElementsByClassName("home-text").length; i++){
            document.getElementsByClassName("home-text")[i].style.color = "white"
        }

        document.getElementById("settings-system-setting-container").style.color = "white";
        document.getElementById("settings-wallet-setting-title").style.color = "white";
        document.getElementById("settings-label-show-tutorial").style.color = "white";
        document.getElementById("settings-label-edit-wallet").style.color = "white";
        document.getElementById("settings-label-backup-wallet").style.color = "white";
        document.getElementById("setting-label-update").style.color = "white";
        document.getElementById("transactions-label-address").style.color = "white";
        document.getElementById("loading-message").style.color = "white";
        document.getElementById("loading-modal").style.backgroundColor = "rgba(0,0,0,0.8)";
        document.getElementById("loading-message").style.backgroundColor = "rgba(0,0,0,0.8)";

        document.getElementById("transactions").classList.remove("transaction-page-day");
        document.getElementById("transactions").classList.add("transaction-page-night");
        document.getElementById("contacts").classList.remove("contact-page-day");
        document.getElementById("contacts").classList.add("contact-page-night");

    }else if(theme_index == 1){
        document.getElementsByClassName("logo")[0].style.color = "#3E464C";
        document.getElementsByClassName("logo")[0].style.backgroundColor = "#efefef";
        document.getElementsByClassName("side-nav")[0].style.backgroundColor = "#efefef";
        var array_a = document.getElementById("menu").getElementsByTagName("a");
        for (var i=0; i<array_a.length; i++){
            array_a[i].style.color = "#3E464C";
        }
        var array_b = document.getElementById("settings_menu").getElementsByTagName("a");
        for (var i=0; i<array_b.length; i++){
            array_b[i].style.color = "#3E464C";
        }
        var array_a = document.getElementById("menu").getElementsByTagName("i");
        for (var i=0; i<array_a.length; i++){
            array_a[i].style.color = "#3E464C";
        }
        document.getElementsByClassName("bodyClass")[0].style.backgroundImage = "linear-gradient(to bottom, #cecece, #cecece)";
        var portfolio_coins = document.getElementsByClassName("image-container");
        for(var i=0; i<portfolio_coins.length; i++){
            portfolio_coins[i].childNodes[0].childNodes[0].style.color = "black";
            portfolio_coins[i].childNodes[0].style.color = "black";
            portfolio_coins[i].childNodes[1].style.color = "black";
            portfolio_coins[i].childNodes[1].childNodes[0].style.color = "black";
            portfolio_coins[i].childNodes[1].childNodes[1].style.color = "black";
        }

        var wallet_coins = document.getElementsByClassName("wallet-images");
        for(var i=0; i<portfolio_coins.length; i++){
            wallet_coins[i].childNodes[1].style.color = "black";
        }
        document.getElementById("setting-label-language").style.color = "black";
        document.getElementById("setting-label-mode").style.color = "black";
      
        var text = document.getElementById("accordion").getElementsByTagName("p");
        for(var i=0; i<text.length; i++){
            text[i].style.color="black";
        }

        var walletAccordion = document.getElementById("wallet_accordion").getElementsByClassName("p");
        for (let i = 0; i < walletAccordion.length; i++) {
            walletAccordion.style.color = "black";
        }

        var cards = document.getElementsByClassName("card");
        for (let i = 0; i < cards.length; i++) {
            cards[i].childNodes[1].style.backgroundColor = "#efefef";
        }

        var div = document.getElementById("accordion").getElementsByClassName("card");
        for(var i=0; i<div.length; i++){
            div[i].style.borderColor = "#80b3ff";
            div[i].classList.remove("theme");
            div[i].classList.add("newTheme");
        }
        
        document.getElementById("mailSender").classList.remove("mail");
        document.getElementById("mailSender").classList.add("newMailSender");

        var body = document.getElementsByClassName("card-body");
        for(var i=0; i<body.length; i++){
            body[i].style.backgroundColor = "#efefef";
        }

        var h6 = document.getElementsByTagName("h6");
        for(var i=0; i<h6.length; i++){
            h6[i].style.color = "black";
            h6[i].style.textDecoration = "underline";
        }

        var walletCards = document.getElementById("wallet-container").getElementsByClassName("wallet-images");
        for(var i=0; i<walletCards.length; i++){
            walletCards[i].classList.remove("theme_class");
            walletCards[i].classList.add("newWalletCards");
        }

        var portfolioCards = document.getElementsByClassName("image-container");
        for(var i=0; i<portfolioCards.length; i++){
            portfolioCards[i].classList.remove("theme_class2");
            portfolioCards[i].classList.add("newPortfolioCards");
        }
        
        var walletScroll = document.getElementById("wallet-container");
        walletScroll.classList.remove("walletScroll");
        walletScroll.classList.add("newWalletScroll");

        var helpScroll = document.getElementById("accordion");
        helpScroll.classList.remove("helpNow");
        helpScroll.classList.add("newHelp");

        var portfolioScroll = document.getElementById("cryptoContent");
        portfolioScroll.classList.remove("portfolioScroll");
        portfolioScroll.classList.add("newPortfolioScroll");

        var walletTable = document.getElementById("wallet-modal-contact-table");
        walletTable.classList.remove("table-night-style");
        walletTable.classList.add("table-day-style");

        var contactTable = document.getElementById("contact-table");
        contactTable.classList.remove("table-night-style");
        contactTable.classList.add("table-day-style");

        var contactTableDiv = document.getElementById("contact-table-div");
        contactTableDiv.classList.add("contact-table-div-day");
        contactTableDiv.classList.remove("contact-table-div-night");

        document.getElementById("contact-div-table-scroll").classList.remove("contact-div-table-scroll-night");
        document.getElementById("contact-div-table-scroll").classList.add("contact-div-table-scroll-day");

        document.getElementById("client_wallets_device").classList.remove("client_wallets_style_night");
        document.getElementById("client_wallets_device").classList.add("client_wallets_style_day");

        document.getElementById("client_wallets_nfc").classList.remove("client_wallets_style_night");
        document.getElementById("client_wallets_nfc").classList.add("client_wallets_style_day");

        var tables = document.querySelectorAll('#wallet-container table[name="wallet-address-types"]');
        for(var i=0; i<tables.length; i++){
          tables[i].classList.remove('table-night-style');
          tables[i].classList.add('table-day-style');
        }

        var transactionTable = document.getElementById("transaction-table");
        transactionTable.classList.add("table-day-style");
        transactionTable.classList.remove("table-night-style");

        document.getElementById("wallet-modal-contacts-search-name-label").style.color = "white";
        document.getElementById("wallet-modal-contacts-search-address-label").style.color = "white";
        document.getElementById("wallet-modal-add-contact-address").style.backgroundColor = "rgba(96,96,96,0.2)";
        document.getElementById("wallet-modal-add-contact-address-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("wallet-modal-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("wallet-modal-receive-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("wallet-modal-generate-address-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("advanced-filter-modal-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("QR-modal-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("add-contact-modal-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("edit-contact-modal-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("add-wallet-modal-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("edit-wallet-modal-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("wallet-modal-confirm-fee-content").style.backgroundColor = "rgba(96,96,96,0.9)";
        document.getElementById("wallet-modal-confirm-fee").style.backgroundColor = "rgba(96,96,96, 0.9)";
        document.getElementById("contact-table").classList.remove("table-night");
        document.getElementById("contact-table").classList.add("table-day");
        document.getElementById("contacts-search-name-label").style.color = "black";
        document.getElementById("contacts-search-address-label").style.color = "black";
        document.getElementById("contacts-pagination-info").classList.add("contacts-pagination-info-day");
        document.getElementById("contacts-pagination-info").classList.remove("contacts-pagination-info-night");
        document.getElementById("transaction-pagination-info").classList.add("transaction-pagination-info-day");
        document.getElementById("transaction-pagination-info").classList.remove("transaction-pagination-info-night");
        document.getElementById("wallet-pagination-info").classList.remove("transaction-pagination-info-day");
        document.getElementById("wallet-pagination-info").classList.add("transaction-pagination-info-night");
        document.getElementById("wallet-modal-receive-pagination-info").style.color = "white";
        document.getElementById("transaction-div-table").classList.add("transaction-div-table-day");
        document.getElementById("transaction-div-table").classList.remove("transaction-div-table-night");
        document.getElementById("wallet-modal-receive-table").classList.add("table-day-style");
        document.getElementById("wallet-modal-receive-table").classList.remove("table-night-style");
        document.getElementById("mailSender").style.backgroundColor = "#d3d3d3";
        document.getElementById("mailSender").style.color = "black";
        document.getElementById("collapseOne").style.backgroundColor = "white"; 
        
        document.getElementById("add-contact-modal-alert-body").classList.remove("modal-body-night");
        document.getElementById("add-contact-modal-alert-body").classList.add("modal-body-day");
        document.getElementById("add-contact-modal-alert-footer").classList.remove("modal-body-night");
        document.getElementById("add-contact-modal-alert-footer").classList.add("modal-body-day");

        document.getElementById("delete-contact-modal-alert-body").classList.remove("modal-body-night");
        document.getElementById("delete-contact-modal-alert-body").classList.add("modal-body-day");
        document.getElementById("delete-contact-modal-alert-footer").classList.remove("modal-footer-night");
        document.getElementById("delete-contact-modal-alert-footer").classList.add("modal-footer-day");

        document.getElementById("modal-warning-body").classList.remove("modal-body-night");
        document.getElementById("modal-warning-body").classList.add("modal-body-day");
        document.getElementById("modal-warning-footer").classList.remove("modal-body-night");
        document.getElementById("modal-warning-footer").classList.add("modal-body-day");

        document.getElementById("modal-error-body").classList.remove("modal-body-night");
        document.getElementById("modal-error-body").classList.add("modal-body-day");
        document.getElementById("modal-error-footer").classList.remove("modal-body-night");
        document.getElementById("modal-error-footer").classList.add("modal-body-day");

        //document.getElementById("home-subtitle").style.color = "black";
        document.getElementById("home-title").style.color = "black";
        document.getElementById("add-nfc_wallet-btn").style.color = "black";
        document.getElementById("add-device_wallet-btn").style.color = "black";
        for(let i = 0; i < document.getElementsByClassName("home-text").length; i++){
            document.getElementsByClassName("home-text")[i].style.color = "black"
        }
        document.getElementById("settings-system-setting-container").style.color = "black";
        document.getElementById("settings-wallet-setting-title").style.color = "black";
        document.getElementById("settings-label-show-tutorial").style.color = "black";
        document.getElementById("settings-label-edit-wallet").style.color = "black";
        document.getElementById("settings-label-backup-wallet").style.color = "black";
        document.getElementById("setting-label-update").style.color = "black";
        document.getElementById("transactions-label-address").style.color = "black";
        document.getElementById("loading-message").style.backgroundColor = "black";
        document.getElementById("loading-modal").style.backgroundColor = "rgba(96,96,96,0.8)";
        document.getElementById("loading-message").style.backgroundColor = "rgba(96,96,96,0.8)";

        document.getElementById("transactions").classList.add("transaction-page-day");
        document.getElementById("transactions").classList.remove("transaction-page-night");
        document.getElementById("contacts").classList.remove("contact-page-day");
        document.getElementById("contacts").classList.add("contact-page-night");
    }
    let db = new sqlite3.Database('./cypher.db');
    // edit settings
    db.run(`UPDATE settings SET theme = ?`, [theme_index], function(err) {
        if (err) {
            return console.log(err.message);
        }
    });
    db.close();
}

/******************************************************************************************************
*                                          Initial load                                               *
******************************************************************************************************/
var default_language, pagination_endpoints = {current_page: 0, start_index: 0, end_index: 6, page_number: 0}, contact_id, row_index, coin_array = [];
var single_coin_selected, selected_wallet;

var db = new sqlite3.Database('./cypher.db');
const tooltip = require('electron-tooltip');

window.onload = function(){
    $.ajax({
        type: "GET",
        url: "language.json",
        data : "",
        dataType: "json",
        contentType : 'application/json',
        success: function (response) {
            try{
                let db = new sqlite3.Database('./cypher.db');
                db.all(`SELECT * FROM settings;`, [], function(err, settings) {
                    if (err) {
                        return console.log(err.message);
                    }
                    document.getElementById("select-language").selectedIndex = settings[0].language;
                    document.getElementById("select-mode").selectedIndex = settings[0].theme;
                    document.getElementsByTagName("body")[0].classList.remove("hide");
                    default_language = response[settings[0].language];
                    changeLabels();
                    changeMode();
                    loadCoins();
                    loadWallets();
                    initTutorial();
                });
                db.close();
            }catch(Exception){
            }
        },
        error: function (jqXHR, exception) { CRDates
            alert("Error");
        }
    });
}

function transactions(){
    const transaction = new Transactions();
    transaction.main().then( () => {
        transaction.disconnectRippleServer(); 
        setTimeout(() => {
            transactions();
        }, 1000 * 60 * 2);                 
    });
}

async function loadWallets(){
    var color;
    var wallet_nodes_device = document.getElementById("client_wallets_device");
    while(wallet_nodes_device.firstChild){
        wallet_nodes_device.removeChild(wallet_nodes_device.firstChild);
    }
    var wallet_nodes_nfc = document.getElementById("client_wallets_nfc");
    while(wallet_nodes_nfc.firstChild){
        wallet_nodes_nfc.removeChild(wallet_nodes_nfc.firstChild);
    }

    if(document.getElementById("select-mode").selectedIndex == 0){
        color = "white";
    }else if(document.getElementById("select-mode").selectedIndex == 1){
        color = "black";
    }
    $('.container-fix').click(function(){
        $("#pages .page:not('.hide')").stop().fadeOut('fast', function() {
            $('li.active').removeClass('active');
            $('#portfolio-side_nav').addClass('active');
            // adds .hide to the element that was showing after it has faded out
            $(this).addClass('hide');
            // remove hidden class from element with the same data attribute as the anchor tag
            $('#pages .page[data-page="portfolio"]').fadeIn('slow').removeClass('hide');
        });
    });
    
    document.getElementById("home-add-nfc_wallet-btn").addEventListener("click", event => {
        document.getElementById("add-wallet-modal-name-input").value = "";
        document.getElementById("add-wallet-modal-description-textarea").value = "";
    });
    document.getElementById("home-add-device_wallet-btn").addEventListener("click", event => {
        document.getElementById("add-wallet-modal-name-input").value = "";
        document.getElementById("add-wallet-modal-description-textarea").value = "";
    });
    return new Promise( (resolve, reject) => {
        db.all('SELECT * FROM wallet', [], (err, rows) => {
            if(err){
                reject(err);
            }
            for (let i = 0; i < rows.length; i++) {
                var container = document.createElement('div');
                container.style.cssFloat = "left";
                container.style.marginTop = "15px";
                container.classList.add("container-fix");
                container.id = "wallet-container-"+i;
                var title = document.createElement('p');
                title.innerHTML = rows[i].wallet_name;
                title.id = "wallet-title-"+rows[i].wallet_id;
                title.style.color = color;
                title.classList.add("home-text");
                title.classList.add("home-text-title")
                var logo = document.createElement('i');
                logo.classList.add('fa');
                if(rows[i].wallet_type === 1){
                    logo.classList.add('fa-credit-card');
                }else{
                    logo.classList.add('fa-briefcase');
                }
                logo.style.fontSize = "35px";
                logo.style.color = color;
                logo.classList.add("home-text");
                logo.id = "wallet-logo-"+rows[i].wallet_id;
                logo.setAttribute("data-tooltip", rows[i].wallet_description);
                logo.setAttribute("data-tooltip-position", "bottom");
                var div = document.createElement("div");
                div.appendChild(title);
                container.appendChild(title);
                container.appendChild(logo);
                container.style.padding = "10px";
                container.style.textAlign = "center";
                container.classList.add("wallet_cards_night");
                container.classList.add("col-4");
                container.setAttribute("onclick", "loadPortfolioBalance(" + rows[i].wallet_id + ")");
                switch(rows[i].wallet_type){
                    case 1:
                        document.getElementById("client_wallets_nfc").appendChild(container);
                        break;
                    case 2:
                        document.getElementById("client_wallets_device").appendChild(container);
                        break;    
                }
            }
            resolve(rows);
            tooltip({
                width: 200,
                style:{
                    backgroundColor: '#1485e2',
                    width: '200px',
                    textAlign: 'justify'
                }
            });
            homeTutorial();
        });
    });
}

function editWallet(){
    db.all('SELECT * FROM wallet where wallet_id = ?', [selected_wallet], (err, rows) => {
        document.getElementById("edit-wallet-modal-name-input").value = rows[0].wallet_name;
        document.getElementById("edit-wallet-modal-description-textarea").value = rows[0].wallet_description;
        $('#edit-wallet-modal').modal('show');
        document.getElementById("edit-wallet-modal-add-btn").addEventListener("click", function(event){
            db.run("UPDATE wallet SET wallet_name = ?, wallet_description = ? WHERE wallet_id = ?", [document.getElementById("edit-wallet-modal-name-input").value, document.getElementById("edit-wallet-modal-description-textarea").value, selected_wallet], (err, rows) => {
                if(err){
                    throw err
                }
                document.getElementById("wallet-title-"+selected_wallet).innerHTML = document.getElementById("edit-wallet-modal-name-input").value;
                document.getElementById("wallet-logo-"+selected_wallet).setAttribute("data-tooltip", document.getElementById("edit-wallet-modal-description-textarea").value);
                $('#edit-wallet-modal').modal('hide');
            });
        });
    });
}

function backupWallet(){
    $("#loading-modal").modal("show");
    executePythonScript(["seriallisten.py", "7", selected_wallet]).then((result)=>{
        $("#loading-modal").modal("hide");    
        if(result == "ERROR"){
            document.getElementById("modal-error-body-info").innerHTML = default_language.nfc_error;
            $("#modal-message-error").modal("show");
        }else{
            document.getElementById("add-header-info").innerHTML = default_language.success;
            document.getElementById("contact-modal-added-message").innerHTML = default_language.nfc_success;
            $("#contact-modal-added").modal("show");
        }
    });
}


function createWallet(wallet_type){
    $('#add-wallet-modal').modal('show');
    console.log(wallet_type);
    document.getElementById("add-wallet-modal-add-btn").addEventListener("click", function(event) {
        $('#loading-modal').modal('show');
        executePythonScript(["seriallisten.py", "4", '"'+document.getElementById("add-wallet-modal-name-input").value+'"']).then((wallet_id)=>{
            db.prepare("INSERT INTO wallet(wallet_id, wallet_name, wallet_description, wallet_type) VALUES (?, ?, ?, ?)").run([wallet_id, document.getElementById("add-wallet-modal-name-input").value, document.getElementById("add-wallet-modal-description-textarea").value, wallet_type], (err, rows) => {
                var container = document.createElement('div');
                container.style.cssFloat = "left";
                container.style.marginTop = "15px";
                var title = document.createElement('p');
                title.innerHTML = document.getElementById("add-wallet-modal-name-input").value;
                title.classList.add("home-text");
                title.classList.add("home-text-title");
                container.appendChild(title);
                var logo = document.createElement('i');
                logo.classList.add('fa');
                if(wallet_type === 1){
                    logo.classList.add('fa-credit-card');
                }else{
                    logo.classList.add('fa-briefcase');
                }
                logo.style.fontSize = "35px";
                logo.style.marginTop = "10px";
                logo.classList.add("home-text");
                logo.setAttribute("data-tooltip", document.getElementById("add-wallet-modal-description-textarea").value);
                container.appendChild(logo);
                container.style.padding = "10px";
                container.style.textAlign = "center";
                container.classList.add("wallet_cards_night");
                container.classList.add("col-4");
                container.setAttribute("onclick", "loadPortfolioBalance(" + wallet_id + ")");
                if(wallet_type === 1){
                    document.getElementById("client_wallets_nfc").appendChild(container);
                }else{
                    document.getElementById("client_wallets_device").appendChild(container);
                }
                $('#add-wallet-modal').modal('hide');
                $('#loading-modal').modal('hide');
            });
        });
    });
}

function changeLabels(){
    //wallets
    document.getElementById("nfc_wallet_text").innerHTML = default_language.nfc_wallet + ": ";
    document.getElementById("device_wallet_text").innerHTML = default_language.device_wallet + ": ";
    //side-nav buttons
    document.getElementById("portfolio-menu-span").innerHTML = default_language.portfolio;
    document.getElementById("wallet-menu-span").innerHTML = default_language.wallet;
    document.getElementById("contacts-menu-span").innerHTML = default_language.contacts;
    document.getElementById("settings-menu-span").innerHTML = default_language.settings;
    document.getElementById("help-menu-span").innerHTML = default_language.help;
    document.getElementById("change-wallet-menu-span").innerHTML = default_language.change_wallet;
    //wallet modal contacts
    document.getElementById("wallet-modal-contacts-search-name-label").innerHTML = default_language.name + ": ";
    document.getElementById("wallet-modal-contacts-search-address-label").innerHTML = default_language.address + ": ";
    document.getElementById("wallet-modal-contacts-search-address-input").placeholder = default_language.search_address;
    document.getElementById("wallet-modal-contacts-search-name-input").placeholder = default_language.search_name;
    document.getElementById("wallet-modal-table-name-header").innerHTML = default_language.name;
    document.getElementById("wallet-modal-table-address-header").innerHTML = default_language.address;
    document.getElementById("wallet-modal-table-coin_type-header").innerHTML = default_language.coin;
    document.getElementById("wallet-modal-table-options-header").innerHTML = default_language.options;
    document.getElementById("wallet-modal-add-contacts-modal-close-btn").innerHTML = default_language.close;
    //document.getElementById("wallet-modal-add-contact-title").innerHTML = default_language.contacts;
    //Settings view
    document.getElementById("setting-label-language").innerHTML = default_language.language + ": ";
    document.getElementById("setting-label-mode").innerHTML = default_language.mode + ": ";
    document.getElementById("settings-system-setting-title").innerHTML = default_language.system_settings;
    document.getElementById("settings-wallet-setting-title").innerHTML = default_language.wallet_settings;
    document.getElementById("settings-label-show-tutorial").innerHTML = default_language.show_tutorial + ": ";
    document.getElementById("settings-label-edit-wallet").innerHTML = default_language.edit_wallet + ": ";
    document.getElementById("settings-label-backup-wallet").innerHTML = default_language.backup_wallet + ": ";
    document.getElementById("setting-update-button").innerHTML = default_language.update;
    document.getElementById("setting-label-update").innerHTML = default_language.update_cypher + ": ";
    //Contacts view
    document.getElementById("contacts-search-name-label").innerHTML = default_language.name + ": ";
    document.getElementById("contacts-search-address-label").innerHTML = default_language.address + ": ";
    document.getElementById("contacts-search-address-input").placeholder = default_language.search_address;
    document.getElementById("contacts-search-name-input").placeholder = default_language.search_name;
    document.getElementById("table-name-header").innerHTML = default_language.name;
    document.getElementById("table-address-header").innerHTML = default_language.address;
    document.getElementById("table-coin_type-header").innerHTML = default_language.coin;
    document.getElementById("table-options-header").innerHTML = default_language.options;
    document.getElementById("table-add-btn").innerHTML = default_language.add;
    //Contacts view (EDIT MODAL)
    document.getElementById("edit-contacts-modal-coin_type-label").innerHTML = default_language.coin_type + ": ";
    document.getElementById("edit-contacts-modal-tittle").innerHTML = default_language.edit_contact + ": ";
    document.getElementById("edit-contacts-modal-name-label").innerHTML = default_language.name + ": ";
    document.getElementById("edit-contacts-modal-address-label").innerHTML = default_language.address + ": ";
    document.getElementById("edit-contacts-modal-edit-btn").innerHTML = default_language.edit;
    document.getElementById("edit-contacts-modal-close-btn").innerHTML = default_language.close;
    //Contacts view (ADD MODAL)
    document.getElementById("add-contacts-modal-name-label").innerHTML = default_language.name + ": ";
    document.getElementById("add-contacts-modal-address-label").innerHTML = default_language.address + ": ";
    document.getElementById("add-contacts-modal-name-input").placeholder = default_language.insert_name;
    document.getElementById("add-contacts-modal-address-input").placeholder = default_language.insert_address;
    document.getElementById("add-contacts-modal-coin_type-label").innerHTML = default_language.coin_type + ": ";
    document.getElementById("add-contacts-modal-tittle").innerHTML = default_language.add_contact;
    document.getElementById("add-contacts-modal-add-btn").innerHTML = default_language.add;
    document.getElementById("add-contacts-modal-close-btn").innerHTML = default_language.close;
    //Paginations
    document.getElementById("contacts-pagination-current-page-number-text").innerHTML = default_language.page;
    document.getElementById("contacts-pagination-last-page-number-text").innerHTML = default_language.of;
    document.getElementById("wallet-pagination-current-page-number-text").innerHTML = default_language.page;
    document.getElementById("wallet-pagination-last-page-number-text").innerHTML = default_language.of;  
    //Help View
    document.getElementById("mailSender").innerHTML = default_language.help_sendEmail;
    document.getElementById("help_q1").innerHTML = default_language.help_q1;
    document.getElementById("help_a1").innerHTML = default_language.help_a1;
    document.getElementById("help_q2").innerHTML = default_language.help_q2;
    document.getElementById("help_a2.1").innerHTML = default_language.help_a2_1;
    document.getElementById("help_a2.2").innerHTML = default_language.help_a2_2;
    document.getElementById("help_a2.3").innerHTML = default_language.help_a2_3;
    document.getElementById("help_a2.4").innerHTML = default_language.help_a2_4;
    document.getElementById("help_a2.5").innerHTML = default_language.help_a2_5;
    document.getElementById("help_a2.6").innerHTML = default_language.help_a2_6;
    document.getElementById("help_a2.7").innerHTML = default_language.help_a2_7;
    document.getElementById("help_q3").innerHTML = default_language.help_q3;
    document.getElementById("help_a3").innerHTML = default_language.help_a3;
    document.getElementById("help_q4").innerHTML = default_language.help_q4;
    document.getElementById("help_a4").innerHTML = default_language.help_a4;
    document.getElementById("help_q5").innerHTML = default_language.help_q5;
    document.getElementById("help_a5").innerHTML = default_language.help_a5;
    document.getElementById("help_q6").innerHTML = default_language.help_q6;
    document.getElementById("help_a6").innerHTML = default_language.help_a6;
    //Transactions 
    document.getElementById("transaction-button-filters").innerHTML = default_language.filters;
    document.getElementById("transactions-menu-span").innerHTML = default_language.transactions;
    document.getElementById("transaction-table-label-address").innerHTML = default_language.from;
    document.getElementById("transactions-label-address").innerHTML = default_language.address;
    document.getElementById("transactions-input-address").placeholder = default_language.search_address;
    document.getElementById("transaction-table-label-type").innerHTML = default_language.type;
    document.getElementById("transaction-table-label-quantity").innerHTML = default_language.amount;
    document.getElementById("transaction-table-label-date").innerHTML = default_language.date;
    document.getElementById("transaction-table-label-coin").innerHTML = default_language.coin;
    document.getElementById("transaction-table-label-hour").innerHTML = default_language.hour;
    document.getElementById("transaction-pagination-current-page-number-text").innerHTML = default_language.page;
    document.getElementById("transaction-pagination-last-page-number-text").innerHTML = default_language.of;
    //advanced filters 
    document.getElementById("advanced-filter-modal-title-text").innerHTML = default_language.advancedFilters;
    document.getElementById("advanced-filter-modal-coinType-label").innerHTML = default_language.coin + ": ";
    document.getElementById("advanced-filter-modal-year-label").innerHTML = default_language.year + ": ";
    document.getElementById("advanced-filter-modal-month-label").innerHTML = default_language.month + ": ";
    document.getElementById("advanced-filter-modal-day-label").innerHTML = default_language.date + ": ";
    document.getElementById("advanced-filter-modal-clear-btn").innerHTML = default_language.clear;
    document.getElementById("advanced-filter-modal-close-btn").innerHTML = default_language.close;
    document.getElementById("select_coin").innerHTML = default_language.selectCoin;
    //Wallet Modal
    document.getElementById("wallet-modal-label-address").innerHTML = default_language.address;
    document.getElementById("wallet-modal-input-address").placeholder = default_language.insert_address;
    document.getElementById("wallet-modal-label-amount").innerHTML = default_language.amount;
    document.getElementById("wallet-modal-label-fee").innerHTML = default_language.fee;
    document.getElementById("wallet-modal-button-receive").innerHTML = default_language.receive;
    document.getElementById("wallet-modal-button-send").innerHTML = default_language.send;
    document.getElementById("wallet-modal-button-close").innerHTML = default_language.close;
    //Wallet Modal Receive
    document.getElementById("wallet-modal-receive-input-search").placeholder = default_language.search_address;
    document.getElementById("wallet-modal-receive-button-generate").innerHTML = default_language.generate_address;
    document.getElementById("wallet-modal-receive-table-head-address").innerHTML = default_language.address;
    document.getElementById("wallet-modal-receive-table-head-description").innerHTML = default_language.description;
    document.getElementById("wallet-modal-receive-table-head-options").innerHTML = default_language.options;
    document.getElementById("wallet-modal-receive-pagination-current-page-text").innerHTML = default_language.page;
    document.getElementById("wallet-modal-receive-pagination-last-page-text").innerHTML = default_language.of;
    document.getElementById("wallet-modal-receive-button-close").innerHTML = default_language.close;
    
    document.getElementById("wallet-modal-generate-address-label-description").innerHTML = default_language.description + ": ";
    document.getElementById("wallet-modal-generate-address-button-generate").innerHTML = default_language.generate;
    document.getElementById("wallet-modal-generate-address-button-close").innerHTML = default_language.close;
    //wallets
    document.getElementById("add-wallet-modal-name-label").innerHTML = default_language.name + ": ";
    document.getElementById("add-wallet-modal-description-label").innerHTML = default_language.description + ": ";
    document.getElementById("add-wallet-modal-name-input").placeholder = default_language.wallet_name_placeholder;
    document.getElementById("add-wallet-modal-description-textarea").placeholder = default_language.wallet_description_placeholder;
    document.getElementById("add-wallet-modal-add-btn").innerHTML = default_language.add;
    document.getElementById("add-wallet-modal-close-btn").innerHTML = default_language.close;
    document.getElementById("add-wallet-modal-title").innerHTML = default_language.add_wallet;
    document.getElementById("edit-wallet-modal-title").innerHTML = default_language.edit_wallet;
    document.getElementById("edit-wallet-modal-name-label").innerHTML = default_language.name;
    document.getElementById("edit-wallet-modal-description-label").innerHTML = default_language.description;
    document.getElementById("edit-wallet-modal-add-btn").innerHTML = default_language.edit;
    document.getElementById("edit-wallet-modal-close-btn").innerHTML = default_language.close;
    document.getElementById("home-title").innerHTML = default_language.welcome_cypher;
    //document.getElementById("home-subtitle").innerHTML = default_language.select_wallet + ": ";
    //wallet modal confirm fee
    document.getElementById("loading-message").innerHTML = default_language.please_wait;
    document.getElementById("wallet-modal-confirm-subtotal-text").innerHTML = default_language.confirm_fee_text1;
    document.getElementById("wallet-modal-confirm-total-text").innerHTML = default_language.confirm_fee_text2;
    document.getElementById("wallet-modal-confirm-btn-confirm").innerHTML = default_language.send;
    document.getElementById("wallet-modal-confirm-btn-close").innerHTML = default_language.close;
}

function generateAddress(){
    $('#loading-modal').modal('show');
    executePythonScript(["seriallisten.py", "2", single_coin_selected.replace(" ", ""), selected_wallet]).then((address)=>{
        console.log(address);
        if(address != null){
            var coin_id = coin_array.indexOf(single_coin_selected) + 1;
            db.run("INSERT INTO addresses (address, coin_id, wallet_id, description) VALUES (?, ?, ?, ?)", [address, coin_id, selected_wallet, document.getElementById("wallet-modal-generate-address-textarea-description").value], ( err ) => {
                if(err){
                    throw err;
                }
                $('#wallet-modal-receive-table').append(
                    '<tr id="wallet-modal-receive-first_row">'+
                        '<td id="wallet-modal-receive-first-address" style="width: 40%">'+ address +'</td>'+
                        '<td style="width: 40%">'+ document.getElementById("wallet-modal-generate-address-textarea-description").value +'</td>'+
                        '<td style="width: 20%;">'+
                            '<button class="btn btn-primary" onclick="loadQR(this)"><span style="color: white;">'+
                                '<i class="fa fa-qrcode"></i>'+
                            '</span></button>'+
                        '</td>'+
                    '</tr>'
                );
                $('#wallet-modal-receive-table tbody tr').css('opacity','0.0').hide().slice(0, (Math.ceil((screen.height / 100)/2) - 1)).css('display','table-row').animate({opacity:1}, 300);
                pagination('wallet-modal-receive-pagination-list', 'wallet-modal-receive-table', 'wallet-modal-receive-div-table-scroll', 'wallet-modal-receive-pagination-lquo-button', 'wallet-modal-receive-pagination-rquo-button', 'wallet-modal-receive-pagination-current-page-number', 'wallet-modal-receive-pagination-last-page-number', (Math.ceil((screen.height / 100)/2) - 1), 'pagination-night-theme', 'pagination-day-theme', 'wallet-modal-receive-pagination-info');

                document.getElementById("wallet-modal-generate-address-textarea-description").value = "";
                $('#loading-modal').modal('hide');
            });
        }
    });
}

function validateAmount(event){
    if(event.keyCode == 189){
        document.getElementById("wallet-modal-input-amount").value = 0;
    }
}

function calculateAmount(event){
    if(parseFloat(document.getElementById("wallet-modal-input-amount").value) < 0 ){
        document.getElementById("wallet-modal-input-amount").value = 0;
    }
    switch(coin_array.indexOf(single_coin_selected) + 1){
        case 1:
            document.getElementById("wallet-modal-label-amount-total").innerHTML = (Math.floor(document.getElementById("wallet-modal-input-amount").value * coins[0].coin_price * 10000)/10000) + " USD";
            break;
        case 2:
            document.getElementById("wallet-modal-label-amount-total").innerHTML = (Math.floor(document.getElementById("wallet-modal-input-amount").value * coins[1].coin_price * 10000)/10000) + " USD";
            break;
        case 3:
            document.getElementById("wallet-modal-label-amount-total").innerHTML = (Math.floor(document.getElementById("wallet-modal-input-amount").value * coins[2].coin_price * 10000)/10000) + " USD";
            break;    
        case 4:
            document.getElementById("wallet-modal-label-amount-total").innerHTML = (Math.floor(document.getElementById("wallet-modal-input-amount").value * coins[3].coin_price * 10000)/10000) + " USD";
            break;    
        case 5:
            document.getElementById("wallet-modal-label-amount-total").innerHTML = (Math.floor(document.getElementById("wallet-modal-input-amount").value * coins[4].coin_price * 10000)/10000) + " USD";
            break;    
    }
}

function prepareTx(coin){
    $('#wallet-modal-content input').each( (index, input) => {
        if(input.value == ""){
            input.style.boxShadow = "red 0px 0px 2px 2px";
        }else{
            input.style.boxShadow = "none";
        }
    });
    
    if(document.getElementById("wallet-modal-input-address").value != "" && document.getElementById("wallet-modal-input-amount").value != ""){
        if(coin_array.indexOf(single_coin_selected) + 1 != 5){
            var max, coin_fee = 0;
            document.getElementById("wallet-modal-confirm-fee-title").innerHTML = coin.label + " ("+ coin.prefix +")" + " " + default_language.select_fee;
            document.getElementById("wallet-modal-confirm-fee-image").setAttribute("src","images/"+ coin.image +"");
            document.getElementById("wallet-modal-confirm-subtotal").innerHTML = document.getElementById("wallet-modal-input-amount").value + " " + coin.prefix + " ("+ document.getElementById("wallet-modal-label-amount-total").innerHTML.split(" ")[0] + " USD " +") ";
            document.getElementById("wallet-modal-confirm-total").innerHTML = document.getElementById("wallet-modal-label-amount-total").innerHTML.split(" ")[0] + " USD";
            document.getElementById("wallet-modal-confirm-input-fee").value = "0.00"; 
        
            switch(coin_array.indexOf(single_coin_selected) + 1){
                case 1:
                    max = Math.ceil(parseFloat(document.getElementById("wallet-modal-label-amount-total").innerHTML.split(" ")[0])*0.025) // parseFloat(coins[0].coin_price);
                    sliderSetup(max);
                    break;
                case 2:
                    max = Math.round(parseFloat(document.getElementById("wallet-modal-label-amount-total").innerHTML.split(" ")[0])*0.025) // parseFloat(coins[1].coin_price);
                    sliderSetup(max);
                    break;  
                case 3:
                    max = Math.round(parseFloat(document.getElementById("wallet-modal-label-amount-total").innerHTML.split(" ")[0])*0.025) // parseFloat(coins[2].coin_price);
                    sliderSetup(max);
                    break;
                case 4:
                    max = Math.round(parseFloat(document.getElementById("wallet-modal-label-amount-total").innerHTML.split(" ")[0])*0.025) // parseFloat(coins[2].coin_price);
                    sliderSetup(max);
                    break;          
            }
        
            document.getElementById("wallet-modal-range-fee").oninput = function(event){
                document.getElementById("wallet-modal-confirm-input-fee").value = this.value;
            }
        
            document.getElementById("wallet-modal-range-fee").onchange = function(event){
                console.log("USD: " + this.value);
                console.log(coin.prefix + ": " + (parseFloat(this.value)/coins[coin_array.indexOf(single_coin_selected)].coin_price));
                coin_fee = parseFloat(this.value) / coins[coin_array.indexOf(single_coin_selected)].coin_price;
                document.getElementById("wallet-modal-confirm-total").innerHTML = (parseFloat(document.getElementById("wallet-modal-label-amount-total").innerHTML.split(" ")[0]) + parseFloat(this.value)) + " USD";
            }
        
            document.getElementById("wallet-modal-confirm-input-fee").onchange = function(){
                document.getElementById("wallet-modal-range-fee").value = this.value;
            }
        
            $('#wallet-modal-confirm-fee').modal('show');
            document.getElementById("wallet-modal-confirm-btn-confirm").addEventListener("click", function(event){    
                $('.modal').modal('hide');
                $('#loading-modal').modal('show').on('shown.bs.modal', () => {
                    sendMoney().then( result => {
                        $('#loading-modal').modal('hide');
                        console.log(result);
                        if(result == "NO DEVICE"){
                            document.getElementById("modal-error-body-info").innerHTML = default_language.connect_device;
                            $("#modal-message-error").modal('show');
                        }else{
                            if(!result.networkErr && !result.balanceErr){
                                $('#contact-modal-added').modal('show');
                                document.getElementById("contact-modal-added-message").innerHTML = default_language.transaction_completed;
                                setTimeout(() => {
                                    $('#contact-modal-added').modal('hide');
                                }, 2000);
                            }else if(result.networkErr){
                                $('#modal-message-error').modal('show');
                                document.getElementById("modal-error-body-info").innerHTML = default_language.transaction_fail;
                                setTimeout(() => {
                                    $('#modal-message-error').modal('hide');
                                }, 2000);
                            }else if(result.balanceErr){
                                $('#modal-message-error').modal('show');
                                document.getElementById("modal-error-body-info").innerHTML = default_language.no_balance;
                                setTimeout(() => {
                                    $('#modal-message-error').modal('hide');
                                }, 2000);
                            }
                        }
                    }).catch(err => console.error(err));
                });
            });
        }else{
            sendMoney();
        }
    }
}

function sliderSetup(max){
    console.log("MAX: " + max);
    slider = document.getElementById("wallet-modal-range-fee");
    if(max == 0){
        max = 1;
    }

    slider.value = 0;
    slider.min = 0;
    slider.max = max;
    slider.step = 0.01;
}


async function sendMoney(){
    var wallet_id = 1, result = { networkErr: false, balanceErr: false };
    const request = require('request');
    return new Promise ( (resolve, reject) => {
        switch(single_coin_selected){
            case "Bitcoin":
                db.each("SELECT SUM(balance) as balance FROM addresses WHERE wallet_id = ? AND coin_id = ? GROUP BY coin_id", [wallet_id, 1],async (err, balance) => {
                    if(balance.balance >= (parseFloat(document.getElementById("wallet-modal-input-amount").value) + (parseFloat(document.getElementById("wallet-modal-range-fee").value/100)))){
                        result.balanceErr = false;
                        executePythonScript(["seriallisten.py", "5", single_coin_selected.replace(" ", ""), document.getElementById("wallet-modal-input-address").value, document.getElementById("wallet-modal-input-amount").value, document.getElementById("wallet-modal-range-fee").value/100]).then(async (hashes)=>{
                            if(!hashes){
                                resolve("NO DEVICE");
                                return "NO DEVICE";
                            }else{
                                var hash_tmp = [];
                                if(hashes.includes('"')){
                                    hash_tmp.push.apply(hash_tmp, JSON.parse(hashes));
                                }else{
                                    hash_tmp.push(hashes);
                                }
                                console.log(hash_tmp);
                                for(const hash of hash_tmp){
                                    var req = await request.post(
                                        "https://api.blockcypher.com/v1/btc/test3/txs/push",
                                        { 
                                            json: { tx: hash },
                                            rejectUnauthorized: false
                                        },
                                        function (error, response, body) {
                                            if(error){
                                                result.networkErr = true;
                                            }
                                        }
                                    );
                                }
                            }
                        });
                    }else{
                        result.balanceErr = true;
                    }
                    resolve(result);
                });
                break;
            case "Bitcoin Cash":
                db.each("SELECT SUM(balance) as balance FROM addresses WHERE wallet_id = ? AND coin_id = ? GROUP BY coin_id", [wallet_id, 2],async (err, balance) => {
                    if(balance.balance >= (parseFloat(document.getElementById("wallet-modal-input-amount").value) + (parseFloat(document.getElementById("wallet-modal-range-fee").value/100)))){
                        result.balanceErr = false;
                        executePythonScript(["seriallisten.py", "5", single_coin_selected.replace(" ", ""), document.getElementById("wallet-modal-input-address").value, document.getElementById("wallet-modal-input-amount").value, document.getElementById("wallet-modal-range-fee").value/100]).then(async (hashes)=>{
                            if(!hashes){
                                resolve("NO DEVICE");
                                return "NO DEVICE";
                            }else{
                                var hash_tmp = [];
                                if(hashes.includes('"')){
                                    hash_tmp.push.apply(hash_tmp, JSON.parse(hashes));
                                }else{
                                    hash_tmp.push(hashes);
                                }
                                console.log(hash_tmp);
                                for(const hash of hash_tmp){
                                    /*request.post(
                                        "https://test-bch-insight.bitpay.com/api/tx/send",
                                        { json: { rawtx: hash }, rejectUnauthorized: false },
                                        function (error, response, body) {
                                            if(error){
                                                result.networkErr = true;
                                            }                        
                                        }
                                    );*/
                                }
                            }
                        });
                    }else{
                        result.balanceErr = true;
                    }
                    resolve(result);
                });
                break;
            case "Litecoin":
                db.each("SELECT SUM(balance) as balance FROM addresses WHERE wallet_id = ? AND coin_id = ? GROUP BY coin_id", [wallet_id, 3],async (err, balance) => {
                    if(balance.balance >= (parseFloat(document.getElementById("wallet-modal-input-amount").value) + (parseFloat(document.getElementById("wallet-modal-range-fee").value/100)))){
                        result.balanceErr = false;
                        executePythonScript(["seriallisten.py", "5", single_coin_selected.replace(" ", ""), document.getElementById("wallet-modal-input-address").value, document.getElementById("wallet-modal-input-amount").value, document.getElementById("wallet-modal-range-fee").value/100]).then(async (hashes)=>{
                            if(!hashes){
                                resolve("NO DEVICE");
                                return "NO DEVICE";
                            }else{
                                var hash_tmp = [];
                                if(hashes.includes('"')){
                                    hash_tmp.push.apply(hash_tmp, JSON.parse(hashes));
                                }else{
                                    hash_tmp.push(hashes);
                                }
                                console.log(hash_tmp);
                                for(const hash of hash_tmp){
                                    request({
                                        url: "https://chain.so/api/v2/send_tx/LTCTEST",
                                        method: 'POST',
                                        json: {tx_hex: hash},
                                        rejectUnauthorized: false
                                    }, function (error, response, body) {
                                        if(error){
                                            result.networkErr = true;
                                        }
                                    });
                                }
                            }
                        });
                    }else{
                        result.balanceErr = true;
                    }
                    resolve(result);
                });
                break;
            case "Ethereum":
                db.each("SELECT SUM(balance) as balance FROM addresses WHERE wallet_id = ? AND coin_id = ? GROUP BY coin_id", [wallet_id, 4],async (err, balance) => {
                    if(balance.balance >= (parseFloat(document.getElementById("wallet-modal-input-amount").value) + (parseFloat(document.getElementById("wallet-modal-range-fee").value/100)))){
                        result.balanceErr = false;
                        executePythonScript(["seriallisten.py", "5", single_coin_selected.replace(" ", ""), document.getElementById("wallet-modal-input-address").value, document.getElementById("wallet-modal-input-amount").value, document.getElementById("wallet-modal-range-fee").value/100]).then(async (hashes)=>{
                            if(!hashes){
                                resolve("NO DEVICE");
                                return "NO DEVICE";
                            }else{
                                var hash_tmp = [];
                                if(hashes.includes('"')){
                                    hash_tmp.push.apply(hash_tmp, JSON.parse(hashes));
                                }else{
                                    hash_tmp.push(hashes);
                                }
                                console.log(hash_tmp);
                                for(const hash of hash_tmp){
                                    /*const apiToken = "JZDEW855XEZFDZ3H3QWXXDG8F9A3YGTVMT";
                                    request.get("https://api-ropsten.etherscan.io/api?module=proxy&action=eth_sendRawTransaction&hex=0x" + hash + "&apikey=" + apiToken, { json: true }, (err, req, body) => {
                                        if(err){
                                            result.networkErr = true;
                                        }
                                    });*/
                                }
                            }
                        });
                    }else{
                        result.balanceErr = true;
                    }
                    resolve(result);
                });
                break;
            case "Ripple":
                db.each("SELECT SUM(balance) as balance FROM addresses WHERE wallet_id = ? AND coin_id = ? GROUP BY coin_id", [wallet_id, 5],async (err, balance) => {
                    if(balance.balance >= (parseFloat(document.getElementById("wallet-modal-input-amount").value) + (parseFloat(document.getElementById("wallet-modal-range-fee").value/100)))){
                        result.balanceErr = false;
                        executePythonScript(["seriallisten.py", "5", single_coin_selected.replace(" ", ""), document.getElementById("wallet-modal-input-address").value, document.getElementById("wallet-modal-input-amount").value, document.getElementById("wallet-modal-range-fee").value/100]).then(async (hashes)=>{
                            if(!hashes){
                                resolve("NO DEVICE");
                                return "NO DEVICE";
                            }else{
                                var hash_tmp = [];
                                if(hashes.includes('"')){
                                    hash_tmp.push.apply(hash_tmp, JSON.parse(hashes));
                                }else{
                                    hash_tmp.push(hashes);
                                }
                                console.log(hash_tmp);
                                for(const hash of hash_tmp){
                                    const RippleAPI = require('ripple-lib').RippleAPI;
                                    var api = new RippleAPI({server: 'wss://s.altnet.rippletest.net:51233'});
                                    api.connect().then(() => {
                                        // Comienza el método hacer el pago
                                        const signedTransaction = hash;
                                        return api.submit(signedTransaction).then(result => {
                                            
                                        }).catch(err=>{
                                            result.networkErr = true;
                                        });
                                    }).then(() => {
                                        return api.disconnect();
                                    }).catch((error)=>{
                                        console.log("Error disconnecting");
                                    });
                                }
                            }
                        });
                    }else{
                        result.balanceErr = true;
                    }
                    resolve(result);
                });
                break;
        }
    });
}

async function executePythonScript(argv){
    const spawn = require("child_process").spawn;
    const pythonProcess = spawn('python', argv);
    return new Promise(function(resolve, reject) {
        pythonProcess.stderr.on('data', function (data) {
            console.log(data.toString());
            if(data.toString().includes("could not open port")){
                resolve(false);
                reject(data);
            }
        });
        pythonProcess.stdout.on('data', (data) => {
            console.log(data.toString());
            resolve(JSON.parse(data));
        });
    });
}

/******************************************************************************************************
*                                         Main function                                               *
******************************************************************************************************/

function main(){
    synchronizeDatabase();
    total_array = [{coin: "bitcoin",
                    total: 0.00,
                    coin_price: 0.00,
                    percentage: 0},
                    {coin: "bitcoin_cash",
                    total: 0.00,
                    coin_price: 0.00,
                    percentage: 0},
                    {coin: "litecoin",
                    total: 0.00,
                    coin_price: 0.00,
                    percentage: 0},
                    {coin: "ethereum",
                    total: 0.00,
                    coin_price: 0.00,
                    percentage: 0},
                    {coin: "ripple",
                    total: 0.00,
                    coin_price: 0.00,
                    percentage: 0}];
	const bitcoin = new Cryptocurrency("Bitcoin", "bitcoin", "BTC", "bitcoin.png", 0);
	const bitcoin_cash = new Cryptocurrency("Bitcoin Cash", "bitcoin_cash", "BCH", "bitcoin_cash.png", 0);
	const litecoin = new Cryptocurrency("Litecoin", "litecoin", "LTC", "litecoin.png", 0);
	const ethereum = new Cryptocurrency("Ethereum", "ethereum", "ETH", "ethereum.png", 0);
	const ripple = new Cryptocurrency("Ripple", "ripple", "XRP", "ripple.png", 0);
	bitcoin.price();
	bitcoin_cash.price();
	litecoin.price();
	ethereum.price();
    ripple.price();
	bitcoin.addHTML();
	bitcoin_cash.addHTML();
	litecoin.addHTML();
	ethereum.addHTML();
    ripple.addHTML();
    db.all('select coin_name from coin', [], (err, rows) => {
        var coins = [];
        for(let i = 0; i < rows.length; i++){
            coins.push(rows[i].coin_name.toLowerCase());
        }
        for(let i = 0; i < document.getElementsByClassName("wallet-images").length; i++){
            document.getElementsByClassName("wallet-images")[i].addEventListener("click", ( event ) => {
                for(let i = 0; i < coins.length; i++){
                    document.querySelectorAll("#"+ coins[i] +"-modal-content .input input").forEach( (input) => {
                        input.value = "";
                        input.style.boxShadow = "none";
                        document.getElementById("wallet-send-btn-"+coins[i]).removeAttribute("data-dismiss","modal");
                    });
                }
            });
        }
    });
}

function openWalletModal(coin){
    document.getElementById("wallet-modal-input-amount").addEventListener("keypress", function(event){
        if(event.keyCode == 45){
            event.preventDefault();
        }
    });
    document.getElementById("wallet-modal-input-address").addEventListener("keypress", function(event){
        if(event.keyCode == 32){
            event.preventDefault();
        }
    });
    $('#wallet-modal-content input').each( (index, input) => {
        input.style.boxShadow = "none";
    });
    walletModalTutorial();
    single_coin_selected = coin.label;
    document.getElementById("wallet-modal-image").setAttribute("src","images/"+ coin.image +"");
    document.getElementById("wallet-modal-title").innerHTML = coin.label + " ("+ coin.prefix +")" + " " + default_language.send;
    document.getElementById("wallet-modal-input-amount").placeholder = "0.00 "+ coin.prefix;
    document.getElementById("wallet-modal-button-receive").setAttribute("onclick", "loadAddresses({label: \""+ coin.label +"\", image: \""+ coin.image +"\", prefix: \""+ coin.prefix +"\"})");
    document.getElementById("wallet-modal-button-send").setAttribute("onclick", "prepareTx({label: \""+ coin.label +"\", image: \""+ coin.image +"\", prefix: \""+ coin.prefix +"\"})");

    document.getElementById("wallet-modal-receive-button-generate").setAttribute("onclick", "openWalletModalReceive({label: \""+ coin.label +"\", prefix: \""+ coin.prefix +"\", image: \""+ coin.image +"\"})");
    document.getElementById("wallet-modal-receive-button-selectcontact").setAttribute("onclick", "loadContactsModal({label: \""+ coin.label +"\", prefix: \""+ coin.prefix +"\", image: \""+ coin.image +"\"})");
    document.getElementById("wallet-modal-input-address").value = "";
    document.getElementById("wallet-modal-input-amount").value = "";
    document.getElementById("wallet-modal-label-amount-total").innerHTML = "0 USD";

    $('#wallet-modal').modal('show');
}

function openWalletModalReceive(coin){
    generateAddressTutorial();
    document.getElementById("wallet-modal-generate-address-image").setAttribute("src", "images/"+ coin.image);
    document.getElementById("wallet-modal-generate-address-title").innerHTML = coin.label + " (" + coin.prefix + ")" + " " + default_language.generate_address;
    $('#wallet-modal-generate-address').modal('show');
}

main();

//functions for tutorial

async function showTutorial(){
    var qry = "SELECT tutorial FROM settings;";
    return await new Promise(function(resolve, reject){
        db.all(qry, [], (err, rows) => {
            if(err){
                console.log(err);
            }
            resolve(rows[0].tutorial);
        });
    });
}

function disableTutorial(option){
    var qry = "UPDATE settings SET tutorial = 0;";
    db.run(qry, [], (err) => {
        if(err){
            console.log(err);
        }
    });
    if(option === "salir"){
        Showcaser.showcase(default_language.tutorial_closed, document.getElementById("settings_menu"), {
            buttonText: "Ok",
            skipText: "",
            scrollBufferPx: -1,
            shape: "rectangle",
            allowSkip: false,
            position: {
                horizontal: "right"
            },
            positionTracker: true,
            paddingPx: 0
        });
    }
}

function enableTutorial(){
    var qry = "UPDATE settings SET tutorial = 1";
    var enable_tutorials = "UPDATE tutorials SET portfolio = 1, wallet = 1, wallet_modal = 1, wallet_modal_contacts = 1, wallet_modal_addresses = 1, wallet_modal_generate = 1, contacts = 1, add_contact = 1, transactions = 1, advanced_filters = 1, settings = 1, help = 1";
    db.run(qry, [], (err) => {
        if(err){
            console.log(err);
        }
    });
    db.run(enable_tutorials, [], (err) => {
        if(err){
            console.log(err);
        }
    });
    setTimeout( () => {
        portfolioTutorial();
    }, 600);
}

function addButton(view){
    setTimeout( () => {
        var button = document.createElement("button");
        button.className = "close close-fix";
        button.innerHTML = "&times;";
        button.addEventListener("click", function(event){
            console.log("skip");
            Showcaser.skipAll();
        });
        document.getElementsByClassName("showcaser-container")[0].appendChild(button);
        document.getElementsByClassName('showcaser-skip')[0].addEventListener("click", ( event ) => {
            disableTutorial("salir");
        });
        switch(view){
            case 'portfolio':
                if(Showcaser.queueLength === 1 || Showcaser.queueLength === 0){
                    document.getElementsByClassName("close-fix")[0].style.marginRight = "96%"
                }
                break;
            case 'transactions':
                if(Showcaser.queueLength === 0){
                    document.getElementsByClassName("close-fix")[0].style.marginRight = "96%"
                }
                break;
            default:
                break;    
        }
    }, 200);
}

async function watchTutorial(view){
    console.log(view);
    var qry = "SELECT "+ view +" FROM tutorials";    
    return await new Promise(function(resolve, reject){
        db.all(qry, [], (err, docs) => {
            if(err){
                reject(err);
            }

            var resp = JSON.stringify(docs[0]);
            if(resp.split(":")[1].replace("}","") === "0"){
                resolve(false);
            }
            resolve(true);
        });
    });
} 

function tutorialWatched(view){
    var qry = "UPDATE Tutorials set "+ view +" = ?";
    db.run(qry, [0], err => {
        if(err){
            console.log(err);
        }
    });
}

/******************************************************************************************************
*                                             Tutorials                                               *
******************************************************************************************************/

function initTutorial(){
    var qry = "select first_time from Tutorials";
    db.all(qry, [], ( err, docs ) => {
        if(docs[0].first_time == 1){
            enableTutorial();
            db.run("UPDATE Tutorials set first_time = 0", [], err => { if(err) console.log(err) })
            Showcaser.showcase(default_language.welcome_tutorial, null, {
                buttonText: default_language.yes,
                skipText: default_language.no,
                before: () => {
                    addButton();
                }
            });
            document.getElementsByClassName("showcaser-button")[0].addEventListener("click", function(){
                portfolioTutorial();
            });
        }else{
            portfolioTutorial(); 
        }
    });
}

function homeTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            Showcaser.showcase("Esta es la pantalla de <strong>Inicio</strong> aqui serán desplegadas todas sus billeteras creadas con Cypher.", null, {
                buttonText: default_language.next,
                skipText: default_language.exit,
                before: () => {
                    addButton("home");
                },
                backgroundColor:{
                    r: 0,
                    g: 0,
                    b: 0,
                    a: 0.92
                }
            });
            Showcaser.showcase("Esta es una billetera. </br> Cada una de ellas contiene información diferente dependiendo de sus transferencias.", document.getElementById("wallet-container-1"), {
                buttonText: default_language.next,
                skipText: default_language.exit,
                before: () => {
                    addButton("home");
                },
                backgroundColor:{
                    r: 0,
                    g: 0,
                    b: 0,
                    a: 0.92
                },
                positionTracker: true,
                position:{
                    vertical: 'middle',
                    horizontal: 'right'
                },
                shape: 'rectangle',
                paddingPx: 0
            });
            Showcaser.showcase("Sus billeteras serán divididas en 2 categorias para un mayor control.", null, {
                buttonText: default_language.next,
                skipText: default_language.exit,
                before: () => {
                    addButton("home");
                },
                backgroundColor:{
                    r: 0,
                    g: 0,
                    b: 0,
                    a: 0.92
                }
            });
            Showcaser.showcase("Billeteras de NFC, donde usted podra guardar un respaldo en una tarjeta NFC para su posterior uso.", document.getElementById("nfc_wallet_header"), {
                buttonText: default_language.next,
                skipText: default_language.exit,
                before: () => {
                    addButton("home");
                },
                backgroundColor:{
                    r: 0,
                    g: 0,
                    b: 0,
                    a: 0.92
                },
                positionTracker: true,
                position:{
                    vertical: 'bottom',
                    horizontal: 'center'
                },
                shape: 'rectangle',
            });
            Showcaser.showcase("Y las billeteras almacenadas en el dispositivo, donde usted podrá manejar sus transacciones solo por medio de Cypher.", document.getElementById("device_wallet_header"), {
                buttonText: default_language.next,
                skipText: default_language.exit,
                before: () => {
                    addButton("home");
                },
                backgroundColor:{
                    r: 0,
                    g: 0,
                    b: 0,
                    a: 0.92
                },
                positionTracker: true,
                position:{
                    vertical: 'top',
                    horizontal: 'center'
                },
                shape: 'rectangle',
            });
            Showcaser.showcase("Y las billeteras almacenadas en el dispositivo, donde usted podrá manejar sus transacciones solo por medio de Cypher.", document.getElementById("device_wallet_header"), {
                buttonText: default_language.next,
                skipText: default_language.exit,
                before: () => {
                    addButton("home");
                },
                backgroundColor:{
                    r: 0,
                    g: 0,
                    b: 0,
                    a: 0.92
                },
                positionTracker: true,
                position:{
                    vertical: 'top',
                    horizontal: 'center'
                },
                shape: 'circle',
            });
        }
    });
}

function portfolioTutorial(){
    showTutorial().then(function(data){
        if(data != 0){
            watchTutorial('portfolio').then( watch => {
                if(watch){
                    tutorialWatched("portfolio");   
                    var side_nav = document.getElementById("side-nav");
                    Showcaser.showcase(default_language.portfolio_tutorial1 , side_nav, {
                        buttonText: default_language.next,
                        skipText: default_language.exit,
                        position:{
                            horizontal: "right"
                        },
                        shape: "rectangle",
                        positionTracker: true,
                        paddingPx: 0,
                        before: () => {
                            addButton("portfolio");
                        }
                    });
                    Showcaser.showcase(default_language.portfolio_tutorial2, document.getElementById("portfolio-side_nav"), {
                        shape: "rectangle",
                        position:{
                            horizontal: "right"
                        },
                        buttonText: default_language.next,
                        paddingPx: 0,
                        skipText: default_language.exit,
                        before: () => {
                            addButton("portfolio");
                        },
                        positionTracker: true
                    });
                    Showcaser.showcase(default_language.portfolio_tutorial3, document.getElementById("wallet-side_nav"), {
                        shape: "rectangle",
                        position:{
                            horizontal: "right"
                        },
                        buttonText: default_language.next,
                        paddingPx: 0,
                        skipText: default_language.exit,
                        before: () => {
                            addButton("portfolio");
                        },
                        positionTracker: true
                    });
                    Showcaser.showcase(default_language.portfolio_tutorial4, document.getElementById("contacts-side_nav"), {
                        shape: "rectangle",
                        position:{
                            horizontal: "right"
                        },
                        buttonText: default_language.next,
                        paddingPx: 0,
                        skipText: default_language.exit,
                        before: () => {
                            addButton("portfolio");
                        },
                        positionTracker: true
                    });
                    Showcaser.showcase(default_language.portfolio_tutorial5, document.getElementById("transactions-side_nav"), {
                        shape: "rectangle",
                        position:{
                            horizontal: "right"
                        },
                        buttonText: default_language.next,
                        paddingPx: 0,
                        skipText: default_language.exit,
                        before: () => {
                            addButton("portfolio");
                        },
                        positionTracker: true
                    });
                    Showcaser.showcase(default_language.portfolio_tutorial6, document.getElementById("settings_menu"), {
                        shape: "rectangle",
                        position:{
                            horizontal: "right"
                        },
                        buttonText: default_language.next,
                        skipText: default_language.exit,
                        paddingPx: 0,
                        positionTracker: true,
                        before: () => {
                            addButton("portfolio");
                        }
                    });
                    Showcaser.showcase(default_language.portfolio_tutorial7, document.getElementById("help-side_nav"), {
                        shape: "rectangle",
                        buttonText: default_language.next,
                        skipText: default_language.exit,
                        positionTracker: true,
                        paddingPx: 0,
                        position:{
                            vertical: "top",
                            horizontal: "right"
                        },
                        scrollBufferPx: -1,
                        before: () => {
                            addButton("portfolio");
                        },
                    });
                    Showcaser.showcase(default_language.portfolio_tutorial8, document.getElementById("mycanvas"), {
                        shape: "circle",
                        position:{
                            horizontal:"right"
                        },
                        buttonText: default_language.next,
                        skipText: default_language.exit,
                        positionTracker: true,
                        paddingPx: 0,
                        before: () => {
                            addButton("portfolio");
                        }
                    });
                    Showcaser.showcase(default_language.portfolio_tutorial9, document.getElementById("cryptoContent"), {
                        shape: "rectangle",
                        position:{
                            horizontal:"left"
                        },
                        buttonText: default_language.next,
                        skipText: default_language.exit,
                        positionTracker: true,
                        paddingPx: 20,
                        before: () => {
                            addButton("portfolio");
                        }
                    });    
                    Showcaser.showcase(default_language.portfolio_tutorial10, document.getElementById("PortfolioImageContainer"),{
                        shape: "rectangle",
                        position:{
                            horizontal:"left"
                        },
                        buttonText: default_language.next,
                        skipText: default_language.exit,
                        positionTracker: true,
                        before: () => {
                            addButton("portfolio");
                        }
                    });
                }else{
                    console.log("ya lo ha visto");
                }
            });
        }else{
            console.log("tutorial deshabilitado")
        }
    });
}

function walletTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial('wallet').then( watch => {
            if(watch){
                tutorialWatched('wallet');
                var bitcoin = document.getElementById("bitcoin-wallet-div");
                var wallet;
                setTimeout( (  ) => {
                    Showcaser.showcase(default_language.wallet_tutorial1, wallet, {
                        buttonText: default_language.next,
                        skipText: default_language.exit,
                        before: () => {
                            addButton("wallet");
                        }
                    });
                    Showcaser.showcase(default_language.wallet_tutorial2, bitcoin, {
                        buttonText: default_language.next,
                        skipText: default_language.exit,
                        positionTracker: true,
                        before: () => {
                            addButton("wallet");
                        },
                        position:{
                            vertical: "bottom",
                            horizontal: "center"
                        }
                    });
                }, 500);
            }else{
                console.log("ya lo ha visto");
            }
            });
        }else{
            console.log("Tutorial deshabilitado");
        }
    }).catch( err => {
        console.error(err);
    });
}


function walletModalTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial('wallet_modal').then( watch => {
                if(watch){
                    tutorialWatched("wallet_modal");   
                    setTimeout( (  ) => {
                        var input_address, input_amount, input_range, buttonadd;
                        input_address = document.getElementById("wallet-modal-input-address");
                        buttonadd = document.getElementById("wallet-modal-receive-button-selectcontact");
                        input_amount = document.getElementById("wallet-modal-input-amount");
                
                        Showcaser.showcase(default_language.walletmodal_tutorial1, input_address, {
                            shape: "rectangle",
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            before: () => {
                                addButton("wallet_modal");
                            }
                        });
                        Showcaser.showcase(default_language.walletmodal_tutorial2, buttonadd, {
                            shape: "rectangle", 
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                horizontal: "left"
                            },
                            positionTracker: true,
                            before: () => {
                                addButton("wallet_modal");
                            }
                        });
                        Showcaser.showcase(default_language.walletmodal_tutorial3, document.getElementById("wallet-modal-input-amount"), {
                            shape: "rectangle",
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            position:{
                                horizontal: "center",
                                vertical: "bottom"
                            },
                            before: () => {
                                addButton("wallet_modal");
                            }
                        });
                        Showcaser.showcase(default_language.walletmodal_tutorial4, document.getElementById("wallet-modal-range-fee"), {
                            shape: "rectangle",
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            position:{
                                horizontal: "right"
                            },
                            before: () => {
                                addButton("wallet_modal");
                            }
                        });
                        Showcaser.showcase(default_language.walletmodal_tutorial5, document.getElementById("wallet-modal-button-send"), {
                            shape: "rectangle",
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            position:{
                                horizontal: "left"
                            },
                            before: () => {
                                addButton("wallet_modal");
                            }
                        });
                        Showcaser.showcase(default_language.walletmodal_tutorial6, document.getElementById("wallet-modal-button-receive"), {
                            shape: "rectangle",
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            position:{
                                horizontal: "right"
                            },
                            before: () => {
                                addButton("wallet_modal");
                            }
                        });
                    }, 500);
                }else{
                    console.log("ya lo ha visto");   
                }
            });
        }else{
            console.log("Tutorial deshabilitado");
        }
    }).catch( err => {
        console.error(err);
    });
}

function walletContactsTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial('wallet_modal_contacts').then( watch => {
                if(watch){
                    tutorialWatched("wallet_modal_contacts");   
                    var table = document.getElementById("wallet-modal-contact-table").getElementsByTagName("tr");
                    setTimeout( ( ) => {
                        Showcaser.showcase(default_language.walletmodalcontacts_tutorial1, document.getElementById("wallet-modal-add-contact-search_container"), {
                            shape: "rectangle",
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            before: () => {
                                addButton("wallet_modal_contacts");
                            },
                            position:{
                                vertical: "bottom",
                                horizontal: "center"
                            }
                        });
                        Showcaser.showcase(default_language.walletmodalcontacts_tutorial2, document.getElementById("wallet-div-table-scroll"),{
                            shape: "rectangle",
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            before: () => {
                                addButton("wallet_modal_contacts");
                            }
                        });
                        if(table.length > 1){
                            Showcaser.showcase(default_language.walletmodalcontacts_tutorial3, document.getElementById("button-send_address"),{
                                shape: "rectangle",
                                buttonText: default_language.next,
                                skipText: default_language.exit,
                                position:{
                                    horizontal: "left"
                                },
                                positionTracker: true,
                                before: () => {
                                    addButton("wallet_modal_contacts");
                                }
                            });
                        }
                    }, 500);
                }else{
                    console.log("ya lo ha visto");
                }
            });
        }else{
            console.log("Tutorial deshabilitado");
        }
    }).catch( err => {
        console.error(err);
    });
}

function receiveTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial("wallet_modal_addresses").then( watch => {
                if(watch){
                    tutorialWatched("wallet_modal_addresses");   
                    var table = document.getElementById("wallet-modal-receive-table").getElementsByTagName("tr");
                    setTimeout( ( ) => {
                        if(table.length > 1){
                            Showcaser.showcase(default_language.walletmodaladdresses_tutorial1, document.getElementById("wallet-modal-receive-div-table-scroll"),{
                                shape: "rectangle",
                                buttonText: default_language.next,
                                skipText: default_language.exit,
                                positionTracker: true,
                                position:{
                                    vertical: "top",
                                    horizontal: "center"
                                },
                                before: () => {
                                    addButton("wallet_modal_addresses");
                                }
                            });
                            Showcaser.showcase(default_language.walletmodaladdresses_tutorial2, document.getElementById("wallet-modal-receive-first-address"), {
                                shape: "rectangle",
                                buttonText: default_language.next,
                                skipText: default_language.exit,
                                positionTracker: true,
                                before: () => {
                                    addButton("wallet_modal_addresses");
                                }
                            });
                        }else{
                            Showcaser.showcase(default_language.walletmodaladdresses_tutorial3, document.getElementById("wallet-modal-receive-div-table-scroll"),{
                                shape: "rectangle",
                                buttonText: default_language.next,
                                skipText: default_language.exit,
                                positionTracker: true,
                                position:{
                                    vertical: "bottom",
                                    horizontal: "center"
                                },
                                before: () => {
                                    addButton("wallet_modal_addresses");
                                }
                            });
                        }
                        Showcaser.showcase(default_language.walletmodaladdresses_tutorial4, document.getElementById("wallet-modal-receive-input-search"), {
                            shape: "rectangle",
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                horizontal: "center",
                                vertical: "bottom"
                            },
                            before: () => {
                                addButton("wallet_modal_addresses");
                            }
                        });
                        Showcaser.showcase(default_language.walletmodaladdresses_tutorial5, document.getElementById("wallet-modal-receive-button-generate"), {
                            shape: "rectangle",
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            position:{
                                horizontal: "left"
                            },
                            before: () => {
                                addButton("wallet_modal_addresses");
                            }
                        });
                    }, 500);
                }else{
                    console.log("ya lo ha visto");
                }
            });
        }else{
            console.log("Tutorial deshabilitado");
        }
    }).catch( err => {
        console.error(err);
    });
}

function generateAddressTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial('wallet_modal_generate').then( watch => {
                if(watch){
                    tutorialWatched("wallet_modal_generate");   
                    setTimeout( ( ) => {
                        Showcaser.showcase(default_language.walletmodalgenerate_tutorial1, document.getElementById("wallet-modal-generate-address-description-container"), {
                            shape: "rectangle",
                            position:{
                                vertical: "bottom",
                                horizontal: "center"
                            },
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            before: () => {
                                addButton("wallet_modal_generate");
                            }
                        });
                        Showcaser.showcase(default_language.walletmodalgenerate_tutorial2, document.getElementById("wallet-modal-generate-address-address_type-container"), {
                            shape: "rectangle",
                            position:{
                                vertical: "top",
                                horizontal: "center"
                            },
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            before: () => {
                                addButton("wallet_modal_generate");
                            }
                        });
                        Showcaser.showcase(default_language.walletmodalgenerate_tutorial3, document.getElementById("wallet-modal-generate-address-button-generate"), {
                            shape: "rectangle",
                            position:{
                                horizontal: "left"
                            },
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            before: () => {
                                addButton("wallet_modal_generate");
                            }
                        });
                    }, 500);
                }else{
                    console.log("ya lo ha visto");
                }
            });
        }else{
            console.log("Tutorial deshabilitado");
        }
    }).catch( err => {
        console.error(err);
    });
}

function contactsTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial('contacts').then( watch => {
                if(watch){
                    tutorialWatched("contacts");   
                    setTimeout( ( ) => {
                        Showcaser.showcase(default_language.contacts_tutorial1, null, {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            before: () => {
                                addButton("contacts");
                            }
                        });
                        Showcaser.showcase(default_language.contacts_tutorial2, document.getElementById("contacts-div-search-container"),{
                            shape: "rectangle",
                            positionTracker: true,
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                vertical: "bottom",
                                horizontal: "center"
                            },
                            before: () => {
                                addButton("contacts");
                            }
                        });
                        Showcaser.showcase(default_language.contacts_tutorial3, document.getElementById("contact-div-table-scroll"),{
                            shape: "rectangle",
                            positionTracker: true,
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                vertical: "top",
                                horizontal: "center"
                            },
                            before: () => {
                                addButton("contacts");
                            }
                        });
                        if(document.getElementById("contact-table").getElementsByTagName("tr").length > 1){
                            Showcaser.showcase(default_language.contacts_tutorial4, document.getElementById("contact-button-edit"), {
                                shape: "rectangle",
                                positionTracker: true,
                                buttonText: default_language.next,
                                skipText: default_language.exit,
                                position:{
                                    horizontal: "left"
                                },
                                before: () => {
                                    addButton("contacts");
                                }
                            });
                            Showcaser.showcase(default_language.contacts_tutorial5, document.getElementById("contact-button-delete"), {
                                shape: "rectangle",
                                positionTracker: true,
                                buttonText: default_language.next,
                                skipText: default_language.exit,
                                position:{
                                    horizontal: "left"
                                },
                                before: () => {
                                    addButton("contacts");
                                }
                            });
                        }
                        Showcaser.showcase(default_language.contacts_tutorial6, document.getElementById("table-add-btn"), {
                            shape: "rectangle",
                            positionTracker: true,
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                horizontal: "left"
                            },
                            before: () => {
                                addButton("contacts");
                            }
                        });
                    }, 500);
                }else{
                    console.log("ya lo ha visto");
                }
            });
        }else{
            console.log("Tutorial deshabilitado");
        }
    }).catch( err => {
        console.error(err);
    });
}

function addContactTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial('add_contact').then( watch => {
                if(watch){
                    tutorialWatched("add_contact");   
                    setTimeout( ( ) => {
                        Showcaser.showcase(default_language.addcontact_tutorial1, document.getElementById("add-contact-modal-name-container"), {
                            shape: "rectangle",
                            position:{
                                vertical: "bottom",
                                horizontal:"center"
                            },
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            before: () => {
                                addButton("add_contact");
                            }
                        });
                        Showcaser.showcase(default_language.addcontact_tutorial2, document.getElementById("add-contact-modal-address-container"), {
                            shape: "rectangle",
                            position:{
                                vertical: "bottom",
                                horizontal:"center"
                            },
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            before: () => {
                                addButton("add_contact");
                            }
                        });
                        Showcaser.showcase(default_language.addcontact_tutorial3, document.getElementById("add-contact-modal-coin-container"), {
                            shape: "rectangle",
                            position:{
                                vertical: "bottom",
                                horizontal:"center"
                            },
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            before: () => {
                                addButton("add_contact");
                            }
                        });
                        Showcaser.showcase(default_language.addcontact_tutorial4, document.getElementById("add-contacts-modal-add-btn"), {
                            shape: "rectangle",
                            position:{
                                horizontal:"left"
                            },
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            before: () => {
                                addButton("add_contact");
                            }
                        });
                    }, 500);
                }else{
                    console.log("ya lo ha visto");
                }
            });
        }else{
            console.log("Tutorial deshabilitado");
        }
    }).catch( err => {
        console.error(err);
    });
}

function transactionTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial('transactions').then( watch => {
                if(watch){
                    tutorialWatched("transactions");   
                    setTimeout( ( ) => {
                        Showcaser.showcase(default_language.transaction_tutorial1, null, {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            before: () => {
                                addButton("transactions");
                            }
                        });
                        Showcaser.showcase(default_language.transaction_tutorial2, document.getElementById("transaction-div-table"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            position:{
                                vertical: "bottom",
                                horizontal: "center"
                            },
                            shape: "rectangle",
                            before: () => {
                                addButton("transactions");
                            }
                        });
                        Showcaser.showcase(default_language.transaction_tutorial3, document.getElementById("transaction-table-header"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            position:{
                                vertical: "bottom",
                                horizontal: "center"
                            },
                            shape: "rectangle",
                            before: () => {
                                addButton("transactions");
                            }
                        });
                        Showcaser.showcase(default_language.transaction_tutorial4, document.getElementById("transactions-advanced-filter-btn"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            position:{
                                horizontal: "left"
                            },
                            shape: "rectangle",
                            before: () => {
                                addButton("transactions");
                            }
                        });
                    }, 400);
                }else{
                    console.log("ya lo ha visto");
                }
            });
        }else{
            console.log("Tutorial deshabilitado");
        }
    }).catch( err => {
        console.error(err);
    });
}

function advancedFiltersTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial('advanced_filters').then( watch => {
                if(watch){
                    tutorialWatched("advanced_filters");   
                    setTimeout( ( ) => {
                        Showcaser.showcase(default_language.advancedfilter_tutorial1, document.getElementById("advanced-filter-modal-body"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                vertical: "bottom",
                                horizontal: "center"
                            },
                            positionTracker: true,
                            shape: "rectangle",
                            before: () => {
                                addButton("advanced_filters");
                            }
                        });
                        Showcaser.showcase(default_language.advancedfilter_tutorial2, document.getElementById("advanced-filter-modal-coin_type-container"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                vertical: "bottom",
                                horizontal: "center"
                            },
                            positionTracker: true,
                            shape: "rectangle",
                            before: () => {
                                addButton("advanced_filters");
                            }
                        });
                        Showcaser.showcase(default_language.advancedfilter_tutorial3, document.getElementById("advanced-filter-modal-year-container"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                vertical: "bottom",
                                horizontal: "center"
                            },
                            positionTracker: true,
                            shape: "rectangle",
                            before: () => {
                                addButton("advanced_filters");
                            }
                        });
                        Showcaser.showcase(default_language.advancedfilter_tutorial4, document.getElementById("advanced-filter-modal-month-container"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                vertical: "bottom",
                                horizontal: "center"
                            },
                            positionTracker: true,
                            shape: "rectangle",
                            before: () => {
                                addButton("advanced_filters");
                            }
                        });
                        Showcaser.showcase(default_language.advancedfilter_tutorial5, document.getElementById("advanced-filter-modal-date-container"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                vertical: "top",
                                horizontal: "center"
                            },
                            positionTracker: true,
                            shape: "rectangle",
                            before: () => {
                                addButton("advanced_filters");
                            }
                        });
                        Showcaser.showcase(default_language.advancedfilter_tutorial6, document.getElementById("advanced-filter-modal-clear-btn"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                vertical: "top",
                                horizontal: "center"
                            },
                            positionTracker: true,
                            shape: "rectangle",
                            before: () => {
                                addButton("advanced_filters");
                            }
                        });
                    }, 500);
                }else{
                    console.log("ya lo ha visto");
                }
            });
        }else{
            console.log("Tutorial deshabilitado");
        }
    }).catch( err => {
        console.error(err);
    });
}

function settingsTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial('settings').then( watch => {
                if(watch){
                    tutorialWatched("settings");
                    setTimeout( ( ) => {
                        Showcaser.showcase(default_language.settings_tutorial1, null, {
                            buttonText: default_language.next,
                            skipText: default_language.exit 
                        });
                        Showcaser.showcase(default_language.settings_tutorial2, document.getElementById("select-language"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                horizontal: "right"
                            },
                            positionTracker: true,
                            before: () => {
                                addButton('settings');
                            },
                            shape: "rectangle"
                        });
                        Showcaser.showcase(default_language.settings_tutorial3, document.getElementById("select-mode"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                horizontal: "right"
                            },
                            positionTracker: true,
                            before: () => {
                                addButton('settings');
                            },
                            shape: "rectangle"
                        });
                        Showcaser.showcase(default_language.settings_tutorial4, document.getElementById("settings-enable_tutorial-button"), {
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            position:{
                                horizontal: "right"
                            },
                            positionTracker: true,
                            before: () => {
                                addButton('settings');
                            },
                            shape: "rectangle"
                        });   
                    }, 400);
                }else{
                    console.log("ya lo vio");
                }
            })
        }else{
            console.log("tutorial deshabilitado");
        }
    })
}

function helpTutorial(){
    showTutorial().then( data => {
        if(data != 0){
            watchTutorial('help').then( watch => {
                if(watch){
                    tutorialWatched("help");
                    setTimeout( ( ) => {
                        Showcaser.showcase(default_language.help_tutorial1, null, {
                            buttonText: default_language.next,
                            skipText: default_language.exit
                        });
                        Showcaser.showcase(default_language.help_tutorial2, document.getElementById("cards1"), {
                            position:{
                                vertical: "bottom",
                                horizontal: "center"
                            },
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            before: () => {
                                addButton('help');
                            },
                            shape: "rectangle"
                        });
                        Showcaser.showcase(default_language.help_tutorial3, document.getElementById("mailSender"), {
                            position:{
                                horizontal: "left"
                            },
                            buttonText: default_language.next,
                            skipText: default_language.exit,
                            positionTracker: true,
                            before: () => {
                                addButton('help');
                            },
                            shape: "rectangle"
                        });
                    }, 400);
                }else{
                    console.log("ya lo ha visto");
                }
            });
        }else{
            console.log("tutorial deshabilitado");
        }
    });
}

function synchronizeDatabase(){
    var flag = false;
    executePythonScript(["seriallisten.py", "3"]).then((database)=>{
        checkDatabaseInfo().then((db_pc)=>{
            console.log(database);
            console.log(db_pc);
            //compare addresses
            if(db_pc.addresses.length != database[3].length){
                flag = true;
            }else{
                for(var i=0; i<db_pc.addresses.length; i++){
                    if(database[1][i] != db_pc.addresses[i].wallet_id || database[2][i] != db_pc.addresses[i].coin_id || database[3][i] != db_pc.addresses[i].address){
                        flag = true;
                    }
                }

            }
            //compare wallets
            if(db_pc.wallets.length != database[0].length/3){
                flag = true;
            }else{
                for(var i=0; i<db_pc.wallets.length; i++){
                    if(db_pc.wallets[i].wallet_id != database[0][i*3] || db_pc.wallets[i].wallet_name != database[0][(i*3)+1] || db_pc.wallets[i].wallet_type != database[0][(i*3)+2]){
                        flag = true;
                    }
                }
            }

            if(flag){
                $("#modal-message-warning").modal("show");
                document.getElementById("modal-warning-header-info").innerHTML = default_language.database_change_header;
                document.getElementById("modal-warning-body-info").innerHTML = default_language.database_change_message;
                document.getElementById("modal-btn-warning-yes").innerHTML = default_language.yes;
                document.getElementById("modal-btn-warning-no").innerHTML = default_language.no;
                document.getElementById("modal-btn-warning-yes").addEventListener("click", function(){
                    $("#loading-modal").modal("show").on('shown.bs.modal', ()=>{
                        deleteLocalDatabase().then((result)=>{
                            if(result){
                                addLocalDatabase(database).then((result)=>{
                                    loadWallets();
                                    $("#loading-modal").modal("hide");                                
                                }).catch((error)=>{
                                    console.log(error);
                                });
                            }
                        }).catch((error)=>{
                            console.log(error);
                        });
                    });
                });
            }
        });

    });
}

async function checkDatabaseInfo(){
    return new Promise((resolve, reject)=>{
        db.all('SELECT address, coin_id, wallet_id FROM addresses', [], (error, addresses)=>{
            if(error){
                reject(error);
            }
            db.all('SELECT wallet_id, wallet_name, wallet_type FROM wallet', [], (err, wallets)=>{
                if(err){
                    reject(err);
                }
                resolve({addresses: addresses, wallets: wallets});
            });
        });  
    });
}

async function deleteLocalDatabase(){
    return new Promise((resolve, reject)=>{
        db.all('DELETE FROM transactions', [], (error)=>{
            if(error){
                reject(error);
            }
            db.all('DELETE FROM addresses', [], (error2)=>{
                if(error2){
                    reject(error2);
                }
                db.all('DELETE FROM wallet', [], (error3)=>{
                    if(error3){
                        reject(error3);
                    }
                    resolve(true);
                });  
            });  
        });  
    });
}

async function addLocalDatabase(database){
    return new Promise(async (resolve, reject)=>{
        var flag = false;
        for(var i = 0; i <= database[0].length/3; i+=3){
            if(database[0][i] && database[0][i+1]){
                await addWalletLocalDatabase(database[0][i], database[0][i+1], database[0][i+2]).then((result)=>{  
                }).catch((error)=>{
                    console.log(error);
                    reject(error);
                });
            }else if(!database[0][i] && !database[0][i+1] && !database[0][i+2]){
                flag = true;
            }
        }
        for(var j=0; j<database[3].length; j++){
            if(database[3][j] && database[2][j] && database[1][j]){
                await addAddressesLocalDatabase(database[3][j], database[2][j], database[1][j]).then((result)=>{
                    if(j == database[3].length-1){
                        flag = true;
                    }
                }).catch((error2)=>{
                    reject(error2);
                })
            }else if(!database[3][j] && !database[2][j] && !database[1][j]){
                flag = true;
            }
        }
        if(flag){
            resolve(true);
        }
    });
}

async function addWalletLocalDatabase(wallet_id, wallet_name, wallet_type){
    return new Promise(async (resolve, reject)=>{
        await db.run("INSERT INTO wallet (wallet_id, wallet_name, wallet_type, wallet_description) VALUES (?, ?, ?, ?);",[wallet_id, wallet_name, wallet_type, ""], (error)=>{
            if(error){
                reject(error);
            }else{
                resolve(true);
            }
        });
    });
}

async function addAddressesLocalDatabase(address, coin_id, wallet_id){
    return new Promise(async (resolve, reject)=>{
        await db.run("INSERT INTO addresses (address, coin_id, wallet_id, description) VALUES (?, ?, ?, ?);",[address, coin_id, wallet_id, ""], (error)=>{
            if(error){
                reject(error);
            }
            resolve(true);
        });               
    });
}

async function updateCypher(){
    document.getElementById("loading-message").innerHTML = default_language.updating;
    $("#loading-modal").modal("show");
    if(await send_update()){
        await update_files();
    }
}

async function send_update(){
    return new Promise(async (resolve, reject)=>{
        await executePythonScript(['seriallisten.py', '1', __dirname.concat("\\update.tar"), "/home/debian/hashbank/update.tar"]).then((result)=>{
            if(result == 'Finished'){
                resolve(true);
            }else{
                reject(false);
            }
        });         
    });
}

async function update_files(){
    return new Promise(async (resolve, reject)=>{
        await executePythonScript(['seriallisten.py', '8']).then((final_result)=>{      
            $("#loading-modal").modal("hide");
            document.getElementById("loading-message").innerHTML = default_language.please_wait;
        }); 
    });
}