# carplay


### how to build for Ubuntu

Only media hub

```
mkdir build
cd build
mkdir libmedia-hub2
cd libmedia-hub2

cmake ../../media-hub2/libmedia-hub2
make 
sudo make install
cd ..

mkdir -p libmedia-hub2-plugins/sdk/AccessorySDK
cd libmedia-hub2-plugins/sdk/AccessorySDK
cmake ../../../../media-hub2/libmedia-hub2-plugins/carplay/sdk/AccessorySDK ..
make 
sudo make install


cd ..
cmake ../../../media-hub2/libmedia-hub2-plugins/carplay/sdk ..
make 
sudo make install

cd ..
cmake ../../media-hub2/libmedia-hub2-plugins ..
make install

mkdir libmedia-hub2-ipc
cd libmedia-hub2-ipc
cmake ../../../media-hub2/libmedia-hub2-ipc ..
make 
sudo make install
cd ..

mkdir libmedia-hub2-misc
cd libmedia-hub2-misc
cmake ../../../media-hub2/libmedia-hub2-misc ..
make 
sudo make install
cd ..

mkdir libmedia-hub2-sample
cd libmedia-hub2-sample
cmake ../../../media-hub2/libmedia-hub2-sample -DUSE_MH2_IPC=True ..
make 

```
