{
  "targets": [
    {
      "target_name": "module",
      "dependencies": [],
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
        "./src/napi_helpers.c"
      ],
      "include_dirs": [
        "<!(pkg-config --cflags purple | cut -c 3-)",
        "./src",
        "./src/bindings"
      ],
      "libraries": [
        "<!(pkg-config --libs purple)"
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [
        "module"
      ],
      "copies": [
        {
          "files": [
            "<(PRODUCT_DIR)/module.node"
          ],
          "destination": "./lib/"
        }
      ]
    }
  ]
}
