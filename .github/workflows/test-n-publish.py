name: test-n-publish

on: [push, pull_request]

jobs:
  test-n-publish:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        python-version: [3.5, 3.6, 3.7, 3.8]
    
    steps:
    - uses: actions/checkout@v2
    - uses: actions/setup-python@v2
      name: Set up Python ${{ matrix.python-version }}
      with:
        python-version: ${{ matrix.python-version }}
    - name: Pre-Install
      run: |
        git clone https://github.com/RRZE-HPC/likwid.git
        cd likwid
        make && sudo make install
        sudo modprobe msr
    - name: Install
      run: |
        python -m pip install --upgrade pip
        python setup.py build_ext
        python -m pip install -e .
    - name: Test
      run: |
        export LD_LIBRARY_PATH=/usr/local:$LD_LIBRARY_PATH
        python -c "import pylikwid"
        ./tests/testtopo.py
        ./tests/testpin.py
        ./tests/testaffinity.py
    - name: Build package
      run: |
        python setup.py build sdist
    - name: Publish to PyPI
      if: github.event_name == 'push' && startsWith(github.ref, 'refs/tags')
      uses: pypa/gh-action-pypi-publish@master
      with:
        skip_existing: true
        user: __token__
        password: ${{ secrets.pypi_password }}
