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
```

take a look at `./app/build/bin` folder

# Notes 
For VSCode you can take a look at `.vscode` and `.devcontainer` folders`
