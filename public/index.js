

// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
  // On page-load AJAX Example


  // Event listener form example , we can use this instead explicitly listening for events
  // No redirects if possible
  $('#submit').submit(function(e){
    $('#information').html("Form has data: "+$('#entryBox').val());
    e.preventDefault();
    //Pass data to the Ajax call, so it gets passed to the server
    $.ajax({
      //Create an object for connecting to another waypoint
    });
  });

  var string = "";
  for (var i = 0; i < 3; i++) {
    string = string.concat("<tr><td>Path " + (i+1) + "</td><td>path data = M100,200,300 L400,500 M800</td><td>" + Math.ceil(Math.random() * 3) + "</td></tr>");
  }
  for (var i = 0; i < 10; i++) {
    string = string.concat("<tr><td>Group " + (i+1) + "</td><td>2 child elements</td><td>" + Math.ceil(Math.random() * 3) + "</td></tr>");
  }
  var tb = document.getElementById('t02');
  tb.innerHTML = string;

  var button = document.createElement("button");
  button.innerHTML = "Edit";
  document.getElementById("d").appendChild(button);
  button.addEventListener ("click", function() {
    var value = prompt("Please enter new description value", "Value");
    console.log(value);
    document.getElementById('description').innerHTML = value;
  });

  var button2 = document.createElement("button");
  button2.innerHTML = "Edit";
  document.getElementById("t").appendChild(button2);
  button2.addEventListener ("click", function() {
    var value = prompt("Please enter new title value", "Value");
    console.log(value);
    document.getElementById('title').innerHTML = value;
  });

  image.onclick = function() {
  }



  image.addEventListener ("click", function(){
    var input = document.querySelector('input');
    var formData = new FormData();
    formData.append('imageFile', $('#image')[0].files[0]);

    $.ajax({
      url : '/upload/:name',
      type : 'POST',
      data : formData,
      processData: false,
      contentType: false,
      success : function(data) {
        console.log("yes");
      }, fail: function(error) {
        console.log(error);
      }
    });
    //updateImageDisplay(input);
  });

  function returnFileSize(number) {
    if(number < 1024) {
      return number + 'bytes';
    } else if(number > 1024 && number < 1048576) {
      return (number/1024).toFixed(1) + 'KB';
    } else if(number > 1048576) {
      return (number/1048576).toFixed(1) + 'MB';
    }
  }

  function updateImageDisplay(input) {

    console.log("I have changed");

    var curFiles = input.files;
    if (curFiles.length == 0) {
      console.log("haah");
    } else {
      console.log("nothing, keep going " + curFiles[0].name);
    }

    var curFiles = input.files;
    if(curFiles.length === 0) {
      var para = document.createElement('p');
      para.textContent = 'No files currently selected for upload';
      alert(para.textContent);
    } else {
      //Please update this section
      var tablebody = document.getElementById('imgList');
      var row = tablebody.insertRow();
      var cell1 = row.insertCell(0);
      var cell2 = row.insertCell(1);
      var cell3 = row.insertCell(2);
      var cell4 = row.insertCell(3);
      var cell5 = row.insertCell(4);
      var cell6 = row.insertCell(5);
      var cell7 = row.insertCell(6);

      cell1.innerHTML = "<a href= svgFiles/" + curFiles[0].name + "\" download><img class = \"resize\" src= " + URL.createObjectURL(curFiles[0]) + curFiles[0].name + "\" height = \"200\"></img></a>";
      cell2.innerHTML = "<a href= svgFiles/" + curFiles[0].name + "\" download>" + curFiles[0].name + "</a>";
      cell3.innerHTML = returnFileSize(curFiles[0].size);
      cell4.innerHTML = "<center>3</center>";
      cell5.innerHTML = "<center>3</center>";
      cell6.innerHTML = "<center>3</center>";


      /*var string = "";
      string = string.concat("<tr>\n<td><a href= " + curFiles[0].name + "\" download><img class = \"resize\" src=" + curFiles[0].name + "\" height = \"200\"></img></a></td>\n");
      string = string.concat("<td><a href= " + curFiles[0].name + "\" download>" + curFiles[0].name + "</a></td>\n");
      string = string.concat("<td>" + returnFileSize(curFiles[0].size) + "</td>\n");
      string = string.concat("<td><center>3</center></td>\n");
      string = string.concat("<td><center>5</center></td>\n");
      string = string.concat("<td><center>1</center></td>\n");
      string = string.concat("<td><center>8</center></td>\n</tr>");*/
      //alert(string);

      //imgList.appendChild(string);
    }

  }

  /*$.ajax({
  type: 'get',            //Request type
  dataType: 'json',       //Data type - we will use JSON for almost everything
  url: '/filelog',   //The server endpoint we are connecting to
  data: {
  name1: "Value 1",
  name2: "Value 2"
},
success: function (data) {
$('#information').html("On page load, received string '"+data.foo+"' from server");
//We write the object to the console to show that the request was successful
console.log(data);

},
fail: function(error) {
$('#information').html("On page load, received error from server");
console.log(error);
}
});*/

var filename = document.getElementById('filename');

filename.onchange = function() {
  $('#attribute').empty();
  var name = document.getElementById("filename").value;
  document.getElementById("img").src = "svgFiles/" + name;
  var html = "";


  if (name.localeCompare("vest.svg") == 0) {
    for (var i = 0; i < 20; i++) {
      html = html.concat("<tr><td>Path " + (i+1) + "</td><td>path data = M100,200,300 L400,500 M800</td><td>" + Math.ceil(Math.random() * 3) + "</td></tr>");
    }
  } else if(name.localeCompare("hen_and_chicks.svg") == 0) {
    html = html.concat("<tr><td>Group 1</td><td>2 child elements</td><td>1</td></tr>");
    for (var i = 0; i < 20; i++) {
      html = html.concat("<tr><td>Path " + (i+1) + "</td><td>path data = M100,200,300 L400,500 M800</td><td>" + Math.ceil(Math.random() * 3) + "</td></tr>");
    }
  }
  var tb = document.getElementById('t02');
  tb.innerHTML = html;
  console.log("The user selected: " + name);

  /*$.ajax({
  type: 'get',            //Request type
  dataType: 'json',       //Data type - we will use JSON for almost everything
  url: '/filelog',   //The server endpoint we are connecting to
  success: function (data) {
  $('#information').html("On page load, received string '"+data.foo+"' from server");
  //We write the object to the console to show that the request was successful
  console.log(data);

},
fail: function(error) {
$('#information').html("On page load, received error from server");
console.log(error);
}
});*/

}

attribute.onchange = function() {
  var otherAttributes = ["fill:\"black\"","stroke=\"#000\"","stroke-width=\"6.1\""];
  var e = document.getElementById("attribute");
  var result = e.options[e.selectedIndex].text;
  console.log("The user selected: " + result);//error checking
  var string = result + ": " + $(tb.rows[e.selectedIndex].cells[1]).text();
  var number_string = $(tb.rows[e.selectedIndex].cells[2]).text();
  var number = parseInt(number_string);
  string = string.concat("\nOther Attributes: ")
  for (var i = 0; i < number; i++) {
    string = string.concat(otherAttributes[i] + " ");
  }
  var button = document.createElement("button");
  button.innerHTML = "Edit";

  button.addEventListener ("click", function() {
    var name = prompt("Please enter attribute name", "Name");
    if (name.localeCompare("") == 0) {
      name = prompt("Please enter a value attribute name", "Name");
    }
    var value = prompt("Please enter attribute value", "Value");


  });

  console.log(string);
  $('#information').html(string);
  document.getElementById("information").appendChild(button);
}







showAttr.onclick = function() {
  var tb = document.getElementById('t02');
  var show = document.getElementById('attribute');
  var rows = tb.getElementsByTagName('tr');
  console.log(show.length);
  $('#attribute').empty();
  for (var i = 0; i < rows.length;i++) {
    var option = document.createElement("option");
    option.text = $(tb.rows[i].cells[0]).text();
    show.add(option,show[i]);
  }

}






});
