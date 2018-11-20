{
    "targets": [{
        "target_name": "module",
        #"type": "static_library",
        "sources": [
            "./src/module.c",
            "./src/helper.c",
            "./src/eventloop.c",
            "./src/signalling.c",
            "./src/messaging.c",
            "./src/bindings/b_core.c",
            "./src/bindings/b_plugins.c",
            "./src/bindings/b_accounts.c",
            "./src/bindings/b_buddy.c",
            "./src/bindings/b_notify.c",
            "./src/napi_helpers.c",
        ],
        # These are probably ultra-fragile
        'include_dirs': [
           'deps/pidgin-2.13.0/libpurple',
           './src/',
           './src/bindings',
	    '<!(pkg-config --cflags glib-2.0 | cut -c 3-)'
        ],
        "libraries": [
            '-Ldeps/libpurple',
	    '<!(pkg-config --libs glib-2.0)'
        ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "module" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/module.node" ],
          "destination": "./lib/binding/"
        }
      ]
    }]
}
