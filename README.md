# carplay

See 'auth.c' for example Apple authentication via i2c

### Build mDNSResponder-567

```
cd mDNSResponder-567
make os=linux -C "mDNSPosix"
sudo make install os=linux -C "mDNSPosix"
```
How to start the mDNSResponder? Before starting mdns, copy mdnsd.conf and mdnsd-services.conf to /etc folder.(don't change name, those are hardcoded in code)

Note: Update the files mdnsd.conf and mdnsd-services.conf according to your requirement.
Refer Services.txt file for more info on how to create services records file.
```
sudo service mdns start
-or-
sudo /etc/init.d/mdns start
```

If things are not working then follow either one of the below method,

Run mdnsd -in debug mode mdnsd -debug

Enabel debug option in Makefile(mDNSResponder/mDNSPosix/MakeFile) set the debug variable to 1

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
