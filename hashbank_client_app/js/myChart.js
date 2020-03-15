class myChart{
    constructor(){
        this.labels = [];
        this.background_colors = [];
        this.hover_colors = [];
        this.data = [];
        this.myFirstChart;
    }	
    
    addElement(label, background_color, hover_color, data){
        this.labels.push(label);
        this.background_colors.push(background_color);
        this.hover_colors.push(hover_color);
        this.data.push(data);
    }
    
    createTable(){
        var chrt = document.getElementById("mycanvas").getContext("2d");
        var data = {
            labels: this.labels,
            datasets: [{
                label: "Currencies",
                backgroundColor:  this.background_colors,
                borderColor: "#666666",
                hoverBackgroundColor: this.hover_colors,
                hoverBorderColor: "#1485e2",
                data: this.data
            }]
        };
        this.myFirstChart = new Chart(chrt, {
        type: 'doughnut',
        data: data,
        options: { legend: { display: false }}
        });
    }
}

module.exports = myChart;