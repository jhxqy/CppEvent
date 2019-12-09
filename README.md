#  一个I/O事件库
> 支持I/O，信号，定时



## 事件种类
#### I/O
    使用select/epoll/kqueue等管理I/O
#### 信号
    能够处理信号事件
#### 定时
    能够使用计时器
    将定时事件放入堆中，每次从堆中取出一个距离当前时间最近的时间事件，以此为限制去执行I/O复用函数，每次epoll结束后，判断是否当前时间超过了堆中函数。如果超过了，则执行并将其从堆中删除
    

## 样例
#### 定时器

```cpp
IoContext ctx; //上下文

Timer t1(ctx); //创建一个Timer，绑定context
t1.ExpiresAfter(chrono::seconds(1));  //设置等待时间，可支持毫秒:chrono::milliseconds
t1.AsyncWait([](){              //加入异步队列
    cout<<"hello world!"<<endl;
});

ctx.Run();  //启动异步。

```

#### 可读事件
```cpp
IoContext ctx; //上下文
ctx.AddEvent(new EventBase(fileno(stdin),EventBaseType::read,[](int fd){
    char buf[1024];
    ssize_t size=read(fd,buf,1024);
    if(size<0){
        cout<<"读取失败！:"<<strerror(errno)<<endl;
        return;
    }
    buf[size]=0;
    cout<<buf;
}));
ctx.Run();

```

#### 可写事件
```cpp
IoContext ctx; //上下文
char buf[1024];
ctx.AddEvent(new EventBase(fileno(stdin),EventBaseType::read,[&](int fd){
    ssize_t size=write(fd,buf,strlen(buf));
    if(size<0){
        cout<<"写出失败！:"<<strerror(errno)<<endl;
    }
    
}));
ctx.Run();

```

#### 信号事件

```cpp
IoContext ctx; //上下文

ctx.AddSignalEvent(SIGINT, [](){
        cout<<"您按下了Control C"<<endl;
})
ctx.Run();

```