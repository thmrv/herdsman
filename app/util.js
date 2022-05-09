const geoip = require('geoip-country');
const notifier = require('node-notifier');
const path = require('path');
let $ = require( "jquery" );
let currentAppStateInstruction,
    currentAppState,
    geo,
    active;

let fsWin = require('fswin');
let winVersionString = require('child_process').execSync('ver').toString().trim();
let powerShellRequired = false;
const connection = require("./connection.js");
const endpoints = require("./endpoints.js");
const settings = require("./settings.js");

if (process.platform === "windows" && winVersionString && !winVersionString.indexOf("windows 7") ) {
    powerShellRequired = true;
    notifier.notify(
        {
            title: 'VPN',
            message: 'Для корректной работы необходим PowerShell. Вы также можете указать необходимые политики вручную.',
            wait: true // Wait with callback, until user action is taken against notification, does not apply to Windows Toasters as they always wait or notify-send as it does not support the wait option
        },
    );
}

function APIRequest(endpoint, method, payload, requestHeader = []){
    const xhr = new XMLHttpRequest();
    xhr.withCredentials = true;
    let responseData;
    xhr.open(method, endpoint + payload, true);
    if (requestHeader){
        xhr.setRequestHeader(requestHeader[0], requestHeader[1]);
    }
    xhr.send();
    xhr.onreadystatechange = function() {
        if (xhr.readyState === 4) {
            responseData = $.parseJSON(xhr.response);
        }
    }
    return responseData;
}

function activityChecker(){
    switch (connection.setStatus()) {
        case true:
            active = true;
            break;
        case false:
            active = false;
            break;
        default:
            active = false;
            break;
    }
}

function geoIPLookup(){
    geo = geoip.lookup(regionAddress);
}

function configInit() {
    switch (settings.allowConfigEdit) {
        case true:
            fsWin.setAttributesSync('conf.json', { IS_HIDDEN: false });
            break;
        case false:
            fsWin.setAttributesSync('conf.json', { IS_HIDDEN: true });
            break;
    }
    switch (settings.hideConfigDirs) {
        case true:
            fsWin.setAttributesSync(settings.ipsecConfDir, { IS_HIDDEN: true });
            fsWin.setAttributesSync(settings.oVPNConfDir, { IS_HIDDEN: true });
            break;
        case false:
            fsWin.setAttributesSync(settings.ipsecConfDir, { IS_HIDDEN: false });
            fsWin.setAttributesSync(settings.oVPNConfDir, { IS_HIDDEN: false });
            break;
    }
}

function uiVarsInit() {
    switch (currentAppStateInstruction) {
        case -1:
            currentAppStateInstruction = 'Войти';
            break;
        case 0:
            currentAppStateInstruction = 'Включить';
            break;
        case 1:
            currentAppStateInstruction = 'Выключить';
            break;
        default:
            currentAppStateInstruction = 'Включить';
            break;
    }
    switch (currentAppState) {
        case 0:
            currentAppState = 'Отключено';
            break;
        case 1:
            currentAppState = 'Подключено';
            break;
        default:
            currentAppState = 'Отключено';
            break;
    }
}

uiVarsInit();
configInit();
activityChecker();

module.exports = {APIRequest,powerShellRequired,uiVarsInit,currentAppState,active,activityChecker,configInit,currentAppStateInstruction,geo};

