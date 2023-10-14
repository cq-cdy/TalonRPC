SERVER:监听客户端的读事件，把读过来的字符流解析成TinyPBProtocol对象
TinyPBProtocol中的m_pb_data字段，存放的是客户端把像请求调用的rpc服务.方法名
字符传，然后，在服务端实例化的google::protobuf::Service（用户重写的父类）对象
调用方法。   
google::protobuf::Message *rsp_msg = service->GetResponsePrototype(method).New();
service->CallMethod(method, rpc_controller, req_msg, rsp_msg, nullptr);
把方法返回的结果到rsp_msg（在proto文件定义的返回类型的父类）对象，然后把对象
序列化到TinyPBProtocol中的m_pb_data字段，在把TinyPBProtocol序列化成字符流通过tcp
发送回客户端，客户端先把TinyPBProtocol解码，然后在将TinyPBProtocol中的m_pb_data字段
反序列化成response 类型就能拿到了。
       