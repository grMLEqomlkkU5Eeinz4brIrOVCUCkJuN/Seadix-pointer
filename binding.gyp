{
  "targets": [
	{
	  "target_name": "seadix",
	  "cflags!": [ "-fno-exceptions" ],
	  "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "src/SeaDix.cc", "src/RadixTrie.cc", "src/RadixTrieNode.cc" ],
	  "include_dirs": [
		"<!@(node -p \"require('node-addon-api').include\")",
		"<(module_root_dir)"
	  ],
	  "libraries": [],
	  "dependencies": [
		"<!(node -p \"require('node-addon-api').gyp\")"
	  ],
	  "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
	}
  ]
}
