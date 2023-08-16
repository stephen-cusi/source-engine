echo "Setup Build Dependencies"
wget https://dl.google.com/android/repository/android-ndk-r10e-linux-x86_64.zip
unzip ./android-ndk-r10e-linux-x86_64.zip
rm ./android-ndk-r10e-linux-x86_64.zip
export ANDROID_NDK_HOME="./android-ndk-r10e"

wget https://github.com/llvm/llvm-project/releases/download/llvmorg-11.1.0/clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz
tar xf ./clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz
rm ./clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz
export PATH="$PWD/clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04/bin:$PATH"

git clone https://github.com/sourceboxgame/srceng-patcher ./patcher
mkdir -p ./patcher/lib

echo "Build armeabi-v7a"
./waf configure -T release --build-games sourcebox --prefix ./patcher --enable-opus --togles --android armeabi-v7a-hard,host,21 
./waf install

echo "Build aarch64"
./waf configure -T release --build-games sourcebox --prefix ./patcher --enable-opus --togles --android aarch64,host,21 --64bits
./waf install

echo "Patch"
cd ./patcher
chmod +x ./patch.sh
./patch.sh
cd ..

echo "Cleanup Build Dependencies"
rm ./android-ndk-r10e -rf
rm ./clang+llvm-10.1.0-x86_64-linux-gnu-ubuntu-16.04 -rf
