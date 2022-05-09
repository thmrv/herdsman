const confparser = require("./confparser");
const { app, ipcMain, TrayWindow, Tray, BrowserWindow, Menu } = require("electron");
const remote = require('electron').remote;
const path = require("path");
let $ = require( "jquery" );
let fsWin = require('fswin');
const positioner = require('electron-traywindow-positioner');
let geo = require("./util.js");
const notifier = require('node-notifier');
const connection = require("./connection.js");
const util = require("./util.js");
const endpoints = require("./endpoints.js");
const settings = require("./settings.js");
const systemCall = require('nativevpncli/ras.js');

const regionCountry = 'Не определено';
const regionPing = 'Не определено';
const regionAddress = '0.0.0.0';
const regionPort = '1194';
const ping = 0;
let win;

try {
    app.on("ready", () => {
        let winVisibilityState = 'visible';
        let tray = null;
        tray = new Tray('static/files/ico/iconsm.png');
        tray.setToolTip('VPNcli');

        win = new BrowserWindow({ width: 240, height: 380, frame: false, show: false,
            icon: 'static/files/ico/appico.png',
            skipTaskbar: true,
            webPreferences:
                {nodeIntegration: true,},});
        win.setResizable(false)
        util.activityChecker();
        util.configInit();
        util.uiVarsInit();
        const contextMenu = Menu.buildFromTemplate([
            { label: util.currentAppStateInstruction},
            { label: util.currentAppState, enabled: false },
            { label: 'Выход', role: 'quit'}
        ])

        function reposition(){
            positioner.calculate(win, tray.getBounds());
            positioner.position(win, tray.getBounds());
        }

        reposition();

        //контекстное меню + позиция окна
        tray.setContextMenu(contextMenu);
            win.loadFile('static/traywin.html').then(r => {return true});
            win.show();
        //ивенты для ico трея
        tray.on('click', () => {
            switch (winVisibilityState){
                case 'visible':
                    win.hide();
                    winVisibilityState = 'hidden';
                    reposition();
                    break;
                case 'hidden':
                    win.show();
                    winVisibilityState = 'visible';
                    reposition();
                    break;
            }
        });

    });
} catch (e) {
    console.error(e);
    app.exit(0);
}

//system-wide тема
ipcMain.handle('dark-mode:toggle', () => {
    if (nativeTheme.shouldUseDarkColors) {
        nativeTheme.themeSource = 'light'
    } else {
        nativeTheme.themeSource = 'dark'
    }
    return nativeTheme.shouldUseDarkColors
})

ipcMain.handle('dark-mode:system', () => {
    nativeTheme.themeSource = 'system'
})

//
app.on("ready", () => {
    notifier.notify(
        {
            title: 'BlyaTop',
            message: 'VPN: ' + util.currentAppState + '',
            wait: true
        },
    );
})

function init(){
    confparser.getConfigArray();
    connection.getAuth();
    connection.setStatus();
    connection.getServersList();
    connection.getServerInfo(1);
}


