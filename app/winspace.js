const path = require("path");
let $ = require( "jquery" );
let fsWin = require('fswin');

const connection = require("./connection.js");
const util = require("./util.js");
const endpoints = require("./endpoints.js");
const settings = require("./settings.js");

function flushContent() {
    $('.regionAddress').text(regionAddress + ':' + regionPort);
    $('.appState').text(geo);
    if (!util.active){
        $('body').removeClass('inactive');
    }else{
        $('body').addClass('inactive');
    }
}

function signIn() {

}

function serverList() {

}

function throwStateAnim() {

}