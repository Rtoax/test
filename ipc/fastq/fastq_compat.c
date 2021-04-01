/**********************************************************************************************************************\
*  文件： fastq_compat.c
*  介绍： 低时延队列 两套函数的接口 参见 fastq.h _FQ_NAME 定义
*  作者： 荣涛
*  日期：
*       2021年2月2日 创建文件
\**********************************************************************************************************************/



// FastQRing
struct _FQ_NAME(FastQRing) {
    unsigned long src;  //是 1- FASTQ_ID_MAX 的任意值
    unsigned long dst;  //是 1- FASTQ_ID_MAX 的任意值 二者不能重复

#ifdef FASTQ_STATISTICS //统计功能
    atomic64_t nr_enqueue;
    atomic64_t nr_dequeue;
#endif    
    
    unsigned int _size;
    size_t _msg_size;
    char _pad1[64]; //强制对齐，省的 cacheline 64 字节 的限制，下同
    volatile unsigned int _head;
    char _pad2[64];    
    volatile unsigned int _tail;
    char _pad3[64];    
    int _evt_fd; //队列eventfd通知
    char _ring_data[]; //保存实际对象
};


// 从 event fd 查找 ring 的最快方法
static  struct {
    struct _FQ_NAME(FastQRing) *_ring;
} _FQ_NAME(_evtfd_to_ring)[FD_SETSIZE] = {{NULL}};


//从 源module ID 和 目的module ID 到 _ring 最快的方法
struct _FQ_NAME(FastQModule) {
    bool already_register;
#if defined(_FASTQ_EPOLL)
    int epfd; //epoll fd
#elif defined(_FASTQ_SELECT)
    struct {
        int maxfd;
        pthread_rwlock_t rwlock;    //保护 fd_set
        fd_set readset, allset;
        int producer[FD_SETSIZE];
    } selector;
#endif
    unsigned long module_id; //是 1- FASTQ_ID_MAX 的任意值
    unsigned int ring_size; //队列大小，ring 节点数
    unsigned int msg_size; //消息大小， ring 节点大小
    
    char *_file; //调用注册函数的 文件名
    char *_func; //调用注册函数的 函数名
    int _line; //调用注册函数的 文件中的行号
    
    struct _FQ_NAME(FastQRing) **_ring;
};

static struct _FQ_NAME(FastQModule) *_FQ_NAME(_AllModulesRings) = NULL;
static pthread_rwlock_t _FQ_NAME(_AllModulesRingsLock) = PTHREAD_RWLOCK_INITIALIZER; //只在注册时保护使用


/**
 *  FastQ 初始化 函数，初始化 _AllModulesRings 全局变量
 */
static void __attribute__((constructor(105))) _FQ_NAME(__FastQInitCtor) () {
    int i, j;
    LOG_DEBUG("Init _module_src_dst_to_ring.\n");
    _FQ_NAME(_AllModulesRings) = FastQMalloc(sizeof(struct _FQ_NAME(FastQModule))*(FASTQ_ID_MAX+1));
    for(i=0; i<=FASTQ_ID_MAX; i++) {
        _FQ_NAME(_AllModulesRings)[i].already_register = false;
        _FQ_NAME(_AllModulesRings)[i].module_id = i;
    
#if defined(_FASTQ_EPOLL)
        _FQ_NAME(_AllModulesRings)[i].epfd = -1;

#elif defined(_FASTQ_SELECT)

        FD_ZERO(&_FQ_NAME(_AllModulesRings)[i].selector.allset);
        FD_ZERO(&_FQ_NAME(_AllModulesRings)[i].selector.readset);
        _FQ_NAME(_AllModulesRings)[i].selector.maxfd    = 0;

        for(j=0; j<FD_SETSIZE; ++j)
        {
            _FQ_NAME(_AllModulesRings)[i].selector.producer[j] = -1;
        }
        pthread_rwlock_init(&_FQ_NAME(_AllModulesRings)[i].selector.rwlock, NULL);
    
#endif
        _FQ_NAME(_AllModulesRings)[i].ring_size = 0;
        _FQ_NAME(_AllModulesRings)[i].msg_size = 0;
        
        _FQ_NAME(_AllModulesRings)[i]._ring = FastQMalloc(sizeof(struct _FQ_NAME(FastQRing)*)*(FASTQ_ID_MAX+1));
        for(j=0; j<=FASTQ_ID_MAX; j++) { 
            _FQ_NAME(_AllModulesRings)[i]._ring[j] = NULL;
        }
    }
}

always_inline  static inline struct _FQ_NAME(FastQRing) *
_FQ_NAME(__fastq_create_ring)(
#if defined(_FASTQ_EPOLL)
                        const int epfd, 
#elif defined(_FASTQ_SELECT)
                        struct _FQ_NAME(FastQModule) *pmodule,
#endif
                        const unsigned long src, const unsigned long dst, 
                        const unsigned int ring_size, const unsigned int msg_size) {

    fastq_log("Create ring : src(%lu)->dst(%lu) ringsize(%d) msgsize(%d).\n", src, dst, ring_size, msg_size);

    unsigned long ring_real_size = sizeof(struct _FQ_NAME(FastQRing)) + ring_size*(msg_size+sizeof(size_t));
                      
    struct _FQ_NAME(FastQRing) *new_ring = FastQMalloc(ring_real_size);
    assert(new_ring && "Allocate FastQRing Failed. (OOM error)");
    
	memset(new_ring, 0x00, ring_real_size);

    LOG_DEBUG("Create ring %ld->%ld.\n", src, dst);
    new_ring->src = src;
    new_ring->dst = dst;
    new_ring->_size = ring_size - 1;
    
    new_ring->_msg_size = msg_size + sizeof(size_t);
    new_ring->_evt_fd = eventfd(0, EFD_CLOEXEC);
    assert(new_ring->_evt_fd && "Too much eventfd called, no fd to use."); //都TMD没有fd了，你也是厉害
    
    LOG_DEBUG("Create module %ld event fd = %d.\n", dst, new_ring->_evt_fd);
    
    _FQ_NAME(_evtfd_to_ring)[new_ring->_evt_fd]._ring = new_ring;
    
#if defined(_FASTQ_EPOLL)
    struct epoll_event event;
    event.data.fd = new_ring->_evt_fd;
    event.events = EPOLLIN; //必须采用水平触发
    epoll_ctl(epfd, EPOLL_CTL_ADD, event.data.fd, &event);
    LOG_DEBUG("Add eventfd %d to epollfd %d.\n", new_ring->_evt_fd, epfd);
#elif defined(_FASTQ_SELECT)
    pthread_rwlock_wrlock(&pmodule->selector.rwlock);
    FD_SET(new_ring->_evt_fd, &pmodule->selector.allset);    
    if(new_ring->_evt_fd > pmodule->selector.maxfd)
	{
		pmodule->selector.maxfd = new_ring->_evt_fd;
	}
    pthread_rwlock_unlock(&pmodule->selector.rwlock);
#endif

#ifdef FASTQ_STATISTICS //统计功能
    atomic64_init(&new_ring->nr_dequeue);
    atomic64_init(&new_ring->nr_enqueue);
#endif
    return new_ring;
    
}

                      
/**
 *  FastQCreateModule - 注册消息队列
 *  
 *  param[in]   moduleID    模块ID， 范围 1 - FASTQ_ID_MAX
 *  param[in]   msgMax      该模块 的 消息队列 的大小
 *  param[in]   msgSize     最大传递的消息大小
 */
always_inline void inline
_FQ_NAME(FastQCreateModule)(const unsigned long module_id, const unsigned int ring_size, const unsigned int msg_size, \
                            const char *_file, const char *_func, const int _line) {
    assert(module_id <= FASTQ_ID_MAX && "Module ID out of range");

    if(unlikely(!_file) || unlikely(!_func) || unlikely(_line <= 0)) {
        assert(0 && "NULL pointer error");
    }
    
    int i;

    pthread_rwlock_wrlock(&_FQ_NAME(_AllModulesRingsLock));
    
    if(_FQ_NAME(_AllModulesRings)[module_id].already_register) {
        LOG_DEBUG("Module %ld already register.\n", module_id);
        _fastq_fprintf(stderr, "\033[1;5;31mModule ID %ld already register in file <%s>'s function <%s> at line %d\033[m\n", \
                        module_id,
                        _FQ_NAME(_AllModulesRings)[module_id]._file,
                        _FQ_NAME(_AllModulesRings)[module_id]._func,
                        _FQ_NAME(_AllModulesRings)[module_id]._line);
        
        pthread_rwlock_unlock(&_FQ_NAME(_AllModulesRingsLock));
        assert(0);
        return ;
    }
    
    _FQ_NAME(_AllModulesRings)[module_id].already_register = true;
    
#if defined(_FASTQ_EPOLL)
    _FQ_NAME(_AllModulesRings)[module_id].epfd = epoll_create(1);
    assert(_FQ_NAME(_AllModulesRings)[module_id].epfd && "Epoll create error");
    LOG_DEBUG("Create module %ld epoll fd = %d.\n", module_id, _FQ_NAME(_AllModulesRings)[module_id].epfd);
    _fastq_fprintf(stderr, "Create FastQ module with epoll, moduleID = %ld.\n", module_id);
#elif defined(_FASTQ_SELECT)
    LOG_DEBUG("Create module %ld.\n", module_id);
    _fastq_fprintf(stderr, "Create FastQ module with select, moduleID = %ld.\n", module_id);
#endif

    _FQ_NAME(_AllModulesRings)[module_id]._file = FastQStrdup(_file);
    _FQ_NAME(_AllModulesRings)[module_id]._func = FastQStrdup(_func);
    _FQ_NAME(_AllModulesRings)[module_id]._line = _line;
    
    
    _FQ_NAME(_AllModulesRings)[module_id].ring_size = __power_of_2(ring_size);
    _FQ_NAME(_AllModulesRings)[module_id].msg_size = msg_size;

    /* 当源模块未初始化时又想向目的模块发送消息 */
    _FQ_NAME(_AllModulesRings)[module_id]._ring[0] = _FQ_NAME(__fastq_create_ring)(
#if defined(_FASTQ_EPOLL)
                                                                _FQ_NAME(_AllModulesRings)[module_id].epfd, 
#elif defined(_FASTQ_SELECT)
                                                                &_FQ_NAME(_AllModulesRings)[module_id],
#endif  
                                                                0, module_id,\
                                                                __power_of_2(ring_size), msg_size);

    /*建立住的模块和其他模块的连接关系
        若注册前的连接关系如下：
        下图为已经注册过两个模块 (模块 A 和 模块 B) 的数据结构

                    +---+
                    |   |
                    | A |
                    |   |
                  / +---+
                 /  /
                /  /
               /  /
              /  /
             /  /
         +---+ /
         |   |
         | B |
         |   |
         +---+

        在此基础上注册新的模块 (模块 C) 通过接下来的操作，将会创建四个 ring

                    +---+
                    |   |
                    | A |
                    |   |
                  / +---+ \
                 /  /   \  \
                /  /     \  \
               /  /       \  \
              /  /         \  \
             /  /           \  \
         +---+ /             \ +---+ 
         |   | <-------------- |   |
         | B |                 | C |
         |   | --------------> |   |
         +---+                 +---+
    */
    for(i=1; i<=FASTQ_ID_MAX && i != module_id; i++) {
        if(!_FQ_NAME(_AllModulesRings)[i].already_register) {
            continue;
        }
        _FQ_NAME(_AllModulesRings)[module_id]._ring[i] = \
                            _FQ_NAME(__fastq_create_ring)(
#if defined(_FASTQ_EPOLL)
                                                        _FQ_NAME(_AllModulesRings)[module_id].epfd, 
#elif defined(_FASTQ_SELECT)
                                                        &_FQ_NAME(_AllModulesRings)[module_id],
#endif                             
                                                        i, module_id,\
                                                        __power_of_2(ring_size), msg_size);
        if(!_FQ_NAME(_AllModulesRings)[i]._ring[module_id]) {
            _FQ_NAME(_AllModulesRings)[i]._ring[module_id] = \
                    _FQ_NAME(__fastq_create_ring)(
#if defined(_FASTQ_EPOLL)
                                                    _FQ_NAME(_AllModulesRings)[i].epfd, 
#elif defined(_FASTQ_SELECT)
                                                    &_FQ_NAME(_AllModulesRings)[i],
#endif                             
                                                    module_id, i, \
                                                    _FQ_NAME(_AllModulesRings)[module_id].ring_size, _FQ_NAME(_AllModulesRings)[i].msg_size);
        }
    }
    
    pthread_rwlock_unlock(&_FQ_NAME(_AllModulesRingsLock));

    return;
}

/**
 *  __FastQSend - 公共发送函数
 */
always_inline static bool inline
_FQ_NAME(__FastQSend)(struct _FQ_NAME(FastQRing) *ring, const void *msg, const size_t size) {
    assert(ring);
    assert(size <= (ring->_msg_size - sizeof(size_t)));

    unsigned int h = (ring->_head - 1) & ring->_size;
    unsigned int t = ring->_tail;
    if (t == h) {
        return false;
    }

    LOG_DEBUG("Send %ld->%ld.\n", ring->src, ring->dst);

    char *d = &ring->_ring_data[t*ring->_msg_size];
    
    memcpy(d, &size, sizeof(size));
    LOG_DEBUG("Send >>> memcpy msg %lx( %lx) size = %d\n", msg, *(unsigned long*)msg, size);
    memcpy(d + sizeof(size), msg, size);
    LOG_DEBUG("Send >>> memcpy addr = %x\n", *(unsigned long*)(d + sizeof(size)));

    // Barrier is needed to make sure that item is updated 
    // before it's made available to the reader
    mwbarrier();
#ifdef FASTQ_STATISTICS //统计功能
    atomic64_inc(&ring->nr_enqueue);
#endif

    ring->_tail = (t + 1) & ring->_size;
    return true;
}

/**
 *  FastQSend - 发送消息（轮询直至成功发送）
 *  
 *  param[in]   from    源模块ID， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   to      目的模块ID， 范围 1 - FASTQ_ID_MAX
 *  param[in]   msg     传递的消息体
 *  param[in]   size    传递的消息大小
 *
 *  return 成功true （轮询直至发送成功，只可能返回 true ）
 *
 *  注意：from 和 to 需要使用 FastQCreateModule 注册后使用
 */
always_inline bool inline
_FQ_NAME(FastQSend)(unsigned int from, unsigned int to, const void *msg, size_t size) {
    struct _FQ_NAME(FastQRing) *ring = _FQ_NAME(_AllModulesRings)[to]._ring[from];
    while (!_FQ_NAME(__FastQSend)(ring, msg, size)) {__relax();}
    
    eventfd_write(ring->_evt_fd, 1);
    
    LOG_DEBUG("Send done %ld->%ld, event fd = %d, msg = %p.\n", ring->src, ring->dst, ring->_evt_fd, msg);
    return true;
}

/**
 *  FastQTrySend - 发送消息（尝试向队列中插入，当队列满是直接返回false）
 *  
 *  param[in]   from    源模块ID， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   to      目的模块ID， 范围 1 - FASTQ_ID_MAX
 *  param[in]   msg     传递的消息体
 *  param[in]   size    传递的消息大小
 *
 *  return 成功true 失败false
 *
 *  注意：from 和 to 需要使用 FastQCreateModule 注册后使用
 */
always_inline bool inline
_FQ_NAME(FastQTrySend)(unsigned int from, unsigned int to, const void *msg, size_t size) {
    struct _FQ_NAME(FastQRing) *ring = _FQ_NAME(_AllModulesRings)[to]._ring[from];
    LOG_DEBUG("Send >>> msg %lx( %lx) size = %d\n", msg, *(unsigned long*)msg, size);
    bool ret = _FQ_NAME(__FastQSend)(ring, msg, size);
    if(ret) {
        eventfd_write(ring->_evt_fd, 1);
        LOG_DEBUG("Send done %ld->%ld, event fd = %d.\n", ring->src, ring->dst, ring->_evt_fd);
    }
    return ret;
}

always_inline static bool inline
_FQ_NAME(__FastQRecv)(struct _FQ_NAME(FastQRing) *ring, void *msg, size_t *size) {

    unsigned int t = ring->_tail;
    unsigned int h = ring->_head;
    if (h == t) {
//        LOG_DEBUG("Recv <<< %ld->%ld.\n", ring->src, ring->dst);
        return false;
    }
    
    LOG_DEBUG("Recv <<< %ld->%ld.\n", ring->src, ring->dst);

    char *d = &ring->_ring_data[h*ring->_msg_size];

    size_t recv_size;
    memcpy(&recv_size, d, sizeof(size_t));
    LOG_DEBUG("Recv <<< memcpy recv_size = %d\n", recv_size);
    assert(recv_size <= *size && "buffer too small");
    *size = recv_size;
    LOG_DEBUG("Recv <<< size\n");
    memcpy(msg, d + sizeof(size_t), recv_size);
    LOG_DEBUG("Recv <<< memcpy addr = %x\n", *(unsigned long*)(d + sizeof(size_t)));
    LOG_DEBUG("Recv <<< memcpy msg %lx( %lx) size = %d\n", msg, *(unsigned long*)msg, *size);

    // Barrier is needed to make sure that we finished reading the item
    // before moving the head
    mbarrier();
    LOG_DEBUG("Recv <<< mbarrier\n");
#ifdef FASTQ_STATISTICS //统计功能
    atomic64_inc(&ring->nr_dequeue);
#endif

    ring->_head = (h + 1) & ring->_size;
    return true;
}

/**
 *  FastQRecv - 接收消息
 *  
 *  param[in]   from    从模块ID from 中读取消息， 范围 1 - FASTQ_ID_MAX 
 *  param[in]   handler 消息处理函数，参照 fq_msg_handler_t 说明
 *
 *  return 成功true 失败false
 *
 *  注意：from 需要使用 FastQCreateModule 注册后使用
 */
always_inline  bool inline
_FQ_NAME(FastQRecv)(unsigned int from, fq_msg_handler_t handler) {

    assert(handler && "NULL pointer error.");

    eventfd_t cnt;
    int nfds;
#if defined(_FASTQ_EPOLL)
    struct epoll_event events[8];
#elif defined(_FASTQ_SELECT)
    int i, max_fd;
#endif 
    int curr_event_fd;
    char __attribute__((aligned(64))) addr[4096] = {0}; //page size
    size_t size = sizeof(addr);
    struct _FQ_NAME(FastQRing) *ring = NULL;

    while(1) {
#if defined(_FASTQ_EPOLL)
        LOG_DEBUG("Recv start >>>>  epoll fd = %d.\n", _FQ_NAME(_AllModulesRings)[from].epfd);
        nfds = epoll_wait(_FQ_NAME(_AllModulesRings)[from].epfd, events, 8, -1);
        LOG_DEBUG("Recv epoll return epfd = %d.\n", _FQ_NAME(_AllModulesRings)[from].epfd);
#elif defined(_FASTQ_SELECT)
        _FQ_NAME(_AllModulesRings)[from].selector.readset = _FQ_NAME(_AllModulesRings)[from].selector.allset;
        max_fd = _FQ_NAME(_AllModulesRings)[from].selector.maxfd;

        nfds = select(max_fd+1, &_FQ_NAME(_AllModulesRings)[from].selector.readset, NULL, NULL, NULL);
        
#endif 
        
#if defined(_FASTQ_EPOLL)
        for(;nfds--;) {
            curr_event_fd = events[nfds].data.fd;
#elif defined(_FASTQ_SELECT)
            
        for (i = 3; i <= max_fd; ++i) {
            if(!FD_ISSET(i, &_FQ_NAME(_AllModulesRings)[from].selector.readset)) {
                continue;
            }
            nfds--;
            curr_event_fd = i;
#endif 
            ring = _FQ_NAME(_evtfd_to_ring)[curr_event_fd]._ring;
            eventfd_read(curr_event_fd, &cnt);
//            if(cnt>1) {
//                printf("cnt = =%ld\n", cnt);
//            }
            LOG_DEBUG("Event fd %d read return cnt = %ld.\n", curr_event_fd, cnt);
            for(; cnt--;) {
                LOG_DEBUG("<<< _FQ_NAME(__FastQRecv).\n");
                while (!_FQ_NAME(__FastQRecv)(ring, addr, &size)) {
                    __relax();
                }
                LOG_DEBUG("<<< _FQ_NAME(__FastQRecv) addr = %lx, size = %ld.\n", *(unsigned long*)addr, size);
                handler((void*)addr, size);
                LOG_DEBUG("<<< _FQ_NAME(__FastQRecv) done.\n");
//                addr[0] = 0;
//                size = sizeof(unsigned long);
            }

        }
    }
    return true;
}


/**
 *  FastQInfo - 查询信息
 *  
 *  param[in]   fp    文件指针,当 fp == NULL，默认使用 stderr 
 *  param[in]   module_id 需要显示的模块ID， 等于 0 时显示全部
 */
always_inline bool inline
_FQ_NAME(FastQMsgStatInfo)(struct FastQModuleMsgStatInfo *buf, unsigned int buf_mod_size, unsigned int *num, 
                            fq_module_filter_t filter) {
    
    assert(buf && num && "NULL pointer error.");
    assert(buf_mod_size && "buf_mod_size MUST bigger than zero.");

#ifdef FASTQ_STATISTICS //统计功能

    unsigned long dstID, srcID, bufIdx = 0;
    *num = 0;

    for(dstID=1; dstID<=FASTQ_ID_MAX; dstID++) {
        if(!_FQ_NAME(_AllModulesRings)[dstID].already_register) {
            continue;
        }
        if(!_FQ_NAME(_AllModulesRings)[dstID]._ring) {
            continue;
        }
        for(srcID=0; srcID<=FASTQ_ID_MAX; srcID++) { 
            
            if(!_FQ_NAME(_AllModulesRings)[dstID]._ring[srcID]) {
                continue;
            }
        
            //过滤掉一些
            if(filter) {
                if(!filter(srcID, dstID)) continue;
            }
            buf[bufIdx].src_module = srcID;
            buf[bufIdx].dst_module = dstID;

            LOG_DEBUG("enqueue = %ld\n", atomic64_read(&_FQ_NAME(_AllModulesRings)[dstID]._ring[srcID]->nr_enqueue));
            LOG_DEBUG("dequeue = %ld\n", atomic64_read(&_FQ_NAME(_AllModulesRings)[dstID]._ring[srcID]->nr_dequeue));
            
            buf[bufIdx].enqueue = atomic64_read(&_FQ_NAME(_AllModulesRings)[dstID]._ring[srcID]->nr_enqueue);
            buf[bufIdx].dequeue = atomic64_read(&_FQ_NAME(_AllModulesRings)[dstID]._ring[srcID]->nr_dequeue);
            
            bufIdx++;
            (*num)++;
            if(buf_mod_size == bufIdx) 
                return true;
        }
    }
    return true;

#else //不支持统计功能，直接返回失败
    return false;
#endif
}

/**
 *  FastQDump - 显示信息
 *  
 *  param[in]   fp    文件指针,当 fp == NULL，默认使用 stderr 
 *  param[in]   module_id 需要显示的模块ID， 等于 0 时显示全部
 */
always_inline void inline
_FQ_NAME(FastQDump)(FILE*fp, unsigned long module_id) {
    
    if(unlikely(!fp)) {
        fp = stderr;
    }
    _fastq_fprintf(fp, "\n FastQ Dump Information.\n");

    unsigned long i, j, max_module = FASTQ_ID_MAX;

    if(module_id == 0 || module_id > FASTQ_ID_MAX) {
        i = 1;
        max_module = FASTQ_ID_MAX;
    } else {
        i = module_id;
        max_module = module_id;
    }
    
    
    for(i=1; i<=max_module; i++) {
        if(!_FQ_NAME(_AllModulesRings)[i].already_register) {
            continue;
        }
        _fastq_fprintf(fp, "\033[1;31mModule ID %ld register in file <%s>'s function <%s> at line %d\033[m\n", \
                        i,
                        _FQ_NAME(_AllModulesRings)[i]._file,
                        _FQ_NAME(_AllModulesRings)[i]._func,
                        _FQ_NAME(_AllModulesRings)[i]._line);
#ifdef FASTQ_STATISTICS //统计功能
        atomic64_t module_total_msgs[2];
        atomic64_init(&module_total_msgs[0]); //总入队数量
        atomic64_init(&module_total_msgs[1]); //总出队数量
#endif        
        _fastq_fprintf(fp, "------------------------------------------\n"\
                    "ID: %3ld, msgMax %4u, msgSize %4u\n"\
                    "\t from-> to   %10s %10s"
#ifdef FASTQ_STATISTICS //统计功能
                    " %16s %16s %16s "
#endif                    
                    "\n"
                    , i, 
                    _FQ_NAME(_AllModulesRings)[i].ring_size, 
                    _FQ_NAME(_AllModulesRings)[i].msg_size, 
                    "head", 
                    "tail"
#ifdef FASTQ_STATISTICS //统计功能
                    , "enqueue", "dequeue", "current"
#endif                    
                    );
        
        for(j=0; j<=FASTQ_ID_MAX; j++) { 
            if(_FQ_NAME(_AllModulesRings)[i]._ring[j]) {
                _fastq_fprintf(fp, "\t %4ld->%-4ld  %10u %10u"
#ifdef FASTQ_STATISTICS //统计功能
                            " %16ld %16ld %16ld"
#endif                            
                            "\n" \
                            , j, i, 
                            _FQ_NAME(_AllModulesRings)[i]._ring[j]->_head, 
                            _FQ_NAME(_AllModulesRings)[i]._ring[j]->_tail
#ifdef FASTQ_STATISTICS //统计功能
                            ,atomic64_read(&_FQ_NAME(_AllModulesRings)[i]._ring[j]->nr_enqueue),
                            atomic64_read(&_FQ_NAME(_AllModulesRings)[i]._ring[j]->nr_dequeue),
                            atomic64_read(&_FQ_NAME(_AllModulesRings)[i]._ring[j]->nr_enqueue)
                                -atomic64_read(&_FQ_NAME(_AllModulesRings)[i]._ring[j]->nr_dequeue)
#endif                                
                            );
#ifdef FASTQ_STATISTICS //统计功能
                atomic64_add(&module_total_msgs[0], atomic64_read(&_FQ_NAME(_AllModulesRings)[i]._ring[j]->nr_enqueue));
                atomic64_add(&module_total_msgs[1], atomic64_read(&_FQ_NAME(_AllModulesRings)[i]._ring[j]->nr_dequeue));
#endif                
            }
        }
        
#ifdef FASTQ_STATISTICS //统计功能
        _fastq_fprintf(fp, "\t Total enqueue %16ld, dequeue %16ld\n", atomic64_read(&module_total_msgs[0]), atomic64_read(&module_total_msgs[1]));
#endif
    }
    fflush(fp);
    return;
}


