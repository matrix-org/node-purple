{
  "targets": [
#    {
#      "target_name": "action_before_build",
#      "type": "none",
#      "hard_dependency": 1,
#      "actions": [
#        {
#          "action_name": "unpack_purple_deps",
#          "inputs": ["./deps/download.sh"],
#          "outputs": ["./deps/download.sh"],
#          "action": [
#             "bash", "./deps/download.sh",
#          ]
#        }
#      ],
#    },
    {
      "target_name": "module",
      "dependencies": [
        "action_before_build"
      ],
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
        "deps/pidgin-2.13.0/libpurple",
        "./src/",
        "./src/bindings",
        "<!(pkg-config --cflags glib-2.0 | cut -c 3-)"
      ],
      "libraries": [
        "-Ldeps/libpurple",
        "<!(pkg-config --libs glib-2.0)"
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
          "destination": "./lib/binding/"
        }
      ]
    }
  ]
}
