import pathlib
import subprocess


if __name__ == "__main__":
    # Basic project setup
    dir = pathlib.Path('.')
    name = dir.absolute().name
    
    # Gather source files
    cpp = [str(x.absolute()) for x in dir.joinpath('src').glob('**/*.cpp')]
    
    # Ensure directories
    dir.joinpath('build').mkdir(exist_ok=True)
    
    # Clang compile 
    clangOut = ['-o', dir.joinpath('build', name + '_clang64.exe').absolute()]
    gnuOptions = [
        '-std=c++20',
        '-Wall',
        '-Wextra',
        '-pedantic',
        '-O2'
    ]
    gnuIncludes = [
        '-IC:\\src\\fmt-8.1.1\\include',
        '-IC:\\src\\VulkanSDK\\Include',
        '-IC:\\src\\glm',
        '-IC:\\src\\tinyobjloader',
        '-IC:\\src\\volk'
    ]
    subprocess.run(['clang++'] + gnuOptions + gnuIncludes + cpp + clangOut)
    
    # MSVC compile
    msvcOptions = [
        '/EHsc',
        '/std:c++latest',
        '/options:strict',
        # '/Zi',
        # '/O1',
        '/O2',
    ]
    msvcIncludes = [
        '/IC:\\src\\fmt-8.1.1\\include',
        '/IC:\\src\\VulkanSDK\\Include',
        '/IC:\\src\\glm',
        '/IC:\\src\\tinyobjloader',
        '/IC:\\src\\volk'
    ]
    dir.joinpath('build/msvc').mkdir(exist_ok=True)
    commandFile = pathlib.Path('./build/msvc/command.bat')
    with commandFile.open(mode='w', encoding='cp850') as bat:
        bat.write('call "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat"\n\n')
        command = 'cl /c /Fo' + str(dir.joinpath('build/msvc').absolute()) + '/ '
        for option in msvcOptions:
            command += option + ' '
        for include in msvcIncludes:
            command += include + ' '
        for file in cpp:
            command += file + ' '
        bat.write(command + '\n\n')
        command = 'link /OUT:build/' + name + '_msvc64.exe /DEFAULTLIB:User32 build/msvc/volk.obj build/msvc/main.obj'
        bat.write(command + '\n\n')
        bat.close()
    subprocess.run(commandFile.absolute(), check=True, capture_output=False)
    commandFile.unlink()
    
    # GCC compile
    gccOut   = ['-o', dir.joinpath('build', name + '_mingw64.exe').absolute()]
    subprocess.run(['g++'] + gnuOptions + gnuIncludes + cpp + gccOut)
    
    # compile shaders
    shaders = [x.name for x in dir.joinpath('src/shaders').glob('**/*.frag')] + [x.name for x in dir.joinpath('src/shaders').glob('**/*.vert')]
    dir.joinpath('build/shaders').mkdir(exist_ok=True)
    for shader in shaders:
        command = ['C:\\src\\VulkanSDK\\Bin\\glslangValidator.exe', '-V', 'src/shaders/' + shader, '-o', 'build/shaders/' + shader + '.spv']
        subprocess.run(command, check=True)
