const confparser = require('./confparser.js');

confparser.getConfigArray();

const allowConfigEdit = confparser.getConfigValue('allowConfigEdit');
const hideConfigDirs = confparser.getConfigValue('hideConfigDirs');
const oVPNConfDir = confparser.getConfigValue('oVPNConfDir');
const ipsecConfDir = confparser.getConfigValue('ipsecConfDir');
const allowDebug = confparser.getConfigValue('allowDebug');

module.exports = {allowConfigEdit,hideConfigDirs,ipsecConfDir,allowDebug,oVPNConfDir};

