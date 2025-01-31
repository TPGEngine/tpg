#Steps to run test


Download cmake

```
sudo apt install cmake
```

Start at base directory

```
mkdir build
cd build
cmake .. && make
ctest --rerun-failed --output-on-failure
``` 

