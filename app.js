'use strict'

// C library API
const ffi = require('ffi-napi');
let sharedLibC = ffi.Library('./parser/bin/libgpxparser', {
  'uploadToJSON' : ['string', ['string']],
  'validateDoc' : ['string', ['string','string']],
});

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');

    var nameFromPath = "uploads/" + uploadFile.name;
    let uploadInfo = sharedLibC.uploadToJSON(nameFromPath);
    let validUpload = sharedLibC.validateDoc(nameFromPath, "parser/bin/gpx.xsd");

    if (validUpload == "false") {
      console.log('GPX doc is not valid: '+validUpload);
    } else {
      console.log("GPX doc "+uploadFile.name+" is valid!");
      app.get('/fileLogPop', function(req , res){
        let retStr = uploadInfo;
        let newName = "" + uploadFile.name;
        console.log("sending json of "+ uploadFile.name+" to index.js ");
        res.send({
          gpxJSON: retStr,
          nameT: newName
        });
      });
    }
    readUploads('uploads/');
  });
});

//Reading the files in the uploads folder
function readUploads (path) {
  let array = [];
  fs.readdir(Buffer.from(path), (err, files) => {
    if (err) {
      console.log("Read Unsuccessful: "+err.message);
    } else {
      //console.log(files);
      for (let file of files) {
          var nameFromPath = "uploads/" + file;
          let uploadInfo = sharedLibC.uploadToJSON(nameFromPath);
          let validUpload = sharedLibC.validateDoc(nameFromPath, "parser/bin/gpx.xsd");

          console.log(file + " " + array);
          if (validUpload == "false" || (validUpload != "valid gpx: status true" && validUpload != "true")) {
            console.log('GPX doc is not valid: '+validUpload);
          } else {
            //console.log("GPX doc "+file+" is valid!");
            app.get('/fileLogPop', function(req , res){
              let retStr = uploadInfo;
              let newName = "" + file;
              console.log(retStr);
              //console.log("sending "+ file +" json to index.js ");
              res.send({
                gpxJSON: retStr,
                nameT: newName
              });
            });
          }
      }
    }
  });
}

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
      console.log("uploaded");
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//******************** Your code goes here ********************
//let credentials = require('./app.js');

// test endpoint
app.get('/loginInfo', function(req , res){
  let retStr = req.query.usersName;
  let username = req.query.usersName;
  let password = req.query.usersPass;
  let dataName = req.query.dataBase;
  console.log("app.js login: " + retStr);
  // res.send(retStr);
  // });
  
  let databaseProfile = {
    host:       'dursley.socs.uoguelph.ca',
    user:       username,
    password:   password,
    database:   dataName
  };
  
  let status = "";
  //TBD: these will be using the information from the files in the uploads folder
  let populate = "INSERT INTO FILE (file_name, ver, creator) VALUES ('filename.gpx','1.1','me')";
  let popStr = "INSERT INTO FILE (file_name, ver, creator) VALUES ('simple.gpx','1.1','Denis N')";
  let uploadInfo = sharedLibC.uploadToJSON('uploads/simple.gpx');
  let uploadedStr = "INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('simple.gpx', '"+uploadInfo.numRoutes+"', '3')";

  async function main() {

    const mysql = require('mysql2/promise');
    let connection;

    try {
        //creating the connection
        connection = await mysql.createConnection(databaseProfile);
        //creating the tables
        connection.execute("CREATE TABLE if not exists FILE (gpx_id INT NOT NULL auto_increment, file_name VARCHAR(60) NOT NULL, ver DECIMAL(2,1) NOT NULL, creator VARCHAR(256) NOT NULL, primary key(gpx_id) )");
        connection.execute("CREATE TABLE if not exists ROUTE (route_id INT NOT NULL auto_increment, route_name VARCHAR(256), route_len FLOAT(15,7) NOT NULL, gpx_id INT NOT NULL, primary key(route_id))"); //, CONSTRAINT fk_route_table FOREIGN KEY(gpx_id) REFERENCES ROUTE(gpx_id) ON DELETE CASCADE 
        connection.execute("CREATE TABLE if not exists POINT (point_id INT NOT NULL auto_increment, point_index INT NOT NULL, latitude DECIMAL(11,7) NOT NULL, longitude DECIMAL(11,7) NOT NULL, point_name VARCHAR(256), route_id INT NOT NULL, primary key(point_id))"); //, CONSTRAINT fk_point_table FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE 
        //Populating tables
        await connection.execute(populate);
        await connection.execute(popStr);

        const [rows1, fields1] = await connection.execute('SELECT * from `FILE` ORDER BY `gpx_id`');
        console.log("Sorted by file name");
        for (let row of rows1){
          console.log("gpx_id: "+row.gpx_id+" file_name: "+row.file_name+" ver: "+row.ver+" creator: "+row.creator);
        }
        
        status = "successful";
        await connection.execute("DELETE FROM FILE");
        await connection.execute("DELETE FROM ROUTE");
        await connection.execute("DELETE FROM POINT");
    } catch (e) {
        status = "unsuccessful";
        console.log("Login Error: "+e);
    } finally {
        console.log("Finished: connection was "+status);
        if (connection && connection.end) connection.end();
    }
    return status;
  }
  let endstatus = "";
  const status1 = main();
  status1.then((promiseData) => {
    endstatus = promiseData;
    //returning to index.js
    res.send({
      statusDB: endstatus+" app.js<-sql"
    });
  });
  //main();
});

//Sample endpoint
app.get('/endpoint1', function(req , res){
  let retStr = req.query.stuff + " " + req.query.junk;
  console.log("inside app.js");
  res.send({
    stuff: retStr
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
