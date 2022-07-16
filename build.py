import pathlib
import subprocess


if __name__ == "__main__":
    dir = pathlib.Path('.')
    name = dir.absolute().name
    cpp = [x.absolute() for x in dir.joinpath('src').glob('**/*.cpp')]
    c = ['C:\\src\\volk\\volk.c']
    dir.joinpath('build').mkdir(exist_ok=True)
    out = ['-o', dir.joinpath('build', name + '_mingw64.exe').absolute()]
    options = [
        '-std=c++20',
        '-Wall',
        '-Wextra',
        '-pedantic',
        '-O3'
    ]
    includes = [
        "-IC:\\src\\fmt-8.1.1\\include",
        '-IC:\\src\\VulkanSDK\\Include',
        '-IC:\\src\\glm',
        '-IC:\\src\\tinyobjloader',
        '-IC:\\src\\volk'
    ]
    link = []
    subprocess.run(['g++'] + options + includes + cpp + c + link + out)
