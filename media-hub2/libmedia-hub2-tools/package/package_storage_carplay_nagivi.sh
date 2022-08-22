rev=`svn info | grep Revision | awk '{if( NR == 1 ){print $2}}'`

mkdir build
cd build
mkdir libmedia-hub2
cd libmedia-hub2
cmake ../../../../libmedia-hub2 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
make install DESTDIR=$SDKTARGETSYSROOT
make install DESTDIR=../../package
cd ..

mkdir libmedia-hub2-plugins
cd libmedia-hub2-plugins

mkdir -p sdk/AccessorySDK
cd sdk/AccessorySDK
cmake ../../../../../../libmedia-hub2-plugins/carplay/sdk/AccessorySDK -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
make install DESTDIR=$SDKTARGETSYSROOT
make install DESTDIR=../../../../package 
cd ..

mkdir Support
cd Support
cmake ../../../../../../libmedia-hub2-plugins/carplay/sdk/Support -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DNAGIVI=True  -DSUPPORT_WIRELESS_CARPLAY=True
#cmake ../../../../../../libmedia-hub2-plugins/carplay/sdk/Support -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DNAGIVI=True
make install DESTDIR=$SDKTARGETSYSROOT
make install DESTDIR=../../../../package 
cd ..

cmake ../../../../../libmedia-hub2-plugins/carplay/sdk/ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
make install DESTDIR=$SDKTARGETSYSROOT
make install DESTDIR=../../../package
cd ..

cmake ../../../../libmedia-hub2-plugins -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DPLUGIN_STORAGE=True -DPLUGIN_IDEV=True -DPLUGIN_CARPLAY=True -DNAGIVI=True
make install DESTDIR=$SDKTARGETSYSROOT
make install DESTDIR=../../package
cd ..

mkdir libmedia-hub2-ipc
cd libmedia-hub2-ipc
cmake ../../../../libmedia-hub2-ipc -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
make install DESTDIR=$SDKTARGETSYSROOT
make install DESTDIR=../../package
cd ..

mkdir libmedia-hub2-misc
cd libmedia-hub2-misc
cmake ../../../../libmedia-hub2-misc -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
make install DESTDIR=$SDKTARGETSYSROOT
make install DESTDIR=../../package
cd ..

#mkdir libmedia-hub2-sample
#cd libmedia-hub2-sample
#cmake ../../../../libmedia-hub2-sample -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DUSE_MH2_IPC=True
#make install DESTDIR=$SDKTARGETSYSROOT
#make install DESTDIR=../../package
#cd ..

cp ../../../LICENSE ../package
cd ../package/usr/include
rm `ls | grep -v mh_api.h | grep -v mh_carplay.h` -rf
cd ../..
tar -czf libmedia-hub2-0.$rev.tar.gz *
mv libmedia-hub2-0.$rev.tar.gz ../

cd ..
rm package -fr
rm build -fr
