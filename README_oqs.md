# Official compilation of liboqs

## See README.md on https://github.com/open-quantum-safe/liboqs

# Unofficial compilation of liboqs

## Dependencies RedHAT/CentOS/Alma/Rocky 9

```sh
sudo dnf install -y cmake gcc git graphviz libxslt ninja-build openssl-devel unzip
```

## Build liboqs

```sh
git clone https://github.com/open-quantum-safe/liboqs.git
cd liboqs
git switch 0.15.0 --detached
mkdir build
cd build
cmake -GNinja -DBUILD_SHARED_LIBS=ON ..
ninja
```

## Verify

```sh
python3.11 -m venv .venv
. .venv/bin/activate
pip install --upgrade pip
pip install pytest pytest-xdist pyyaml requests
ninja run_test
```


## Install

```sh
sudo ninja install
find /usr/local -name "*oqs*"
```



