{
  "targets": [
    {
      "target_name": "cpuinfo",
      "sources": ["src/cpuinfo.cpp"],
      "include_dirs": [ "<!@(node -p \"require('node-addon-api').include\")" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
    }
  ]
}