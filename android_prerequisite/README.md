#ABOUT
This is the folder to install the Android related eviroment
Run
```
./setup_android_env.sh
```
It will download the Android SDK and NDK into $HOME/android directory.
Then it will set the android-cmake enviroment
At last it will build Boost and Openssl for Android platform

Currently, it works only for arm chip set. 

A) Build the libsrch2-core.so and Srch2SDK.jar

Prerequisite: 
- Install ANT
- Install JDK

Steps:
```
cd ..
source ~/.bashrc
mkdir build
android-cmake .. -DBUILD_SERVER=0
```
Output : 
- libsrch2-core.so in build/libs/armeabi-v7a 
- Srch2SDK.jar in build/src/java/

B) Build srch2 standalone server for android. 

Prerequisite:
- thirdparty depenenencies are built using "thirdparty-build-android.sh" in the thirdparty folder

Steps:
```
cd <trunk_root>
mkdir build
android-cmake -DANDROID_SERVER=1 ..
```
Output:
build/android/bin
