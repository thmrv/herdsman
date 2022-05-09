const fs = require('fs');
const configPath = 'conf.json';
let configArray = [];
let configKeys = [];
let raw;
let $ = require( "jquery" );

function parseConfig() {
    let raw = $.getJSON(configPath, function (result) {
        return JSON.parse(raw);
    });
}
function getConfigValue(getValue) {
        return parseConfig(configPath).getValue;
}
function getConfigArray(){
    for (let i = 0;i<Object.keys(parseConfig()).length;i++){
        configArray[i] = parseConfig().Object.keys(parseConfig())[i];
    }
}
function getConfigObj(){
    return fs.readFileSync(configPath);
}
function saveConfig(jsonData){
   return  fs.writeFileSync(configPath, jsonData);
}

module.exports = {saveConfig, getConfigObj, getConfigValue,parseConfig,getConfigArray,configArray};