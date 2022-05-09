{
  "targets": [
    {
      "target_name": "oVPNLib",
      "sources": [ "nodeapi/includes/ovpn/core/client/ovpncli_wrap.cxx" ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "include_dirs": [
        "nodeapi/includes/ovpn/core"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    }
  ]
}