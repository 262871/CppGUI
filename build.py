import pathlib
import subprocess


if __name__ == "__main__":
    dir = pathlib.Path('.')
    name = dir.absolute().name
    cpp = [x.absolute() for x in dir.joinpath('src').glob('**/*.cpp')]
    dir.joinpath('build').mkdir(exist_ok=True)
    out = ['-o', dir.joinpath('build', name + '_mingw64.exe').absolute()]
    options = [
        '-std=c++20',
        '-Wall',
        '-Wextra',
        '-pedantic',
        '-O3'
    ]
    subprocess.run(['g++'] + options + cpp + out)
