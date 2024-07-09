# VPN client for easy deployment in numbers with IPsec and oVPN support

![herdsman](https://github.com/thmrv/herdsman/blob/main/logo.svg?raw=true)

> Requires further integration
with the existing API, however
, both endpoints and different methods of interaction are already provided,
such as an active connection to a vpn server is possible
through a request for a config file from a specific
api endpoint, or by requesting 3 files separately in json format or perhaps
any other suitable one.

## Quick start

*The finished build simulating the client's work is located in the build folder of the root.*

In case of any changes, 
to compile and run it, just enter the following commands:

```
npm install
npm rebuild
npm start
``` 

For a build in .exe, you can use the electron-packager and the following command:

```
electron-packager <source code folder> <vpncli> --platform=win32 --arch=x64
``` 

The arguments are taken for windowsx64, individual ones will be required for other platforms, more details [here](https://github.com/electron-userland/electron-packager ).
Of course, all dependencies are already registered in the package.json
and their localized versions in package-lock.json

## Stack and the principle of operation

- nodejs *the head of the application from which all its component parts are called*
- electron *for rendering and working with the interface*
- node-gyp/node-api *as a layer between cpp and js*
- lottie *is used for vector animations*

All libraries, including both shared and static ones, are selected
so that there is always the possibility of compiling for win, linux and macos. 
*The application is cross-platform initially.*

To work with ovpn, the node api is used as an abstraction layer between ovpn and node, all methods are called
directly through the linked library and without using network protocols 
+ allows you to freely update node and its modules. To work with ipsec (which includes ikev2 as an additional protection method)
, a native vpn client is used (each os has its own, respectively).

## Working with the API, connections

VPN connections are initiated through configs.ovpn and .p12 (optional). Configs are obtained by a get request
to one of the endpoints specified in the application configuration file of the type '../api/v1/ovpn' and downloading the desired file(s)
to a dedicated hidden folder, by default it is expected that in the request to the endpoint 
it will return the vpn config file immediately in the desired format, however, there is a commented
-out functionality for requesting the necessary data separately in json,
without using config files at all for connections. 
 
> Any interaction with vpn connections is possible if you are logged into a valid account. VPN configs are requested by default only if the directory with these configs (separately for each protocol) is empty and the user has logged in to the account.
There is also a check for the conditional connected tariff of the account (by default it is commented out, but there is an opportunity to give the privilege in accordance with the connected tariff to the user).

The account is logged in via a separate api endpoint like '../api/v1/signin'. Login to the account can be done in two ways:

1. request for the endpoint of the login + password pair in json format (preferably work only in json), either true or false is expected in response
2. through a separate login web page
 
The second option may be more successful in terms of security, 
but both can be implemented functionally and there is a base for each of the options.
 
VPN connections themselves are carried out through the ipsec and ovpn3 libraries, 
both are taken from open source sources with the appropriate license, 
prohibiting their use and modification, including for commercial purposes.
 
## Interface
  
The application works through the system tray, the interface is provided with:
  
- tray icon, the main element
- right-click context menu (functional) with the following items:
- current status (connected/not connected)
  - activity depending on the current state (connect, disconnect), if the user has not logged in to the account, the item will be disabled
- log in to the account (either redirect to the login web page, or open a modal window with login, or a hybrid of the last two)
- exit the application (app.exit (0))
    
- the graphical interface (without the system window frames), called by left click, includes 
  - status indication, there are two of them (either connected or not)
- address and port of the connected connection (updated every time when restarting)
  - connection country (detected by IP address)
- arrow to close (hide) the active window
    
- modal window for account login (optional)
  
The graphical part of the UI provides some complex animations, they are completely vector and works on lottie 
(they do not require an active network connection, all files are hosted locally).
  
## Config
   
The config exists to simplify the work, by default it is hidden for the average user. The file has the following json structure:
   
 ```
{
  "allowConfigEdit": true,
  "oVPNConfDir": "ovpn/",
  "ikev2ConfDir": "p12/",
  "hideConfigDirs": true,
  "allowDebug": false,
  "OVPNConfEndpoint": "localhost/api/v1/config/ovpn",
  "Ikev2ConfEndpoint": "localhost/api/v1/config/ikev2",
  "signinEndpoint": "localhost/api/v1/signin",
  "accountInfoEndpoint": "localhost/api/v1/account",
  "updatesEndpoint": "localhost/api/v1/updates"
  ...
}
```

It contains both purely utilitarian parameters, such as api endpoints,
which the client will access during direct operation. If the endpoints are not specified, or they need to be updated, the client will update the config file from the corresponding repository (or any uri).

The application has a mini update manager - it checks for updates by referring to the corresponding endpoint, which should return either true or false. If the response is true, the manager
will automatically update the files from the specified application repository, regardless of whether there will actually be changes or not.
