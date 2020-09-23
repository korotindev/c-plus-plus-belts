# Requirenments 
  - Docker
# Run
```
docker-compose build
docker-compose run --rm app bash
mkdir -p ./build
cd ./build
cmake ..
cmake --build . -j $(nproc)
cmake --build . -t test

```

Take a look at `CMakeLists.txt` for `make run` implementation

# Notes 

For VSCode you can take a look at `.vscode` and `.devcontainer` folders`