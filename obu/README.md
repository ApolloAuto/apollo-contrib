1. Introduction
These pieces of code under this folder is for OBU box.

2. How to compile and run
Follow the step below to generate a binary for OBU.
a. Install cross compilation chain, we may get it from the OBU vendor. 
b. Install gRPC, glog, gflags and protobuf on the host PC and copy their include folder into third_party/ directory. 
--Get gRPC from https://github.com/grpc/grpc/releases/tag/v1.14.0
--Get Protobuf from https://github.com/protocolbuffers/protobuf/releases/tag/v2.6.1
--Get glog from https://github.com/google/glog/releases/tag/v0.3.3
--Get gflags from https://github.com/gflags/gflags/releases/tag/v2.2.1
c. Download the so libraries from https://apollo-v2x.bj.bcebos.com/obu-library.tar.gz and decompress it into obu folder. 
d. Run the command "bash build.sh build", it will generate the v2x.tar.gz in the output folder.
e. Download and decompress the v2x.tar.gz to the OBU box. Run "./v2x --flagfile=v2x.flag". 

3. How to integrate to a new OBU
For PC5 mode, we define the interfaces between the network layer and application layer. The interfaces are in network/pc5/cv2x_app.h. Also need to provide the decode/encode interfaces for messages to application layer. 
For uU mode, need to define the following variables in the conf/v2x.flag.
auth_url, # server authentication address
uu_server_addr, # server address to get the SPAT and the MAP messages
uu_socket_port, # server port for uu UDP socket
uu_device_id,   # device id which is assigned by the uu platform
uu_device_mac,  # device MAC address for OBU

Also need to implement the interfaces which are defined in network/uu/gb_bsm.h

4. Security for communications

The apollo V2X security is for data security. Asymmetric encryption algorithm SM2 and cryptographic hash algorithm SM3 are used.

The CA format is followed the standard "Intelligent transport - Certificate application interface". More detail is at http://www.mot.gov.cn/yijianzhengji/lishizhengji/201711/P020171101362371804539.pdf.

In asymmetric encryption algorithm, every user has a CA file including private key and public key, it also has another file of root CA public key. User use his private key to sign the payload, send a package including the raw payload, signature and the security info(public key and signature by root CA). The receiver should also has the root CA public key. He would use the root CA public key to verify the validation of received public key then use the received public key to verify the signature.

Usage example

For privacy reason, the generating CA function is not published. You could use the sample file
"ca_cert.pem" for root CA file and "child_cert.pem" for child CA. Or you could write your own
generating function by the format defined in "third_party/security/include/Certificate.h".


