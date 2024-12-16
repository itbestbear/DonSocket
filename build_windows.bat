echo off

REM 声明采用UTF-8编码
chcp 65001
echo 武林至尊,宝刀屠龙!号令天下，莫敢不从！倚天不出，谁与争锋?

echo 'start cmake in windows'

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build/vcvarsall.bat" x86

rd /s /q build
mkdir build

cd  build

cmake .. -G "NMake Makefiles"

nmake

echo 'end all'

cd ../

