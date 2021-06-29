{
  "targets": [
    {
      "target_name": "node-purple",
      "dependencies": [],
      "sources": [
        "./src/*.h",
        "./src/bindings/*.c"
      ],
      "include_dirs": [
        "./src/",
        "./src/bindings",
        "<!(pkg-config --cflags purple | cut -c 3-)"
      ],
      "libraries": [
        "<!(pkg-config --libs purple)"
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [
        "node-purple"
      ],
      "copies": [
        {
          "files": [
            "<(PRODUCT_DIR)/node-purple.node"
          ],
          "destination": "./lib/"
        }
      ]
    }
  ]
}
