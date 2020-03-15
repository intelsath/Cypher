class Cryptocurrency{
    constructor(label, currency, currency_price_prefix, image, percentage){
        this.label = label;
        this.currency = currency;
        this.currency_price_prefix = currency_price_prefix;
        this.image = image;
        this.percentage = percentage;
        //el total se deberá agregar cuando ya tengamos el precio
        this.total = 0.00;
    }
    price (){
        var cont = 0.00;
        var tot = 0.00;
        var currency_price_prefix = this.currency_price_prefix;
        var currency = this.currency;
        
        $.ajax({
            url: 'https://api.bitfinex.com/v1/pubticker/' + currency_price_prefix.toLowerCase()+'usd',
            dataType: 'json', 
            success: function(data_response){  
                tot += parseFloat(data_response.last_price);
                cont++;
                $.ajax({
                    url: 'https://www.bitstamp.net/api/v2/ticker/' + currency_price_prefix.toLowerCase()+'usd',
                    dataType: 'json', 
                    success: function(data_response2){  
                        tot += parseFloat(data_response2.last);
                        cont++;
                        //coinToUSD((tot/cont), currency);
                    },
                    error: function(data){
                        //coinToUSD((tot/cont), currency);
                    }
                });
            },
            error: function(data){
                $.ajax({
                    url: 'https://www.bitstamp.net/api/v2/ticker/'+currency_price_prefix.toLowerCase()+'usd',
                    dataType: 'json', 
                    success: function(data_response){  
                        tot += parseFloat(data_response.last);
                        cont++;
                        //coinToUSD((tot/cont), currency);				
                    },
                    error: function(data){
                        //coinToUSD((tot/cont), currency);		
                    }
                });
            }
        });
    }    

    addHTML (){
        $('#images').append(
            '<div id="PortfolioImageContainer" class="image-container theme_class2">'+
                '<div id="name-image-portfolio-cards">'+
                    '<p class="title" style="color:white;">'+this.label+'</p>'+
                    '<img src="images/'+this.image+'" width="50" height="50"">'+
                '</div>'+
                '<div class="portfolio-currency-price-container">'+
                    '<p id="'+this.currency+'-percent" class"percent" style="margin-bottom: 0px; margin-top: 17px;">'+this.percentage+'%</p>'+
                    '<p id="'+this.currency+'-price" class="price">'+this.total+'</p>'+
                '</div>'+
            '</div> <br>'
        );

        $('#wallet-container').append(
            '<div name="div-walletContainer" class="col-sm-4 col-md-4 col-lg-3">'+
                '<div class="wallet-images theme_class" id="'+ this.currency +'-wallet-div" onclick="openWalletModal({currency:\''+ this.currency +'\', image: \''+ this.image +'\', prefix: \''+ this.currency_price_prefix +'\', label:\''+ this.label +'\'})" data-toggle="modal" data-target="#'+this.currency+'-modal">'+
                    '<img src="images/'+this.image+'" width="80" height="80">'+
                    '<p class="tittle">'+this.label+'</p>'+
                '</div>'+
            '</div>'
        );
    }
}

module.exports = Cryptocurrency;