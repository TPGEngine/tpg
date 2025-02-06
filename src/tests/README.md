#Steps to run test


Download cmake

```
sudo apt install cmake
```

Start at base directory && run tests in build directory

```
cd build
make && ctest --rerun-failed --output-on-failure
``` 

