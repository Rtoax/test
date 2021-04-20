/**********************************************************************************************************************\
*  文件： fastq_compat.c
*  介绍： 低时延队列 两套函数的接口 参见 fastq.h _FQ_NAME 定义
*  作者： 荣涛
*  日期：
*       2021年2月2日 创建文件
\**********************************************************************************************************************/


// 从 event fd 查找 ring 的最快方法
static  struct {
    struct FastQRing *tlb_ring;
} _FQ_NAME(_evtfd_to_ring)[FD_SETSIZE] = {{NULL}};

static struct FastQModule *_FQ_NAME(_AllModulesRings) = NULL;
static pthread_rwlock_t _FQ_NAME(_AllModulesRingsLock) = PTHREAD_RWLOCK_INITIALIZER; //只在注册时保护使用
static dict *_FQ_NAME(dictModuleNameID) = NULL;

/**
 *  FastQ 初始化 函数，初始化 _AllModulesRings 全局变量
 */
static void __attribute__((constructor(105))) _FQ_NAME(__FastQInitCtor) () {
    int i, j;
    LOG_DEBUG("Init _module_src_dst_to_ring.\n");
    _FQ_NAME(_AllModulesRings) = FastQMalloc(sizeof(struct FastQModule)*(FASTQ_ID_MAX+1));
    for(i=0; i<=FASTQ_ID_MAX; i++) {
        __atomic_store_n(&_FQ_NAME(_AllModulesRings)[i].already_register, false, __ATOMIC_RELEASE);
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
        //清空   rx 和 tx set
        MOD_ZERO(&_FQ_NAME(_AllModulesRings)[i].rx.set);
        MOD_ZERO(&_FQ_NAME(_AllModulesRings)[i].tx.set);
        __atomic_store_n(&_FQ_NAME(_AllModulesRings)[i].rx.use, false, __ATOMIC_RELAXED);
        __atomic_store_n(&_FQ_NAME(_AllModulesRings)[i].tx.use, false, __ATOMIC_RELAXED);

        //清空 ring
        _FQ_NAME(_AllModulesRings)[i].ring_size = 0;
        _FQ_NAME(_AllModulesRings)[i].msg_size = 0;

        //分配所有 ring 指针
        struct FastQRing **___ring = FastQMalloc(sizeof(struct FastQRing*)*(FASTQ_ID_MAX+1));
        assert(___ring && "Malloc Failed: Out of Memory.");
        
        _FQ_NAME(_AllModulesRings)[i]._ring = ___ring;
        for(j=0; j<=FASTQ_ID_MAX; j++) { 
	        __atomic_store_n(&_FQ_NAME(_AllModulesRings)[i]._ring[j], NULL, __ATOMIC_RELAXED);
        }
    }
    
    //初始化 模块名->模块ID 字典
    _FQ_NAME(dictModuleNameID) = dictCreate(&commandTableDictType,NULL);
    dictExpand(_FQ_NAME(dictModuleNameID), FASTQ_ID_MAX);
}

static struct FastQRing *
_FQ_NAME(__fastq_create_ring)(
                        struct FastQModule *pmodule,
                        const unsigned long src, const unsigned long dst, 
                        const unsigned int ring_size, const unsigned int msg_size) {

    fastq_log("Create ring : src(%lu)->dst(%lu) ringsize(%d) msgsize(%d).\n", src, dst, ring_size, msg_size);

    unsigned long ring_real_size = sizeof(struct FastQRing) + ring_size*(msg_size+sizeof(size_t));
                      
    struct FastQRing *new_ring = FastQMalloc(ring_real_size);
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
        
	/* fd->ring 的快表 更应该是空的 */
	if (likely(__atomic_load_n(&_FQ_NAME(_evtfd_to_ring)[new_ring->_evt_fd].tlb_ring, __ATOMIC_RELAXED) == NULL)) {
	    __atomic_store_n(&_FQ_NAME(_evtfd_to_ring)[new_ring->_evt_fd].tlb_ring, new_ring, __ATOMIC_RELAXED);
	} else {
	    fastq_log("Already Register src(%ul)->dst(%ul) module.", src, dst);
        assert(0 && "Already Register.");
    }
    
#if defined(_FASTQ_EPOLL)
    struct epoll_event event;
    event.data.fd = new_ring->_evt_fd;
    event.events = EPOLLIN; //必须采用水平触发
    epoll_ctl(pmodule->epfd, EPOLL_CTL_ADD, event.data.fd, &event);
    LOG_DEBUG("Add eventfd %d to epollfd %d.\n", new_ring->_evt_fd, pmodule->epfd);
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
_FQ_NAME(FastQCreateModule)(const char *name, const unsigned long module_id, \
                     const mod_set *rxset, const mod_set *txset, \
                     const unsigned int ring_size, const unsigned int msg_size, \
                            const char *_file, const char *_func, const int _line) {
    assert(module_id <= FASTQ_ID_MAX && "Module ID out of range");

    if(unlikely(!_file) || unlikely(!_func) || unlikely(_line <= 0) || unlikely(!name)) {
        assert(0 && "NULL pointer error");
    }
    
    int i;

    //锁
    pthread_rwlock_wrlock(&_FQ_NAME(_AllModulesRingsLock));

    //检查模块是否已经注册
    if(__atomic_load_n(&_FQ_NAME(_AllModulesRings)[module_id].already_register, __ATOMIC_RELAXED)) {
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

    //保存名字并添加至 字典
    strncpy(_FQ_NAME(_AllModulesRings)[module_id].name, name, NAME_LEN);
    dict_register_module(_FQ_NAME(dictModuleNameID), name, module_id);

    //设置注册标志位
    __atomic_store_n(&_FQ_NAME(_AllModulesRings)[module_id].already_register, true, __ATOMIC_RELEASE);
        
    //设置 发送 接收 set
    __atomic_store_n(&_FQ_NAME(_AllModulesRings)[module_id].rx.use, rxset?true:false, __ATOMIC_RELAXED);
    if(__atomic_load_n(&_FQ_NAME(_AllModulesRings)[module_id].rx.use, __ATOMIC_RELAXED)) {
        memcpy(&_FQ_NAME(_AllModulesRings)[module_id].rx.set, rxset, sizeof(mod_set));
    } else {
        memset(&_FQ_NAME(_AllModulesRings)[module_id].rx.set, 0x0, sizeof(mod_set));
    }    
    __atomic_store_n(&_FQ_NAME(_AllModulesRings)[module_id].tx.use, txset?true:false, __ATOMIC_RELAXED);
    if(__atomic_load_n(&_FQ_NAME(_AllModulesRings)[module_id].tx.use, __ATOMIC_RELAXED)) {
        memcpy(&_FQ_NAME(_AllModulesRings)[module_id].tx.set, txset, sizeof(mod_set));
    } else {
        memset(&_FQ_NAME(_AllModulesRings)[module_id].tx.set, 0x0, sizeof(mod_set));
    }
    
#if defined(_FASTQ_EPOLL)
    _FQ_NAME(_AllModulesRings)[module_id].epfd = epoll_create(1);
    assert(_FQ_NAME(_AllModulesRings)[module_id].epfd && "Epoll create error");
    LOG_DEBUG("Create module %ld epoll fd = %d.\n", module_id, _FQ_NAME(_AllModulesRings)[module_id].epfd);
    _fastq_fprintf(stderr, "Create FastQ module with epoll, moduleID = %ld.\n", module_id);
#elif defined(_FASTQ_SELECT)
    LOG_DEBUG("Create module %ld.\n", module_id);
    _fastq_fprintf(stderr, "Create FastQ module with select, moduleID = %ld.\n", module_id);
#endif

    //在哪里注册，用于调试
    _FQ_NAME(_AllModulesRings)[module_id]._file = FastQStrdup(_file);
    _FQ_NAME(_AllModulesRings)[module_id]._func = FastQStrdup(_func);
    _FQ_NAME(_AllModulesRings)[module_id]._line = _line;
    
    //队列大小
    _FQ_NAME(_AllModulesRings)[module_id].ring_size = __power_of_2(ring_size);
    _FQ_NAME(_AllModulesRings)[module_id].msg_size = msg_size;

    //当设置了标志位，并且对应的 ring 为空
    if(MOD_ISSET(0, &_FQ_NAME(_AllModulesRings)[module_id].rx.set) && 
        !__atomic_load_n(&_FQ_NAME(_AllModulesRings)[module_id]._ring[0], __ATOMIC_RELAXED)) {
        /* 当源模块未初始化时又想向目的模块发送消息 */
        struct FastQRing *__ring = _FQ_NAME(__fastq_create_ring)(
                                                                    &_FQ_NAME(_AllModulesRings)[module_id],
                                                                    0, module_id,\
                                                                    __power_of_2(ring_size), msg_size);
        __atomic_store_n(&_FQ_NAME(_AllModulesRings)[module_id]._ring[0], __ring, __ATOMIC_RELAXED);
    }
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

        值得注意的是，在创建 ring 时，会根据入参中的 rxset 和 txset 决定分配哪些 ring 和 eventfd
        以上面的 ABC 模块为例，具体如下：

        注册过程：假设模块ID分别为 A=1, B=2, C=3
        
                    rxset       txset           表明
            A(1)    0110        0000    A可能从B(2)C(3)接收，不接收任何数据
            B(2)    0000        0010    B不接受任何数据，可能向A(1)发送
            C(3)    0000        0010    C不接受任何数据，可能向A(1)发送
         那么创建的 底层数据结构将会是
         
                    +---+
                    |   |
                    | A |
                    |   |
                  /`+---+ \`
                 /         \
                /           \
               /             \
              /               \
             /                 \
         +---+                 +---+ 
         |   |                 |   |
         | B |                 | C |
         |   |                 |   |
         +---+                 +---+
        
    */
    for(i=1; i<=FASTQ_ID_MAX; i++) {
        if(i==module_id) continue;
        if(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[i].already_register, __ATOMIC_RELAXED)) {
            continue;
        }

        //任意一个模块标记了可能发送或者接收的模块，都将创建队列
        if(MOD_ISSET(i, &_FQ_NAME(_AllModulesRings)[module_id].rx.set) || 
           MOD_ISSET(module_id, &_FQ_NAME(_AllModulesRings)[i].tx.set)) {

           if(!MOD_ISSET(i, &_FQ_NAME(_AllModulesRings)[module_id].rx.set)) {
                MOD_SET(i, &_FQ_NAME(_AllModulesRings)[module_id].rx.set);
           }
           if(!MOD_ISSET(module_id, &_FQ_NAME(_AllModulesRings)[i].tx.set)) {
                MOD_SET(module_id, &_FQ_NAME(_AllModulesRings)[i].tx.set);
           }
            _FQ_NAME(_AllModulesRings)[module_id]._ring[i] = \
                                _FQ_NAME(__fastq_create_ring)(
                                                            &_FQ_NAME(_AllModulesRings)[module_id],
                                                            i, module_id,\
                                                            __power_of_2(ring_size), msg_size);
        }
        if(!_FQ_NAME(_AllModulesRings)[i]._ring[module_id]) {
            
            if(MOD_ISSET(i, &_FQ_NAME(_AllModulesRings)[module_id].tx.set) || 
               MOD_ISSET(module_id, &_FQ_NAME(_AllModulesRings)[i].rx.set)) {

                if(!MOD_ISSET(i, &_FQ_NAME(_AllModulesRings)[module_id].tx.set)) {
                    MOD_SET(i, &_FQ_NAME(_AllModulesRings)[module_id].tx.set);
                }
                if(!MOD_ISSET(module_id, &_FQ_NAME(_AllModulesRings)[i].rx.set)) {
                    MOD_SET(module_id, &_FQ_NAME(_AllModulesRings)[i].rx.set);
                }
               
                _FQ_NAME(_AllModulesRings)[i]._ring[module_id] = \
                        _FQ_NAME(__fastq_create_ring)(
                                                        &_FQ_NAME(_AllModulesRings)[i],
                                                        module_id, i, \
                                                        _FQ_NAME(_AllModulesRings)[i].ring_size,\
                                                        _FQ_NAME(_AllModulesRings)[i].msg_size);
            }
        }
    }
    
    pthread_rwlock_unlock(&_FQ_NAME(_AllModulesRingsLock));

    return;
}

always_inline bool inline
_FQ_NAME(FastQAddSet)(const unsigned long moduleID, 
                        const mod_set *rxset, const mod_set *txset) {
                                
    if(unlikely(!rxset && !txset)) {
        return false;
    }
    if(moduleID <= 0 || moduleID > FASTQ_ID_MAX) {
        return false;
    }
    if(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[moduleID].already_register, __ATOMIC_RELAXED)) {
        return false;
    }
    int i;
    _fastq_fprintf(stderr, "Add FastQ module set to moduleID = %ld.\n", moduleID);
    
    pthread_rwlock_wrlock(&_FQ_NAME(_AllModulesRingsLock));

    //遍历
    for(i=1; i<=FASTQ_ID_MAX; i++) {
        if(i==moduleID) continue;

        //目的模块必须存在
        if(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[i].already_register, __ATOMIC_RELAXED)) {
            continue;
        }

        //接收
        if(rxset && MOD_ISSET(i, rxset) && !MOD_ISSET(i, &_FQ_NAME(_AllModulesRings)[moduleID].rx.set)) {
            MOD_SET(i, &_FQ_NAME(_AllModulesRings)[moduleID].rx.set);
            MOD_SET(moduleID, &_FQ_NAME(_AllModulesRings)[i].tx.set);

            _FQ_NAME(_AllModulesRings)[moduleID]._ring[i] = \
                        _FQ_NAME(__fastq_create_ring)(
                                                        &_FQ_NAME(_AllModulesRings)[moduleID],
                                                        i, moduleID, \
                                                        _FQ_NAME(_AllModulesRings)[moduleID].ring_size,\
                                                        _FQ_NAME(_AllModulesRings)[moduleID].msg_size);
            
        }
        //发送
        if(txset && MOD_ISSET(i, txset) && !MOD_ISSET(i, &_FQ_NAME(_AllModulesRings)[moduleID].tx.set)) {
            MOD_SET(i, &_FQ_NAME(_AllModulesRings)[moduleID].tx.set);
            MOD_SET(moduleID, &_FQ_NAME(_AllModulesRings)[i].rx.set);
        
            _FQ_NAME(_AllModulesRings)[i]._ring[moduleID] = \
                    _FQ_NAME(__fastq_create_ring)(
                                                    &_FQ_NAME(_AllModulesRings)[i],
                                                    moduleID, i, \
                                                    _FQ_NAME(_AllModulesRings)[i].ring_size,\
                                                    _FQ_NAME(_AllModulesRings)[i].msg_size);

            
        }
    }
    
    pthread_rwlock_unlock(&_FQ_NAME(_AllModulesRingsLock));

    return true;
}

always_inline bool inline
_FQ_NAME(FastQDeleteModule)(const char *name, const unsigned long moduleID, 
                        void *residual_msgs, int *residual_nbytes) {
    if(!name && (moduleID <= 0 || moduleID > FASTQ_ID_MAX) ) {
        fastq_log("Try to delete not exist MODULE.\n");
        assert(0);
        return false;
    }
    unsigned long _moduleID = moduleID;
    //检查模块是否已经注册
    if(__atomic_load_n(&_FQ_NAME(_AllModulesRings)[_moduleID].already_register, __ATOMIC_RELAXED)) {
        LOG_DEBUG("Try delete Module %ld.\n", _moduleID);
        _fastq_fprintf(stderr, "\033[1;5;31m Delete Module ID %ld, Which register in file <%s>'s function <%s> at line %d\033[m\n", \
                        _moduleID,
                        _FQ_NAME(_AllModulesRings)[_moduleID]._file,
                        _FQ_NAME(_AllModulesRings)[_moduleID]._func,
                        _FQ_NAME(_AllModulesRings)[_moduleID]._line);
        
        pthread_rwlock_unlock(&_FQ_NAME(_AllModulesRingsLock));
        assert(0);
        return ;
    }

    return true;
}


/**
 *  __FastQSend - 公共发送函数
 */
always_inline static bool inline
_FQ_NAME(__FastQSend)(struct FastQRing *ring, const void *msg, const size_t size) {
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
    struct FastQRing *ring = __atomic_load_n(&_FQ_NAME(_AllModulesRings)[to]._ring[from], __ATOMIC_RELAXED);

    while (!_FQ_NAME(__FastQSend)(ring, msg, size)) {__relax();}
    
    eventfd_write(ring->_evt_fd, 1);
    
    LOG_DEBUG("Send done %ld->%ld, event fd = %d, msg = %p.\n", ring->src, ring->dst, ring->_evt_fd, msg);
    return true;
}

always_inline bool inline
_FQ_NAME(FastQSendByName)(const char* from, const char* to, const void *msg, size_t size) {
    assert(from && "NULL string.");
    assert(to && "NULL string.");
    unsigned long from_id = dict_find_module_id_byname(_FQ_NAME(dictModuleNameID), from);
    unsigned long to_id = dict_find_module_id_byname(_FQ_NAME(dictModuleNameID), to);
    
    if(unlikely(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[from_id].already_register, __ATOMIC_RELAXED))) {
        fastq_log("No such module %s.\n", from);
        return false;
    } if(unlikely(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[to_id].already_register, __ATOMIC_RELAXED))) {
        fastq_log("No such module %s.\n", from);
        return false;
    }
    return _FQ_NAME(FastQSend)(from_id, to_id, msg, size);
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
    struct FastQRing *ring = __atomic_load_n(&_FQ_NAME(_AllModulesRings)[to]._ring[from], __ATOMIC_RELAXED);
    
    LOG_DEBUG("Send >>> msg %lx( %lx) size = %d\n", msg, *(unsigned long*)msg, size);
    bool ret = _FQ_NAME(__FastQSend)(ring, msg, size);
    if(ret) {
        eventfd_write(ring->_evt_fd, 1);
        LOG_DEBUG("Send done %ld->%ld, event fd = %d.\n", ring->src, ring->dst, ring->_evt_fd);
    }
    return ret;
}

always_inline bool inline
_FQ_NAME(FastQTrySendByName)(const char* from, const char* to, const void *msg, size_t size) {
    assert(from && "NULL string.");
    assert(to && "NULL string.");
    unsigned long from_id = dict_find_module_id_byname(_FQ_NAME(dictModuleNameID), from);
    unsigned long to_id = dict_find_module_id_byname(_FQ_NAME(dictModuleNameID), to);
    if(unlikely(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[from_id].already_register, __ATOMIC_RELAXED))) {
        fastq_log("No such module %s.\n", from);
        return false;
    } if(unlikely(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[to_id].already_register, __ATOMIC_RELAXED))) {
        fastq_log("No such module %s.\n", from);
        return false;
    }

    return _FQ_NAME(FastQTrySend)(from_id, to_id, msg, size);
}

always_inline static bool inline
_FQ_NAME(__FastQRecv)(struct FastQRing *ring, void *msg, size_t *size) {

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
    struct FastQRing *ring = NULL;

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
            ring = __atomic_load_n(&_FQ_NAME(_evtfd_to_ring)[curr_event_fd].tlb_ring, __ATOMIC_RELAXED);
            eventfd_read(curr_event_fd, &cnt);

            LOG_DEBUG("Event fd %d read return cnt = %ld.\n", curr_event_fd, cnt);
            for(; cnt--;) {
                LOG_DEBUG("<<< _FQ_NAME(__FastQRecv).\n");
                while (!_FQ_NAME(__FastQRecv)(ring, addr, &size)) {
                    __relax();
                }
                LOG_DEBUG("<<< _FQ_NAME(__FastQRecv) addr = %lx, size = %ld.\n", *(unsigned long*)addr, size);
                handler((void*)addr, size);
                LOG_DEBUG("<<< _FQ_NAME(__FastQRecv) done.\n");
            }

        }
    }
    return true;
}

always_inline  bool inline
_FQ_NAME(FastQRecvByName)(const char *from, fq_msg_handler_t handler) { 
    assert(from && "NULL string.");
    unsigned long from_id = dict_find_module_id_byname(_FQ_NAME(dictModuleNameID), from);
    if(unlikely(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[from_id].already_register, __ATOMIC_RELAXED))) {
        fastq_log("No such module %s.\n", from);
        return false;
    }
    return _FQ_NAME(FastQRecv)(from_id, handler);
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
//        printf("------------------ %d -> reg %d\n",dstID, _FQ_NAME(_AllModulesRings)[dstID].already_register);
        if(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[dstID].already_register, __ATOMIC_RELAXED)) {
            continue;
        }
        if(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[dstID]._ring, __ATOMIC_RELAXED)) {
            continue;
        }
        for(srcID=0; srcID<=FASTQ_ID_MAX; srcID++) { 
            
            if(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[dstID]._ring[srcID], __ATOMIC_RELAXED)) {
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
        if(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[i].already_register, __ATOMIC_RELAXED)) {
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
            if(__atomic_load_n(&_FQ_NAME(_AllModulesRings)[i]._ring[j], __ATOMIC_RELAXED)) {
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


always_inline  bool inline
_FQ_NAME(FastQMsgNum)(unsigned int ID, 
            unsigned long *nr_enqueues, unsigned long *nr_dequeues, unsigned long *nr_currents) {

    if(ID <= 0 || ID > FASTQ_ID_MAX) {
        return false;
    }
    if(!__atomic_load_n(&_FQ_NAME(_AllModulesRings)[ID].already_register, __ATOMIC_RELAXED)) {
        return false;
    }
//    printf("FastQMsgNum .\n");
    
#ifdef FASTQ_STATISTICS //统计功能
    int i;
    *nr_dequeues = *nr_enqueues = *nr_currents = 0;
    
    for(i=1; i<=FASTQ_ID_MAX; i++) {
        if(MOD_ISSET(i, &_FQ_NAME(_AllModulesRings)[ID].rx.set)) {
            *nr_enqueues += atomic64_read(&_FQ_NAME(_AllModulesRings)[ID]._ring[i]->nr_enqueue);
            *nr_dequeues += atomic64_read(&_FQ_NAME(_AllModulesRings)[ID]._ring[i]->nr_dequeue);
        }
    }
    *nr_currents = (*nr_enqueues) - (*nr_dequeues);
    
    return true;
#endif

    return false;

}

