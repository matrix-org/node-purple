{
    "targets": [{
        "target_name": "module",
        "sources": [
            "./src/module.c",
            "./src/helper.c",
            "./src/eventloop.c",
            "./src/signalling.c",
            "./src/bindings/b_core.c",
            "./src/bindings/b_plugins.c",
            "./src/bindings/b_accounts.c",
        ],
        # These are probably ultra-fragile
        'include_dirs': [
           '/usr/include/libpurple',
           "/usr/lib/purple-2/",
           '/usr/include/glib-2.0',
           '/usr/lib/x86_64-linux-gnu/glib-2.0/include',
           '/home/will/source/libuv/include',
           './src/',
           './src/bindings'
        ],
        "libraries": [
            "<!@(pkg-config --libs purple)",
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
