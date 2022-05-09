const PowerShell = require ( 'node-powershell' );
const confparser = require("./confparser");

const util = require("./util.js");
var VPNConnNameModifier;
var $ = require( "jquery" );

let VPNRemoteAddress, VPNUsername,
    VPNPwd, certificateString, certificateFile;
let VPNName = "BlyaTop";
let status = 0;
let authToken = "";

const systemCall = require("../nativevpncli/ras.js");

function checkForUpdates(){
    //...
}

function getRandomInt(max) {
    return Math.floor(Math.random() * max);
}

let VPNconnectionNameModifier = getRandomInt(31);

function getServerInfo(serverID){
    let token = getToken();
    let requestHeader = [
        'Authorization',
        'Bearer ' + token
    ];
    const serverInfo = util.APIRequest(endpoints.serverListEndpoint + serverID, 'GET', '', requestHeader);
    return serverInfo;
}

function getServersList(){
    let token = getToken();
    let requestHeader = [
        'Authorization',
        'Bearer ' + token
    ];
    const serverList = util.APIRequest(endpoints.serverListEndpoint, 'GET', '', requestHeader);
    return serverList;
}

function checkStatus(){
    return status;
}

function setWindowsPolicy(){
    PowerShell.$`Set-VpnConnectionIPsecConfiguration -ConnectionName "BlyaTop" -AuthenticationTransformConstants None -CipherTransformConstants AES256 -EncryptionMethod AES256 -IntegrityCheckMethod SHA384 -PfsGroup None -DHGroup ECP384 -PassThru -Force`;
}

function executeVPNConnection() {
    try {
        systemCall.AddVPNLink(
            VPNName,
            VPNRemoteAddress,
            VPNUsername,
            VPNPwd);
        //setWindowsPolicy();
        systemCall.StartVPNLink(VPNName);
        setStatus(true);
        return true;
    }catch (e){
        console.log(e);
        setStatus(false);
        return false;
    }
}

function setCertificate(){

}

function dropVPNConnection() {
        systemCall.StopVPNLink(VPNName);
        systemCall.DestroyVPNLink(VPNName);
        setStatus(false);
        return true;
}

function getAuth(){
    const login = "paid@example.com";
    const password = "12345678";
        try {
            const authData = util.APIRequest(endpoints.signinEndpoint, 'POST', '?email=' + login + '&password=' + password);
            saveToken(authData.token);
        }catch (e){
            console.log('API Request Failed');
        }
    }

function setStatus(){
    switch (status){
        case false:
            util.currentAppState = 0;
            util.uiVarsInit();
        case true:
            util.currentAppState = 1;
            util.uiVarsInit();
    }
}

function breakAuth(){

}

function saveToken(tokenStr){
    let obj = confparser.getConfigObj();
    obj.token = tokenStr;
    confparser.saveConfig(obj);
    return true;
}

function getToken(){
    let obj = confparser.getConfigObj();
    return obj.token;
}

module.exports = {getServerInfo, getServersList, getAuth, setStatus,checkForUpdates,executeVPNConnection,dropVPNConnection};
