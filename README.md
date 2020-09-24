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
ctest
```

build/bin/executor_runner - app runner
build/lib/libexecutor.a - static lib for tests
build/bin/tests_runner - tests runner

# Notes 
For VSCode you can take a look at `.vscode` and `.devcontainer` folders`