const confparser = require('./confparser.js');

confparser.getConfigArray();

const APIUri = confparser.getConfigValue('APIUri');
const signinEndpoint = APIUri + confparser.getConfigValue('signinEndpoint');
const accountInfoEndpoint = APIUri + confparser.getConfigValue('accountInfoEndpoint');
const serverListEndpoint = APIUri + confparser.getConfigValue('serverListEndpoint');
const updatesEndpoint = APIUri + confparser.getConfigValue('updatesEndpoint');

module.exports = {signinEndpoint, accountInfoEndpoint, updatesEndpoint, serverListEndpoint};