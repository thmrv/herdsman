{
  'targets': [
    {
      'target_name': 'nativevpncli-native',
      'sources': [ 'src/nativevpncli.cc' ],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")","<(module_root_dir)/src","C:/Program Files (x86)/Windows Kits/10/Include"],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'libraries':["rasapi32.lib"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7'
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      }
    }
  ]
}