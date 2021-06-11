//Login page loads first

// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything
        url: '/endpoint1',   //The server endpoint we are connecting to
        data: {
            stuff: "",
            junk: ""
        },
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string,
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            //$('#show-other').html("User info was validated!");
            //We write the object to the console to show that the request was successful
            console.log(data);

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#show-other').html("Invalid info: connection unsuccessful");
            console.log(error);
        }
    });

    //recieving login information
    $('#login-btn').click(function (e) {
        console.log("in index and recieved user info");
        e.preventDefault();
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/loginInfo',
            data: {
                usersName: $('#usernameTxt').val(),
                usersPass: $('#passwordTxt').val(),
                dataBase: $('#databaseTxt').val(),
                statusDB: ""
            },
            success: function (data) {
                console.log(data.usersName + " " + data.statusDB);
                if (data.statusDB === "unsuccessful app.js<-sql") {
                    alert("Login unsuccessful! Please enter your credentials again:\n\n             Username, Password, Database Name");
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
        document.getElementById('passwordTxt').value='';
        document.getElementById('usernameTxt').value='';
        document.getElementById('databaseTxt').value='';
    });

    //to show the file log table and other info from a3 ui
    $('#login-to-index').click(function (e) {
        e.preventDefault();
        var show = document.getElementById('main-index');
        if (show.style.display === "none") {
            show.style.display = "block";
            //$('#main-index').show(1000);
        } else {
            show.style.display = "none";
            //$('#main-index').hide(1000);
        }
    });

    //uploading a new gpx file to the file log table
    $(document).ready(function() {
    //$('#show-other').submit(function(e){
      $.ajax({
          type: 'get',
          dataType: 'json',
          url: '/fileLogPop',
          data: {
              gpxJSON: "",
              nameT: ""
          },
          success: function (data) {
              let filestuff = JSON.parse(data.gpxJSON);
              var filename = data.nameT;
              $('#filleeee').html("GPX doc " + filename + " added!");
              var ver = filestuff.version;
              var cre = filestuff.creator;
              var wpt = filestuff.numWaypoints;
              var rte = filestuff.numRoutes;
              var trk = filestuff.numTracks;
              console.log("recieved"+filename+ " & info from app.js");
              var tableRow = '<tr id='+filename+'><td><a href="/uploads/'+filename+'" download="'+filename+'"</a>'+filename+'</td><td>'+ver+'</td><td>'+cre+'</td><td>'+wpt+'</td><td>'+rte+'</td><td>'+trk+'</td></tr>';
              $('#t0').append(tableRow);
              console.log(data);
          },
          fail: function(error) {
              // Non-200 return, do something with error
              $('#filleeee').html("Invalid file uploaded");
              console.log(error);
          }
      });
       //e.preventDefault();
       //});

    });

    $('#rename-btn').click(function() {
        $('#route-id').html("newname");
    //$('#t0').append('<tr id="filename"><td><a href="/uploads/simple.gpx" download="simple">simple.gpx</a></td><td>$ver</td><td>cre</td><td>wpt</td><td>rte</td><td>trk</td></tr>');
    });

    $('#gpxViewing').submit(function(e){
        $('#create-gpx').html("Selected to view: "+$('#newGpx').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything
            url: '/gpx-new',   //The server endpoint we are connecting to
            data: {
                filename: $('#gpxFilenew'), // $textbox value forfilename //
                creator: $('#gpxCreatornew')  // $text value for creator //
            },
            success: function (data) {
                /*  Do something with returned object
                    Note that what we get is an object, not a string,
                    so we do not need to parse it on the server.
                    JavaScript really does handle JSONs seamlessly
                */
                $('#create-gpx').html("received string: '"+data.filename+"' from textbox in server");
                //We write the object to the console to show that the request was successful
                console.log(data);

            },
            fail: function(error) {
                // Non-200 return, do something with error
                $('#blah').html("On page load, received error from server");
                console.log(error);
            }
        });
    });

    //test input
    function someFnc(fileName, index) {
    var drop = document.getElementById('gpx-file');
    var name = "";
    var value = "";
    var total = "";
        $.ajax({
          type: 'get',
          dataType: 'JSON',
          url: '/testendpoint',
          async: false,
          data: {
            name1: "heyooo"
          },
          success: function(data){
            data.forEach(function(elem){
              console.log(elem.content);
              var someitem = document.createElement('someitemtype');
              option.test = elem.content
              drop.append(option);
            });
          }
            //Create an object for connecting to another waypoint
        });
    }

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    $('#someform').submit(function(e){
        $('#blah').html("Form has data: "+$('#entryBox').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });
});
