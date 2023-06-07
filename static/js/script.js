var element = d3.select('#graph').node();

console.log(element.getBoundingClientRect().width)
var margin = { top: 30, right: 50, bottom: 30, left: 50 },
  width = (800 - margin.left - margin.right),
  height = 500 - margin.top - margin.bottom;
 
var x = d3.scale
  .linear()
  .range([0, width])
  .domain([-100 * (width / height), 100 * (width / height)]);

var y = d3.scale.linear().range([height, 0]).domain([-100, 100]);

var xAxis = d3.svg.axis().scale(x).orient("bottom");

var yAxis = d3.svg.axis().scale(y).orient("left");

var line = d3.svg
  .line()
  .x(function (d) {
    return x(d.x);
  })
  .y(function (d) {
    return y(d.y);
  });

var svg = d3
  .select("#graph")
  .append("svg")
  .attr("width", width + margin.left + margin.right)
  .attr("height", height + margin.top + margin.bottom)
  .append("g")
  .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

svg
  .append("g")
  .attr("class", "xAxis")
  .attr("transform", "translate(0," + height + ")")
  .attr("fill", "#aaaaaa")
  .attr("width", "0px")
  .call(xAxis);

svg
  .append("g")
  .attr("class", "yAxis")
  .attr("x", x(0))
  .attr("y", y(0))
  .attr("fill", "#aaaaaa")
  .call(yAxis);

var imgs;
var div = d3
  .select("body")
  .append("div")
  .attr("class", "tooltip-donut")
  .style("opacity", 0);

var layer1 = svg.append("g");
var layer2 = svg.append("g");

function make_graph(lines, time) {
  var Tooltip = d3
    .select("#tooltip")
    .append("div")
    .style("opacity", 0)
    .attr("class", "tooltip")
    .style("background-color", "#323232")
    .style("color", "#ffffff")
    .style("border", "solid")
    .style("border-color", "#ffffff")
    .style("border-width", "2px")
    .style("padding", "5px");
  // .style("left", d3.mouse(this)[0] + 70 + "px")
  // .style("top", d3.mouse(this)[1] + "px");
  var mouseover = function (d) {
    Tooltip.style("opacity", 1)
      .style("left", d3.mouse(this)[0] + 70 + "px")
      .style("top", d3.mouse(this)[1] + "px");
    d3.select(this).style("stroke", "#00ffff").style("opacity", 0.5);
    console.log("mouseover");
  };
  var mousemove = function (d) {
    Tooltip.html(
      time +
        " : " +
        "(" +
        lines[0][0]["x"] +
        ", " +
        lines[0][0]["y"] +
        ") -> (" +
        lines[0][1]["x"] +
        ", " +
        lines[0][1]["y"] +
        ")"
      // "chirag"
    )
      .style("left", d3.mouse(this)[0] + 70 - 400 + "px")
      .style("top", d3.mouse(this)[1] - 400 + "px");
    console.log("mousemove");
    console.log(d3.mouse(this)[0] + 70 + "px" + d3.mouse(this)[1] + "px");
  };
  var mouseleave = function (d) {
    Tooltip.style("opacity", 0);
    d3.select(this).style("opacity", 1);
    console.log("mouseleave");
  };

  for (var i = 0; i < lines.length; i++) {
    var lln = layer1
      .append("path")
      .attr("class", "plot")
      .datum(lines[i])
      .attr("d", line)
      .style("stroke", "#00ffff")
      .style("stroke-width", "0px")
      .attr("id", "for_graph")
      //   .on("mouseover", mouseover)
      //   .on("mousemove", mousemove)
      //   .on("mouseleave", mouseleave)

      //   .append("svg:title")
      //   .text("Your tooltip info")
      .transition()
      // .ease(d3.easeLinear)
      .duration(800)
      // .attr("height", 100);
      .style("stroke-width", "3px");
  }

  d3.select("#the_SVG_ID").remove();
  imgs = layer2
    .append("svg:image")
    .attr("id", "the_SVG_ID")
    .attr("xlink:href", "/static/my-image.png")
    .attr("x", x(lines[0][1]["x"]))
    .attr("y", y(lines[0][1]["y"] + 15))
    .attr("width", "60")
    .attr("height", "60");

  // d3.select("#the_SVG_ID").remove();
  imgs = layer2
    .append("circle")
    .attr("cx", x(lines[0][1]["x"]))
    .attr("cy", y(lines[0][1]["y"]))
    .attr("r", 4)
    .attr("stroke", "#00ffff")
    .attr("fill", "black")
    // .raise()
    .on("mouseover", function (d, i) {
      d3.select(this).transition().duration("50").attr("opacity", ".7");
      div.transition().duration(50).style("opacity", 1);
      let num = "(x,y)=(" + lines[0][1]["x"] + "," + lines[0][1]["y"] + ")";
      div
        .html(num)
        .style("left", d3.event.pageX + "px")
        .style("top", d3.event.pageY + "px");
    })

    .on("mouseout", function (d, i) {
      d3.select(this).transition().duration("50").attr("opacity", "1");
      div.transition().duration("50").style("opacity", 0);
    });
}

let data = {};

var init_x = 0,
  init_y = 0;

function requestData() {
  $.ajax({
    url: "/live-data",
    success: function (point) {
      if (!data[point[2]]) {
        var lines = [
          [
            { x: init_x, y: init_y },
            { x: point[0], y: point[1] },
          ],
        ];
        make_graph(lines, point[2]);
        console.log(lines);
        init_x = point[0];
        init_y = point[1];
        data[point[2]] = 1;
      }
      setTimeout(requestData, 1000);
    },
    cache: false,
  });
}

function init_data() {
  $.ajax({
    url: "/live-data-init",
    success: function (point) {
      for (i = 0; i < point.length - 1; i++) {
        var lines = [
          [
            { x: init_x, y: init_y },
            { x: point[i]["x"], y: point[i]["y"] },
          ],
        ];
        make_graph(lines, point[i]["t"]);
        console.log(lines);
        init_x = point[i]["x"];
        init_y = point[i]["y"];
        data[point[i]["t"]] = 1;
      }
      requestData();
    },
    cache: false,
  });
}

$(document).ready(function () {
  init_data();
});

let data1=[17,20,30]; // left, straight, right
// let arr =[{x1,y1},{x2,y2}];;
// write math 


const labels1 = ["Left","Straight","Right"];
// const labels2=[]  //--> should be time array for graph 2
// let labels1 = ["L","S","R"];
function makechart(id,label,data,chart_label)
{
  Chart.defaults.global.responsive = false;
  // Chart.defaults.color = "blue";
//   Chart.defaults.backgroundColor = '#9BD0F5';
// Chart.defaults.borderColor = '#36A2EB';
   chartData= {
    labels: label,
    datasets: [{
      label: chart_label,
      data: data,
      fill : false,
      borderColor: 'rgb(75, 192, 192)',
      backgroundColor: '#9BD0F5',
      color : '#000'
    }]
  };

 // get chart canvas
//  var ctx = document.getElementById(id).getContext('2d');
var ctx = document.getElementById(id).getContext("2d");

var temp = new Chart(ctx, {
    type: 'bar',
    data: chartData,
    options: {
      scales: {
  
        yAxes: [{
          ticks: {
              beginAtZero: true,
              fontColor : "white",
            color : "white"
            
          },
          gridLines: {
            color : "white"
          ,display : false

        },
        color : "white",
      },
    ],
      xAxes: [{
        ticks: {
            fontColor : "white",
            color : "white"
        },
        gridLines: {
          color : "white"
          ,display : false
      }
    }]
      }
    },
  });
}
makechart("overall_analysis",labels1,data1,"Directions");
makechart("sleep_scr",labels1,data1,"Directions");

// makechart("sleep_scr",time_labels,number_of_obstacles_detected,"Directions");
