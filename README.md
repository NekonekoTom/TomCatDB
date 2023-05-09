# **Overview**
**TomCatDB is a simple K-V database based on LSM-Tree.**

# **Features**
- Simple interface. The TomCatDB includes simple interfaces, such as `Insert`, `Delete`, `Get` and `ContainsKey`, provided by `class TCDB`. For more details, see [TomCatDB-doc]().
- Clear structure and good coding style. This project mostly follows [Google C++ style](https://google.github.io/styleguide/cppguide.html), and has detailed comments. All modules are well organized that you can replace any of them without making significant changes to the project.
- Easy to use. The users can run a database by specifying a `Config` object and then create a `TCDB` instance by passing the object as the construction function's parameter.

# **Limitations**
- The TomCatDB works only on Linux.
- Multithread operations are not supported yet for the current version.

# **Getting the source**
Get the debug version of TomCatDB by running command:
```
git clone -b debug https://github.com/NekonekoTom/TomCatDB.git
```

# **Building**
TomCatDB can be build through `makefile`:
```
cd TomCatDB
make
```
Then you can run `main.out` for test.

TomCatDB provides two build pipelines.
- Build using CMake script (recommended)

  Run the bash script `build_debug.sh`.
  The script will create a directory `build`.
  If `build` already exists, it will be removed by the script.
  All modules are compiled to static library `.a` files, and linked with the `main.cc`.

- Build from `makefile`
  - Rename the `old_makefile.txt` to makefile;
  - Rename all `.cc` source files to `.cpp` by running the Python script in `./src`:
    ```
    cd ./src
    python rename_postfix.py .cc .cpp -r
    ```
    Or you can modify the `old_makefile.txt` by replacing all `.cpp` with `.cc` without renaming the source files.
  - Move main.cpp or main.cc to the root directory;
  - Run command `make` under the root directory. All intermediate files will NOT be reserved by default.

# **Performance**
TODO: Untested.

# **Others**
## TODO list
-[x] Build by CMake instead of makefile.

-[ ] Bloom filter.

-[ ] Multithread access.

-[ ] More functions for TCLogger.

-[ ] LRU Cache.