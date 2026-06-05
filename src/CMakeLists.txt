# aux_source_directory(. SRC_LIST)
set (SRC_LIST
    rpcprovider.cc
    mprpcapplication.cc
    mprpcconfig.cc
    rpcheader.pb.cc
    mprpcchannel.cc
    mprpccontroller.cc
    mprpczookeeper.cc
)
add_library(mprpc ${SRC_LIST})
target_compile_definitions(mprpc PRIVATE THREADED) 
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)