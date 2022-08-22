rev=`svn info | grep Revision | awk '{if( NR == 1 ){print $2}}'`

mkdir build
cd build
mkdir libmedia-hub2
cd libmedia-hub2
cmake ../../../../libmedia-hub2 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
make install DESTDIR=../../package
cd ..

mkdir libmedia-hub2-plugins
cd libmedia-hub2-plugins
cmake ../../../../libmedia-hub2-plugins -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
make install DESTDIR=../../package
cd ..

mkdir libmedia-hub2-ipc
cd libmedia-hub2-ipc
cmake ../../../../libmedia-hub2-ipc -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
make install DESTDIR=../../package
cd ..

mkdir libmedia-hub2-misc
cd libmedia-hub2-misc
cmake ../../../../libmedia-hub2-misc -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr
make install DESTDIR=../../package
cd ..

cp ../../../LICENSE ../package
cd ../package/usr/include
rm `ls | grep -v mh_api.h` -f
cd ../..
tar -czf libmedia-hub2-0.$rev.tar.gz *
mv libmedia-hub2-0.$rev.tar.gz ../

cd ..
rm package -fr
rm build -fr

